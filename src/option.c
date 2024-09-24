#include "traceroute.h"

int read_input(char *input, char *err_msg, int min_value, int max_value)
{
	long val = strtol(input, NULL, 10);

	if (val == LONG_MAX || val == LONG_MIN || val < min_value || val > max_value)
	{
		error_exit(err_msg);
	}
	return (int)val;
}

void print_usage()
{
	printf("NAME\n\
	ft_traceroute - print the route packets trace to network host\n\
SYNOPSIS\n\
	traceroute  [-f first_ttl]   [-m max_ttl]\n \
				[-p port] [-q nqueries]\n \
OPTIONS\n\
	-h Print help info and exit.\n\
	-p port\n\
		For UDP tracing, specifies the destination port base traceroute will use (the  destina‐\n\
		tion port number will be incremented by each probe).\n\
	-q nqueries\n\
		Sets the number of probe packets per hop. The default is 3.\n\
	-m max_ttl\n\
		Specifies  the  maximum  number of hops (max time-to-live value) traceroute will probe.\n\
		The default is 30.\n\
	-f first_ttl\n\
		Specifies with what TTL to start. Defaults to 1.\n\
");
	exit(0);
}

char *init_option(struct ft_option *option, int argc, char **argv)
{
	option->first_port = 33434;
	option->ttl_max = 30;
	option->ttl_start = 1;
	option->nb_queries_per_hop = 3;
	option->timeout_s = 1.1;
	option->max_packet_to_receive = option->nb_queries_per_hop * (option->ttl_max - option->ttl_start + 1);
	int i = 1;
	// we cant use opt arg because of the subject
	if (!strcmp(argv[1], "-h"))
		print_usage();
	for (i = 1; i < argc - 1; i += 2)
	{
		if (argv[i][0] != '-')
		{
			error_exit("Please use a '-' before flag");
		}
		if (argv[i][1] != 0 && argv[i][2] != 0)
		{
			error_exit("Flag must be 1 letter\n");
		}
		switch (argv[i][1])
		{
		case 'f':
		{
			option->ttl_start = read_input(argv[i + 1], "TTL start: wrong number => between 1 and 256", 1, 256);
			break;
		}
		case 'm':
		{
			option->ttl_max = read_input(argv[i + 1], "TTL end: wrong number => between 1 and 256", 1, 256);
			break;
		}
		case 'q':
		{
			option->nb_queries_per_hop = read_input(argv[i + 1], "query per hop: wrong number => between 1 and 256", 1, 256);
			break;
		}
		case 'p':
		{
			option->first_port = read_input(argv[i + 1], "first port: wrong number => between 1000 and 65535", 1000, 65535);
			break;
		}
		case 'h':
		{
			print_usage();
		}
		default:
			printf("Error unknown flag %c\n", argv[i][1]);
			exit(1);
			break;
		}
	}
	if (option->ttl_start >= option->ttl_max)
	{
		error_exit("TTL start is bigger than ttl end");
	}
	if (i == argc)
	{
		error_exit("Missing argument");
	}
	return argv[argc - 1];
}
