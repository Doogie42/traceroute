#include "traceroute.h"

void init_socket(struct ft_socket *ft_socket, char *dest, char ip_address_string[IP_LEN])
{
	struct 	addrinfo hints;
	struct addrinfo *results, *rp;
	int socket_fd;
	
	bzero(&ft_socket->dest, sizeof(ft_socket->dest));
	ft_socket->dest.sin_family = AF_INET;
	ft_socket->dest.sin_port = AF_INET;
	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = UDP_PROT;
	if (getaddrinfo(dest, NULL, &hints, &results) != 0){
		printf("traceroute : %s: Temporary failure in name resolution\n", dest);
		exit(1);
	}
	if (results == NULL)
		error_exit("NO info found for host");
	
	for(rp = results; rp != NULL; rp = results->ai_next){
		socket_fd = socket(rp->ai_family, rp->ai_socktype,
                            UDP_PROT);
		if (socket_fd == -1){
			continue;
		}
		else{
			break;
		}
	}

	char *ip_address = inet_ntoa(((struct sockaddr_in *)rp->ai_addr)->sin_addr);
	freeaddrinfo(results);

	if (inet_pton(AF_INET,ip_address, &(ft_socket->dest.sin_addr)) != 1){
		error_exit("Wrong ip");
	}
	strncpy(ip_address_string, ip_address, IP_LEN);
	ft_socket->socket_send = socket_fd;
	ft_socket->socket_recv = socket(AF_INET, SOCK_RAW | SOCK_NONBLOCK, IPPROTO_ICMP);
	if (ft_socket->socket_send == -1 || ft_socket->socket_recv == -1){
		ft_close(ft_socket->socket_send);
		ft_close(ft_socket->socket_recv);
		error_exit("Socket error");
	}
}

t_probe *init_probes(struct ft_option option, char *ip_dest)
{
	struct sockaddr_in dest;
	dest.sin_family = AF_INET;
	if (inet_pton(AF_INET,ip_dest, &(dest.sin_addr)) != 1){
		perror("inet");
		return NULL;
	}
	
	t_probe *probe = malloc(option.max_packet_to_receive * sizeof(t_probe));
	if (!probe)
		return NULL;

	for (unsigned int i = 0; i < option.max_packet_to_receive; ++i){
		probe[i].dest_port = option.first_port + i;
		while (probe[i].dest_port > 65535){
			probe[i].dest_port -= 65535;
		}
		probe[i].dest_port = reverse_endian(probe[i].dest_port);
		dest.sin_port = probe[i].dest_port;
		memcpy(&probe[i].dest, &dest, sizeof(struct sockaddr_in));
		probe[i].status = not_received;
		probe[i].ttl = i / option.nb_queries_per_hop + option.ttl_start;
		memset(probe[i].reached_ip, 0, IP_LEN);
		memset(probe[i].resolved_host, 0, NI_MAXHOST);
	}
	return probe;
}

