#include "traceroute.h"

void error_exit(char *str)
{
	dprintf(2, "%s\n", str);
	exit(1);
}

uint16_t reverse_endian(uint16_t src)
{
	uint16_t rev;
	unsigned char buff[2];
	buff[0] = ((unsigned char *)&src)[1];
	buff[1] = ((unsigned char *)&src)[0];
	memcpy(&rev, &buff, 2);
	return rev;
}


double get_time_diff_ms(ft_time start, ft_time end)
{
	long diff_s = end.tv_sec -  start.tv_sec; 
	long diff_us = end.tv_usec -  start.tv_usec; 

	return ((double)diff_s * 1000.0 + (double) diff_us / 1000.0);
}

void ft_close(int fd)
{
	if (fd >= 0)
		close(fd);
}

ssize_t set_ttl(int socket_fd, int ttl)
{
	return setsockopt(socket_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
}

ft_time get_time_now()
{
	struct timeval now;
	gettimeofday(&now, 0);
	return now;
}
