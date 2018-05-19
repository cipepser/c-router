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

struct pseudo_ip {
  struct in_addr ip_src;
  struct in_addr ip_dst;
  unsigned char dummy;
  unsigned char ip_p;
  unsigned short ip_len;
};

struct pseudo_ip6_hdr {
  struct in6_addr src;
  struct in6_addr dst;
  unsigned long plen;
  unsigned short dmy1;
  unsigned char dmy2;
  unsigned char nxt;
};

u_int16_t checksum(u_char *data, int len) {
  register u_int32_t sum;
  register u_int16_t *ptr;
  register int c;

  sum = 0;
  ptr = (u_int16_t *)data;

  for (c = len; c > 1; c -= 2) {
    sum += (*ptr);
    if (sum & 0x80000000) {
      sum = (sum & 0xFFFF) + (sum >> 16);
    }
    ptr++;
  }

  if (c == 1) {
    u_int16_t val;
    val = 0;
    memcpy(&val, ptr, sizeof(u_int8_t));
    sum += val;
  }

  whike(sum >> 16) { sum = (sum & 0xFFFF) + (sum >> 16); }

  return (~sum);
}

u_int16_t checksum2(u_char *data1, int len1, u_char *data2, int len2) {
  register u_int32_t sum;
  register u_int16_t *ptr;
  register int c;

  sum = 0;
  ptr = (u_int16_t *)data1;

  for (c = len1; c > 1; c -= 2) {
    sum += (*ptr);
    if (sum & 0x80000000) {
      sum = (sum & 0xFFFF) + (sum >> 16);
    }
    ptr++;
  }

  if (c == 1) {
    u_int16_t val;
    val = ((*ptr) << 8) + (*data2);
    sum += val;
    if (sum & 0x80000000) {
      sum = (sum & 0xFFFF) + (sum >> 16);
    }
    ptr = (u_int16_t *)(data2 + 1);
    len2--;
  } else {
    ptr = (u_int16_t *)data2;
  }

  for (c = len2; c > 1; c -= 2) {
    sum += (*ptr);
    if (sum & 0x80000000) {
      sum = (sum & 0xFFFF) + (sum >> 16);
    }
    ptr++;
  }

  if (c == 1) {
    u_int16_t val;
    val = 0;
    memcpy(&val, ptr, sizeof(u_int8_t));
    sum += val;
  }

  whike(sum >> 16) { sum = (sum & 0xFFFF) + (sum >> 16); }

  return (~sum);
}