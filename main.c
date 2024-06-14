#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <sys/time.h>
#include <errno.h>


#define BUFFER_SIZE 1024


typedef enum ft_packet_status{
	not_sent,
	sent,
	received,
	not_received
}ft_packet_status;


struct ft_udp_packet{
	uint16_t source_port;
	uint16_t dest_port;
	uint16_t len;
	uint16_t checksum;
	char data[32];
};



struct ft_option{
	unsigned int first_port;
	unsigned int ttl_start;
	unsigned int ttl_max;
	unsigned int nb_queries_per_hop;
	unsigned int sim_queries;
	float timeout;

	unsigned int max_packet_to_receive;
};


struct ft_packet_stat{
	struct timeval time_sent;
	ft_packet_status status;
	// will be used as 'id' because each probe has a unique destination port
	unsigned int dest_port; 
};

uint16_t reverse_endian(uint16_t src)
{
	uint16_t rev;
	unsigned char buff[2];
	buff[0] = ((unsigned char *)&src)[1];
	buff[1] = ((unsigned char *)&src)[0];
	memcpy(&rev, &buff, 2);
	return rev;
}




void init_option(struct ft_option *option)
{
	option->first_port = 33434;
	option->ttl_max = 30;
	option->ttl_start = 1;
	option->nb_queries_per_hop = 3;
	option->sim_queries = 1;
	option->timeout = 5.0;
	// option->max_packet_to_receive = option->nb_queries_per_hop * (option->ttl_max - option->ttl_start  + 1);
	option->max_packet_to_receive = 4;
}


int create_udp_socket()
{
	int socket_fd = socket(AF_INET, SOCK_RAW, 17);
	// int socket_recv = socket(AF_INET, SOCK_DGRAM, 1);
	int val = 1;
	if (setsockopt(socket_fd, SOL_IP, IP_RECVERR, &val, sizeof(val)) == -1){
		dprintf(2,"Error setsocket option");
		close(socket_fd);
		exit(1);
	}
	if (socket_fd == -1){
		dprintf(2, "Error opening socket\n");
		exit (1);
	}
	return socket_fd;
}


struct ft_udp_packet create_udp_packet(unsigned int first_port)
{
	static unsigned int nb_probe_sent = 0;
	struct ft_udp_packet packet;
	unsigned int dest_port = first_port + nb_probe_sent;
	// max port number ==> if we go over traceroute start from port 0
	if (dest_port > 65535){
		dest_port = dest_port % 65535;
	}
	packet.source_port = 0;
	packet.checksum = 0;
	packet.len = sizeof(struct ft_udp_packet);
	packet.dest_port = dest_port;
	// filler data default is 32, we won't change it
	for (int i = 0;i < 32; i++)
		packet.data[i] = 0x40 + i;
	packet.len = reverse_endian(packet.len);
	packet.dest_port = reverse_endian(packet.dest_port);
	return packet;
}

struct ft_packet_stat* init_packet_stat(struct ft_option option)
{
	struct ft_packet_stat* packet_stat = malloc(option.max_packet_to_receive * sizeof(struct ft_packet_stat));
	if (packet_stat == NULL){
		dprintf(2, "Malloc error\n");
		return NULL;
	}
	for (size_t i = 0; i < option.max_packet_to_receive; ++i){
		packet_stat->status = not_sent;
	}
	return packet_stat;
}

ssize_t send_packet(int socket_fd, struct sockaddr *dest, size_t size_dest, unsigned int first_port,
					unsigned int ttl, struct ft_packet_stat *packet_stat)
{	
	struct ft_udp_packet packet = create_udp_packet(first_port);
	char buffer[100] = {0};
	memcpy(&buffer, &packet, sizeof(packet));
	setsockopt(socket_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
	ssize_t bytes_sent = sendto(socket_fd, buffer, sizeof(struct ft_udp_packet), 0,dest, size_dest);
	gettimeofday(&packet_stat->time_sent, 0);
	packet_stat->dest_port = packet.dest_port;
	return bytes_sent;
}

unsigned int update_probe_ttl(unsigned int current_packet_sent, struct ft_option option)
{
	unsigned int ttl_level = current_packet_sent % option.nb_queries_per_hop;
	unsigned int ttl = option.ttl_start + ttl_level;
	return ttl; 
}


void handle_recv_packet(struct ft_packet_stat *packet_stat, unsigned int max_packet, ssize_t buffer_size, char *raw_packet)
{
	//Our packet is raw ==> we have IP packet + ICMP packet
	// header len of ip is the second byte of the packet, it give us the len in bytes
	uint8_t header_len = (uint8_t) (raw_packet[0] & 0x0f) * 32 / 8;

	// icmp type is the first byte after the ip packet
	uint8_t icmp_type = raw_packet[header_len];
	// icmp code is just after
	uint8_t icmp_code = raw_packet[header_len + 1];

	// UDP packet start 8 bytes + header_len bytes after header_len ==> icmp apcket has 8 bytes of error + include ip_packet again AND THEN UDP

	unsigned int start_udp = header_len + 8 + header_len;
	// the dest_port is the 3-4th byte of UPD packet => our packet_stat id
	uint16_t dest_port = (uint16_t) raw_packet[start_udp + 2];
	printf("start udp %d dest port %x %x \n", start_udp,  raw_packet[start_udp + 2],  raw_packet[start_udp + 3]);
	for (int i = 0; i < buffer_size; ++i)
		printf("%x ", raw_packet[i]);
	printf("\n");
}

int main(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	struct ft_option option;
	init_option(&option);
	//udp	17	UDP		# user datagram protocol in /etc/protocols

	int socket_fd = create_udp_socket();
	struct ft_packet_stat *packet_stat = init_packet_stat(option);
	if (packet_stat == NULL){
		close(socket_fd);
		return 1;
	}

	struct sockaddr_in dest = {0};
	struct sockaddr src = {0};
	socklen_t len = sizeof(struct sockaddr);
	dest.sin_family = AF_INET;
	dest.sin_port = option.first_port;
	inet_pton(AF_INET, argv[1], &(dest.sin_addr));
	
	char buffer_recv[BUFFER_SIZE] = {0};
	ssize_t byte_recv = 0;

	unsigned int nb_packet_receive = 0;
	unsigned int current_packet_sent = 0;
	unsigned int packet_id = 0;
	while (nb_packet_receive < option.max_packet_to_receive)
	{
		// sending probe
		if (current_packet_sent < option.sim_queries){
			ssize_t byte_sent = send_packet(socket_fd, (struct sockaddr *) &dest, 
										sizeof(struct sockaddr_in), option.first_port,
										update_probe_ttl(current_packet_sent, option),
										&packet_stat[packet_id]);

			if (byte_sent == -1){
				dprintf(2, "Error sendto Quitting \n");
				break;
			}
			current_packet_sent ++;
			printf("packet sent current is %d\n", current_packet_sent);
			packet_id ++;
		}

		// reveiving
		// if we didn't send all our packet we check if socket has a msg with recvfrom NONBLOCKING ==> will return immediatly if no packet
		// so we can send the next one without waiting
		// If we sent all our packet we use select to wait for the next packet, the timeout will be the smallest one possible
		//  so option.timeout - oldest packet_sent
		if (current_packet_sent < option.sim_queries){
			byte_recv = recvfrom(socket_fd, buffer_recv, BUFFER_SIZE, MSG_DONTWAIT | MSG_ERRQUEUE, &src, &len);
			if (byte_recv == -1){
				if (errno == EAGAIN || errno == EWOULDBLOCK)
					continue;
				dprintf(2, "Error recvfrom quitting\n");
				break;
			}
			if (byte_recv == 0)
				continue;
			buffer_recv[byte_recv] = 0;
			current_packet_sent --;
		}
		// we can wait here :)
		else{
			fd_set readfd;
			FD_SET(socket_fd, &readfd);
			printf("WAITING FOR SELECT\n");
			int nb_fd_ready = select(socket_fd + 1, &readfd, NULL, NULL, NULL);
			printf("SELECT RETURNED\n");

			if (nb_fd_ready == -1){
				dprintf(2, "Error select quitting\n");
				break;
			}
			if (FD_ISSET(socket_fd, &readfd)){ // should always be true
				byte_recv = recvfrom(socket_fd, buffer_recv, BUFFER_SIZE | MSG_ERRQUEUE, 0, &src, &len);
				if (byte_recv <= 0){
					int err = errno;
					perror("ERROR:");
					dprintf(2, "Error recvfrom quitting after select got errno %d\n", err);
					break;
				}
				buffer_recv[byte_recv] = 0;
				current_packet_sent --;
			}
		}
		if (byte_recv > 0){
			nb_packet_receive ++;
			dprintf(1, "We have a packet nb :%d size is %zu:)\n", nb_packet_receive, byte_recv);
			handle_recv_packet(packet_stat, option.max_packet_to_receive, byte_recv, buffer_recv);
			// reset evrything
			byte_recv = 0;
			memset(buffer_recv,0, byte_recv);
		}
	}
	


	free(packet_stat);
	close(socket_fd);
	return 0;
}