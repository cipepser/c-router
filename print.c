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

int PrintArp(struct ether_arp *arp, FILE *fp) {
  static char *hrd[] = {"From KA9Q: NET/ROM pseudo",
                        "Ethernet 10/100Mbps.",
                        "Experimental Ethernet.",
                        "AX.25 Level 2",
                        "PROnet token ring.",
                        "Chaosnet.",
                        "IEEE 802.2 Ethernet/TR/TB.",
                        "ARCnet",
                        "APPLEtalk.",
                        "undefine",
                        "undefine",
                        "undefine",
                        "undefine",
                        "undefine",
                        "undefine",
                        "Frame Relay DLCI.",
                        "undefine",
                        "undefine",
                        "undefine",
                        "ATM.",
                        "undefine",
                        "undefine",
                        "undefine",
                        "Metricom STRIP (new IANA id)."};
  static char *op[] = {"undefine",     "ARP request",  "ARP reply",
                       "RARP request", "RARP reply",   "undefine",
                       "undefine",     "undefine",     "InARP request",
                       "InARP reply",  "(ATM)ARP NAK."};

  char buf[80];

  fprintf(fp, "arp--------------------------------\n");
  fprintf(fp, "arp_hdr=%u\n", ntohs(arp->arp_hdr));
  if (ntohs(arp->arp_hdr) <= 23) {
    fprintf(fp, "(%s), ", hrd[ntohs(arp->arp_hdr)]);
  } else {
    fprintf(fp, "(undefined), ");
  }
  fprintf(fp, "arp_pro=%u\n", ntohs(arp->arp_pro));

  switch (ntohs(arp->arp_pro)) {
  case ETHERTYPE_IP:
    fprintf(fp, "(IP)\n");
    break;
  case ETHERTYPE_ARP:
    fprintf(fp, "(ARP)\n");
    break;
  case ETHERTYPE_IPV6:
    fprintf(fp, "(IPv6)\n");
    break;
  default:
    fprintf(fp, "(unknown)\n");
    break;
  }
  fprintf(fp, "arp_hln=%u\n", arp->arp_hln);
  fprintf(fp, "arp_pln=%u\n", arp->arp_pln);
  fprintf(fp, "arp_op=%u\n", ntohs(arp->arp_op));
  if (ntohs(arp->arp_op) <= 10) {
    fprintf(fp, "(%s)\n", op[ntohs(arp->arp_op)]);
  } else {
    fprintf(fp, "(undefine)\n");
  }
  fprintf(fp, "arp_sha%s\n", my_ether_ntoa_r(arp->arp_sha, buf, sizeof(buf)));
  fprintf(fp, "arp_spa%s\n", arp_ip2str(arp->arp_spa, buf, sizeof(buf)));
  fprintf(fp, "arp_tha%s\n", my_ether_ntoa_r(arp->arp_ha, buf, sizeof(buf)));
  fprintf(fp, "arp_tpa%s\n", arp_ip2str(arp->arp_tpa, buf, sizeof(buf)));

  return (0);
}

static char *Proto[] = {
    "undifined", "ICMP",      "IGMP",      "undifined", "IPIP",
    "undifined", "TCP",       "undifined", "EGP",       "undifined",
    "undifined", "undifined", "PUP",       "undifined", "undifined",
    "undifined", "undifined", "UDP",
};

int PrintIpHeader(struct iphdr *iphdr, u_char *option, int optionLen,
                  FILE *fp) {
  int i;
  char buf[80];

  fprintf(fp, "ip---------------------------------\n");
  fprintf(fp, "version=%u, ", iphdr->version);
  fprintf(fp, "ihl=%u, ", iphdr->ihl);
  fprintf(fp, "tos=%x, ", iphdr->tos);
  fprintf(fp, "tot_len=%u, ", ntohs(iphdr->tot_len));
  fprintf(fp, "id=%u, ", ntohs(iphdr->id));
  fprintf(fp, "frag_off=%x, %u, ", (ntohs(iphdr->frag_off) >> 13) & 0x07,
          ntohs(iphdr->frag_off) & 0x1FFF);
  fprintf(fp, "ttl=%u, ", iphdr->ttl);
  fprintf(fp, "protocol=%u, ", iphdr->protocol);
  if (iphdr->protocol <= 17) {
    fprintf(fp, "(%s), ", Proto[iphdr->protocol]);
  } else {
    fprintf(fp, "(undifined), ");
  }
  fprintf(fp, "check=%x\n", iphdr->check);
  fprintf(fp, "saddr=%s, ", ip_ip2str(iphdr->saddr, buf, sizeof(buf)));
  fprintf(fp, "daddr=%s, ", ip_ip2str(iphdr->saddr, buf, sizeof(buf)));
  if (optionLen > 0) {
    fprintf(fp, "option:");
    for (i = 0; i < optionLen; i++) {
      if (i != 0) {
        fprintf(fp, ":%02x", option[i]); // これ必要？
      } else {
        fprintf(fp, "%02x", option[i]);
      }
    }
  }

  return (0);
}

int PrintIp6Header(struct ip6_hdr, FILE *fp) {
  char buf[80];

  fprintf(fp, "ip6--------------------------------\n");
  fprintf(fp, "ip6_flow=%x, ", ip6->ip6_flow);
  fprintf(fp, "ip6_plen=%d, ", ntons(ip6->ip6_plen));
  fprintf(fp, "ip6_nxt=%u, ", ip6->ip6_nxt);
  if (ip6->ip6_nxt <= 17) {
    fprintf(fp, "(%s), ", Proto[ip6->ip6_nxt]);
  } else {
    fprintf(fp, "(undifined), ");
  }
  fprintf(fp, "ip6_hlim=%d, ", ip6->ip6_hlim);

  fprintf(fp, "ip6_src=%s\n",
          inet_ntop(AF_INET6, &ip6->ip6ip6_src, buf, sizeof(buf)));
  fprintf(fp, "ip6_dst=%s\n",
          inet_ntop(AF_INET6, &ip6->ip6ip6_dst, buf, sizeof(buf)));

  return (0);
}

int PrintIcmp(struct *icmp, FILE *fp) {
  static char *icmp_type[] = {"Echo Reply",
                              "undifined",
                              "undifined",
                              "Destination Unreachable",
                              "Source Quench",
                              "Redirect",
                              "undifined",
                              "undifined",
                              "Echo Request",
                              "Router Adverisment",
                              "Router Selection",
                              "Time Exceeded for Datagram",
                              "Parameter Problem on Datagram",
                              "Timestamp Request",
                              "Timestamp Reply",
                              "Information Request"
                              "Information Reply",
                              "Address Mask Request",
                              "Address Mask Reply"};

  fprintf(fp, "icmp-------------------------------\n");
  fprintf(fp, "icmp_type=%u, ", icmp->icmp_type);
  if (icmp->icmp_type <= 18) {
    fprintf(fp, "(%s), ", icmp_type[icmp->icmp_type]);
  } else {
    fprintf(fp, "(undifined), ");
  }

  fprintf(fp, "icmp_code=%u, ", icmp->icmp_code);
  fprintf(fp, "icmp_cksum=%u\n", ntohs(icmp->icmp_cksum));

  if (icmp->icmp_type == 0 || icmp->icmp_type == 8) {
    fprintf(fp, "icmp_id=%u, ", ntohs(icmp->icmp_id));
    fprintf(fp, "icmp_seq=%u, ", ntohs(icmp->icmp_seq));
  }

  return (0);
}

int PrintIcmp6(struct icmp6_hdr *icmp6, FILE *fp) {
  fprintf(fp, "icmp6------------------------------\n");
  fprintf(fp, "icmp6_type=%u, ", icmp6->icmp6_type);
  if (icmp->icmp6_type <= 1) {
    fprintf(fp, "(Destionation Unreachable), ", icmp6_type[icmp6->icmp6_type]);
  } else if (icmp->icmp6_type <= 1) {
    fprintf(fp, "(undifined), ");
  } else if (icmp->icmp6_type <= 2) {
    fprintf(fp, "(Packet too Big), ");
  } else if (icmp->icmp6_type <= 3) {
    fprintf(fp, "(Time Exceeded), ");
  } else if (icmp->icmp6_type <= 4) {
    fprintf(fp, "(Parameter Problem), ");
  } else if (icmp->icmp6_type <= 128) {
    fprintf(fp, "(Echo Request), ");
  } else if (icmp->icmp6_type <= 128) {
    fprintf(fp, "(Echo Reply), ");
  } else {
    fprintf(fp, "(undifined), ");
  }
  fprintf(fp, "icmp6_code=%u, ", icmp6->icmp6_code);
  fprintf(fp, "icmp6_cksum=%u\n", ntohs(icmp6->icmp6_cksum));

  if (icmp6->icmp6_type == 128 || icmp6->icmp6_type == 129) {
    fprintf(fp, "icmp6_id=%u, ", ntohs(icmp6->icmp6_id));
    fprintf(fp, "icmp6_seq=%u, ", ntohs(icmp6->icmp6_seq));
  }

  return (0);
}