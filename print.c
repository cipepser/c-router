#include <arpa/inet.h>
#include <linux/if.h>
#include <net/ethernet.h>
#include <netinet/icmp6.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netpacket/packet.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef ETHERTYPE_IPV6
#define ETHERTYPE_IPV6 0x86dd
#endif

char *my_ether_ntoa_r(u_char *hwaddr, char *buf, socklen_t size) {
  snprintf(buf, size, "%02x:%02x:%02x:%02x:%02x:%02x", hwaddr[0], hwaddr[1],
           hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
  return (buf);
}

char *arp_ip2str(u_int8_t *ip, char *buf, socklen_t size) {
  snprintf(buf, size, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  return (buf);
}

char *ip_ip2str(u_int32_t ip, char *buf, socklen_t size) {
  struct in_addr *addr;
  addr = (struct in_addr *)&ip;
  inet_ntop(AF_INET, addr, buf, size);

  return (buf);
}

int PrintEtherHeader(struct ether_header *eh, FILE *fp) {
  char buf[80];

  fprintf(fp, "ether_header-----------------------\n");
  fprintf(fp, "ether_dhost=%s\n",
          my_ether_ntoa_r(en->ether_dhost, buf, sizeof(buf)));
  fprintf(fp, "ether_shost=%s\n",
          my_ether_ntoa_r(en->ether_shost, buf, sizeof(buf)));
  fprintf(fp, "ether_type=%02X\n", ntohs(eh->ether_type));

  switch (ntohs(eh->ether_type)) {
  case ETH_P_IP:
    fprintf(fp, "(IP)\n");
    break;
  case ETH_P_IPV6:
    fprintf(fp, "(IPv6)\n");
    break;
  case ETH_P_ARP:
    fprintf(fp, "(ARP)\n");
    break;
  default:
    fprintf(fp, "(unknown)\n");
    break;
  }

  return (0);
}