#define _GNU_SOURCE

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
#include <sys/socket.h>
#include <netdb.h>
#include <stdbool.h>
#include <limits.h>


#define BUFFER_SIZE 1024
#define MAX_HOSTNAME_SIZE 254
#define IP_LEN 18
#define UDP_PROT 17

typedef struct timeval ft_time;

struct ft_option{
	unsigned int first_port;
	unsigned int ttl_start;
	unsigned int ttl_max;
	unsigned int nb_queries_per_hop;
	float timeout_s;

	unsigned int max_packet_to_receive;
	char target_ip [IP_LEN];
}; 

typedef enum ft_status{
	sent,
	received,
	not_received,
	received_timeout
}ft_status;


typedef struct ft_probe
{
	int socket_send;
	int socket_recv;
	ft_time time_sent;
	ft_time time_recv;
	unsigned int ttl;

	ft_status status;
	uint16_t dest_port;
	struct sockaddr dest_reached;
	struct sockaddr dest;

	char resolved_host[NI_MAXHOST];
	char reached_ip[18];

}t_probe;

struct ft_udp_packet{
	uint16_t source_port;
	uint16_t dest_port;
	uint16_t len;
	uint16_t checksum;
	char data[32];
};

struct ft_socket{
	int socket_send;
	int socket_recv;
	struct sockaddr_in dest;
};

char * init_option(struct ft_option *option, int argc, char **argv);

void error_exit(char *str);
uint16_t reverse_endian(uint16_t src);
double get_time_diff_ms(ft_time start, ft_time end);
void print_usage();
void ft_close(int fd);
ssize_t set_ttl(int socket_fd, int ttl);
ft_time get_time_now();

void init_socket(struct ft_socket *ft_socket, char *dest, char ip_address_string[IP_LEN]);
t_probe *init_probes(struct ft_option option, char *ip_dest);


ssize_t send_udp_packet(t_probe *probe, int socket_fd);
ssize_t receive_icmp_error_probe(int socket_fd , t_probe *probe);