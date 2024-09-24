#include "traceroute.h"

struct timeval get_select_timeout(double timeout_s)
{
	struct timeval timeout;
	timeout.tv_sec = (int)timeout_s;
	timeout.tv_usec = (timeout_s - (int)timeout_s) * 1000;
	return timeout;
}

void print_probe(struct ft_probe probe)
{
	static char last_ip[IP_LEN];

	if (probe.status != received)
	{
		printf("* ");
	}
	else
	{
		if (strcmp(last_ip, probe.reached_ip))
		{
			printf("%s (%s) ", probe.resolved_host, probe.reached_ip);
		}
		printf("%.3fms ", get_time_diff_ms(probe.time_sent, probe.time_recv));
	}
	strcpy(last_ip, probe.reached_ip);
}

void loop(t_probe *probe, struct ft_option option, struct ft_socket ft_socket, char *ip_dest)
{
	unsigned int current_probe = 0;
	unsigned int reached_ip_dest_count = 0;
	unsigned int hop_number = 0;
	while (current_probe < option.max_packet_to_receive)
	{
		if (send_udp_packet(&probe[current_probe], ft_socket.socket_send) == -1)
		{
			printf("Error sendto\n");
			break;
		}
		fd_set readfd;
		FD_ZERO(&readfd);
		FD_SET(ft_socket.socket_recv, &readfd);
		struct timeval timeout = get_select_timeout(option.timeout_s);

		int nfds = select(ft_socket.socket_recv + 1, &readfd, NULL, NULL, &timeout);
		if (nfds == -1)
		{
			perror("SELECT :");
			break;
		}
		if (nfds > 0)
		{
			receive_icmp_error_probe(ft_socket.socket_recv, &probe[current_probe]);
			// We have a raw socket so we will receive EVERY ICMP packet (from another ping programm for exemple)
			// We check that the ICMP packet is ours
			if (probe[current_probe].status != received)
				continue;
		}
		if (current_probe % option.nb_queries_per_hop == 0)
		{
			hop_number++;
			printf("\n%d. ", hop_number);
		}
		if (nfds == 0)
		{ // select timeout
			probe[current_probe].status = received_timeout;
		}
		print_probe(probe[current_probe]);

		if (!strcmp(probe[current_probe].reached_ip, ip_dest))
		{
			reached_ip_dest_count++;
		}
		if (reached_ip_dest_count >= option.nb_queries_per_hop)
			break;
		current_probe++;
	}
	printf("\n");
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		print_usage();
	}
	struct ft_option option;
	char *arg_dest = init_option(&option, argc, argv);

	struct ft_socket my_socket;
	char ip_dest[IP_LEN] = {0};
	init_socket(&my_socket, arg_dest, ip_dest);

	t_probe *probe = init_probes(option, ip_dest);
	if (probe == NULL)
	{
		ft_close(my_socket.socket_recv);
		ft_close(my_socket.socket_send);
		return 1;
	}
	printf("traceroute to %s (%s), %d hops max", argv[1], ip_dest, option.ttl_max - option.ttl_start + 1);

	loop(probe, option, my_socket, ip_dest);
	free(probe);
	ft_close(my_socket.socket_recv);
	ft_close(my_socket.socket_send);
	return 0;
}
