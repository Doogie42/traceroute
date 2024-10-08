# README

## ft_traceroute

ft_traceroute is a simple implementation of the traceroute utility in C. It allows you to trace the route of packets to a specified destination and measure the round-trip time (RTT) for each hop.

## Features

- Trace the route of packets to a specified destination.
- Measure the round-trip time (RTT) for each hop.
- Display the IP address and hostname of each hop.
- Set custom options such as first TTL, maximum TTL, and number of queries per hop.

## Usage

```
sudo ./ft_traceroute [options] <destination>
```

Options:

- `-f <first_ttl>`: Set the first TTL value for the packets. Defaults to 1.
- `-m <max_ttl>`: Set the maximum TTL value for the packets. Defaults to 30.
- `-p <port>`: Set the destination port number for the packets. The default port number will be incremented by each probe.
- `-q <nqueries>`: Set the number of probe packets per hop. The default is 3.

## Build

To build the ft_traceroute executable, run the following command:

```
git clone https://github.com/Doogie42/ft_traceroute.git
cd ft_traceroute
make
```