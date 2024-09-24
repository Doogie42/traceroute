#include "traceroute.h"

ssize_t send_udp_packet(t_probe *probe, int socket_fd)
{
	struct ft_udp_packet packet;

	packet.dest_port = probe->dest_port;
	packet.checksum = 0;
	packet.len = sizeof(struct ft_udp_packet);
	packet.len = reverse_endian(packet.len);
	// filler data default is 32, we won't change it
	for (int i = 0; i < 32; i++)
		packet.data[i] = 0x40 + i;
	char buffer[100];
	if (set_ttl(socket_fd, probe->ttl) == -1)
	{
		perror("TTL:");
		return -1;
	}
	packet.source_port = probe->dest_port;
	probe->dest_port = reverse_endian(probe->dest_port);

	memcpy(buffer, &packet, sizeof(packet));
	ssize_t bytes_sent = sendto(socket_fd, buffer,
								sizeof(struct ft_udp_packet), 0, &probe->dest, sizeof(probe->dest));
	if (bytes_sent == -1)
	{
		perror("sendto ");
		return -1;
	}
	probe->status = sent;
	probe->time_sent = get_time_now();

	return bytes_sent;
}

ssize_t receive_icmp_error_probe(int socket_fd, t_probe *probe)
{
	struct sockaddr src;
	socklen_t len = sizeof(struct sockaddr_in);
	char buffer_recv[BUFFER_SIZE] = {0};

	ssize_t byte_recv = recvfrom(socket_fd, buffer_recv, BUFFER_SIZE, 0, &src, &len);
	if (byte_recv == -1)
	{
		printf("RECVFROM -1\n");
		return -1;
	}
	probe->time_recv = get_time_now();
	buffer_recv[byte_recv] = 0;

	// Our packet is raw ==> we have IP packet + ICMP packet
	//  header len of ip is the second byte of the packet, it give us the len in bytes
	uint8_t header_len = (uint8_t)(buffer_recv[0] & 0x0f) * 32 / 8;

	// icmp type is the first byte after the ip packet
	// uint8_t icmp_type = buffer_recv[header_len];
	// icmp code is just after
	// uint8_t icmp_code = buffer_recv[header_len + 1];

	// UDP packet start 8 bytes + header_len bytes after header_len ==> icmp apcket has 8 bytes of error + include ip_packet again AND THEN UDP

	unsigned int start_udp = header_len + 8 + header_len;

	// the dest_port is the 3-4th byte of UPD packet => our packet_stat id
	uint16_t dest_port = 0;
	if (start_udp + 2 >= BUFFER_SIZE)
	{
		return -1;
	}
	memcpy(&dest_port, &buffer_recv[start_udp + 2], 2);
	dest_port = reverse_endian(dest_port);
	// we received another ICMP packet not destined to us
	// Our id will be the dest_port
	if (dest_port != probe->dest_port)
		return 0;
	probe->status = received;

	if (getnameinfo(&src, sizeof(src), probe->resolved_host, NI_MAXHOST, 0, 0, 0) != 0)
		return 0;
	char *ip_reached = inet_ntoa((*(struct sockaddr_in *)(&src)).sin_addr);

	strncpy(probe->reached_ip, ip_reached, IP_LEN);

	return 0;
}