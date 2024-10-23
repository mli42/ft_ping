#include "ft_ping.h"
#include "utils/icmp.h"
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

void signal_handler(int signum) {
  const double loss_percentage = (1 - (double)ping.stats.received / ping.stats.sent) * 100;

  switch (signum) {
  case SIGQUIT:
    printf("Quit\n");
    close(ping.sock_fd);
    exit(131);
  case SIGINT:
    printf("--- %s ping statistics ---\n", ping.hostname);
    close(ping.sock_fd);

    printf("%lu packets transmitted, %lu packets received, %.4g%% packet loss\n", ping.stats.sent, ping.stats.received, loss_percentage);
    printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n", ping.stats.min_rtt, ping.stats.total_rtt / ping.stats.received, ping.stats.max_rtt, ping.stats.max_rtt);
    exit(0);
  default:
    return ;
  }
}

void fill_payload(t_icmp_packet_payload *payload) {
  unsigned long int i;
  const char data[] = "fortytwo! ";
  const int data_len = strlen(data);

  if (gettimeofday(&payload->timeval, NULL) == -1) {
    fprintf(stderr, "%s: %s\n", ping.program_name, strerror(errno));
  }
  for (i = 0; i < PACKET_DATA_SIZE; i++) {
    payload->data[i] = data[i % data_len];
  }
  payload->data[i - 1] = '\0';
}

void fill_icmp_packet(t_icmp_packet *packet) {
  fill_payload(&packet->payload);
  packet->icmphdr.type = ICMP_ECHO;
  packet->icmphdr.un.echo.id = getpid();
  packet->icmphdr.un.echo.sequence = ping.stats.sent++;
  packet->icmphdr.checksum = checksum(packet, sizeof(*packet));
}

void ft_ping(__attribute__((unused)) int dummy) {
  t_icmp_packet packet;

  memset(&packet, 0, sizeof(packet));
  fill_icmp_packet(&packet);
  if (sendto(ping.sock_fd, &packet, sizeof(packet), 0, (sockaddr_t *)&ping.sockaddr, sizeof(ping.sockaddr)) == -1) {
    fprintf(stderr, "%s: sendto: %s\n", ping.program_name, strerror(errno));
  }
  alarm(1);
}
