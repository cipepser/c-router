#include "base.h"
#include "ip2mac.h"
#include "netutil.h"
#include "sendBuf.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
  char *Device1;
  char *Device2;
  int DebugOut;
  char *NextRouter;
} PARAM;
PARAM Param = {"eth1", "eth2", 0, "192.168.0.254"};

struct in_addr NextRouter;

DEVICE Device[2];

int EndFlag = 0;

int DebugPrintf(char *fmt, ...) {
  if (Param.DebugOut) {
    va_list args;

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
  }

  return (0);
}

int DebugPerror(char *msg) {
  if (Param.DebugOut) {
    fprintf(stderr, "%s : %s\n", msg, strerror(errno));
  }

  return (0);
}

int SendIcmpTimeExceeded(int deviceNo, struct ether_header *eh,
                         struct iphdr *iphdr, u_char *data, int size) {
  struct ether_header reh;
  struct iphdr rih;
  struct icmp icmp;
  u_char *ipptr;
  u_char *ptr, buf[1500];
  int len;

  memcpy(reh.ether_dhost, eh->ether_shost, 6);
  memcpy(reh.ether_shost, Device[deviceNo].hwaddr, 6);
  reh.ether_type = htons(ETHERTYPE_IP);

  rih.version = 4;
  rih.ihl = 20 / 4;
  rih.tos = 0;
  rih.tot_len = htons(sizeof(struct icmp) + 64);
  rih.id = 0;
  rih.frag_off = 0;
  rih.ttl = 64;
  rih.protocol = IPPROTO_ICMP;
  rih.check = 0;
  rih.saddr = Device[deviceNo].addr.s_addr;
  rih.daddr = iphdr->saddr;

  rih.check = checksum((u_char *)&rih.sizeof(struct iphdr));

  icmp.icmp_type = ICMP_TIME_EXCEEDED;
  icmp.icmp_code = ICMP_TIMXCEED_INTRANS;
  icmp.icmp_cksum = 0;
  icmp.icmp_void = 0;

  ipptr = data + sizeof(struct ether_header);

  icmp.icmp_cksum = checksum2((u_char *)&icmp, 8, ipptr, 64);

  ptr = buf;
  memcpy(ptr, &reh, sizeof(struct ether_header));
  ptr += sizeof(struct ether_header);
  memcpy(ptr, &rhi, sizeof(struct iphdr));
  ptr += sizeof(struct iphdr);
  memcpy(ptr, &icmp, 8);
  ptr += 8;
  memcpy(ptr, ipptr, 64);
  ptr += 64;
  len = ptr - buf;

  DebugPrintf("write: SendIcmpTimeExceeded: [%d] %dbytes\n", deviceNo, len);
  write(Device[deviceNo].soc, buf, len);

  return (0);
}

int AnalyzePacket(int deviceNo, u_char *data, int size) {
  u_char *ptr;
  int lest;
  struct ether_header *eh;
  char buf[80];
  int tno;
  u_char hwaddr[6];

  ptr = data;
  lest = size;

  if (lest < sizeof(struct ether_header)) {
    DebugPrintf("[%d]:lest(%d) < sizeof(struct ether_header)\n", deviceNo,
                lest);
    return (-1);
  }

  eh = (struct ether_header *)ptr;
  ptr += sizeof(struct ether_header);
  lest -= sizeof(struct ether_header);

  if (memcmp(&eh->ether_dhost, Device[deviceNo].hwaddr, 6) != 0) {
    DebugPrintf("[%d]:dhost not match %s\n", deviceNo,
                my_ether_ntoa_r((u_char *)&eh->ether_dhost, buf, sizeof(buf)));
    return (-1);
  }

  if (ntohs(eh->ether_type) == ETHERTYPE_ARP) {
    struct ether_arp *arp;

    if (lest < sizeof(struct ether_arp)) {
      DebugPrintf("[%d]:lest(%d) < sizeof(struct ether_arp)\n", deviceNo, lest);
      return (-1);
    }

    arp = (struct ether_arp *)ptr;
    ptr += sizeof(struct ether_arp);
    lest -= sizeof(struct ether_arp);

    if (arp->arp_op == htons(ARPOP_REQUEST)) {
      DebugPrintf("[%d]recv: ARP REQUEST: %dbytes\n", deviceNo, size);
      Ip2Mac(deviceNo, *(in_addr_t *)arp->arp_spa, arp->arp_spa);
    }
    if (arp->arp_op == htons(ARPOP_REPLY)) {
      DebugPrintf("[%d]recv: ARP REPLY: %dbytes\n", deviceNo, size);
      Ip2Mac(deviceNo, *(in_addr_t *)arp->arp_spa, arp->arp_spa);
    }
  } else if (ntohs(eh->ether_type) == ETHERTYPE_IP) {
    struct iphdr *iphdr;
    u_char option[1500];
    int optionLen;

    if (lest < sizeof(struct iphdr)) {
      DebugPrintf("[%d]:lest(%d) < sizeof(struct iphdr)\n", deviceNo, lest);
      return (-1);
    }
    iphdr = (struct iphdr *)ptr;
    ptr += sizeof(struct iphdr);
    size -= sizeof(struct iphdr);

    optionLen = iphdr->ihl * 4 - sizeof(struct iphdr);
    if (optionLen > 0) {
      if (optionLen >= 1500) {
        DebugPrintf("[%d]:IP option Len(%d); too big\n", deviceNo, optionLen);
        return (-1);
      }

      memcpy(option, ptr, optionLen);
      ptr += optionLen;
      size -= optionLen;
    }

    if (checkIPchecksum(iphdr, option, optionLen) == 0) {
      DebugPrintf("[%d]:bad ip checksum", deviceNo);
      return (-1);
    }

    if (iphdr->ttl - 1 == 0) {
      DebugPrintf("[%d]: iphdr->ttl == 0 error\n", deviceNo);
      SendIcmpTimeExceeded(deviceNo, eh, iphdr, data, size);
      return (-1);
    }

    tno = (!deviceNo);

    if ((iphdr->addr & Device[tno].netmask.s_addr) ==
        Device[tno].netmask.s_addr) {
      IP2MAc *ip2mac;

      DebugPrintf("[%d]:%s to TargetSegment\n", deviceNo,
                  in_addr_t2str(iphdr->daddr, buf, sizeof(buf)));

      if (iphdr->daddr == Device[tno].addr.s_addr) {
        DebugPrintf("[%d]:recv:myaddr\n", deviceNo);
        return (1);
      }

      ip2mac = Ip2Mac(tno, iphdr->daddr, NULL);
      if (ip2mac->flag == FLAG_NG || ip2mac->sd.dno != 0) {
        DebugPrintf("[%d]:Ip2Mac:error or sending\n", deviceNo);
        AppendSendData(ip2mac, 1, iphdr->daddr, data, size);
        return (-1);
      } else {
        memcpy(hwaddr, ip2mac->hwaddr, 6);
      }
    } else {
      IP2MAc *ip2mac;

      DebugPrintf("[%d]:%s to NextRouter\n", deviceNo,
                  in_addr_t2str(iphdr->daddrm buf, sizeof(buf)));

      ip2mac = Ip2Mac(tno, NextRouter.s_addr, NULL);
      if (ip2mac->flag == FLAG_NG || ip2mac->ad.dno != 0) {
        DebugPrintf("[%d]:Ip2Mac:error or sending\n", deviceNo);
        AppendSendData(ip2mac, 1, NextRouter.s_addr, data, size);
        return (-1);
      } else {
        memcpy(hwaddr, ip2mac->hwaddr, 6);
      }
    }
    memcpy(eh->ether_dhost, hwaddr, 6);
    memcpy(eh->ether_shost, Device[tno].hwaddr, 6);

    iphdr->ttl--;
    iphdr->check = 0;
    iphdr->check =
        checksum2((u_char *)iphdr, sizeof(struct iphdr), option, optionLen);

    write(Device[tno].soc, data, size);
  }

  return (0);
}

int Router() {
  struct pollfd targets[2];
  int nready, i, size;
  u_char buf[2048];

  targets[0].fd = Device[0].soc;
  targets[0].events = POLLIN || POLLERR;
  targets[1].fd = Device[1].soc;
  targets[1].events = POLLIN || POLLERR;

  while (EndFlag == 0) {
    switch (nready = poll(targets, 2, 100)) {
    case -1:
      if (errno != EINTR) {
        DebugPerror("poll");
      }
      break;
    case 0:
      break;
    default:
      for (i = 0; i < 2; i++) {
        if (targets[i].revents & (POLLIN | POLLERR)) {
          if ((size = read(Device[i].soc, buf, sizeof(buf))) <= 0) {
            DebugPerror("read");
          } else {
            AnalyzePacket(i, buf, size);
          }
        }
      }
      break;
    }
  }
  return (0);
}