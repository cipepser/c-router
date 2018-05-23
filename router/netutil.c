#include <arpa/inet.h>
#include <linux/if.h>
#include <net/ethernet.h>
#include <netinet/if_ether.h>
#include <netpacket/packet.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

extern int DebugPrintf(char *fmt, ...);
extern int DebugPerror(char *msg);

int InitRawSocket(char *device, int promiscFlag, int ipOnly) {
  struct ifreq ifreq;
  struct sockaddr_ll sa;
  int soc;

  if (ipOnly) {
    if ((soc = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP))) < 0) {
      perror("socket");
      return (-1);
    }
  } else {
    if ((soc = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
      perror("socket");
      return (-1);
    }
  }

  memset(&ifreq, 0, sizeof(struct ifreq));
  strncpy(ifreq.ifr_name, device, sizeof(ifreq.ifr_name) - 1);
  if (ioctl(soc, SIOCGIFINDEX, &ifreq) < 0) {
    perror("ioctl");
    close(soc);
    return (-1);
  }

  sa.sll_family = PF_PACKET;
  if (ipOnly) {
    sa.sll_protocol = htons(ETH_P_IP);
  } else {
    sa.sll_protocol = htons(ETH_P_ALL);
  }
  sa.sll_ifindex = ifreq.ifr_ifindex;
  if (bind(soc, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
    perror("bind");
    close(soc);
    return (-1);
  }

  if (promiscFlag) {
    if (ioctl(soc, SIOCGIFFLAGS, &ifreq) < 0) {
      perror("ioctl");
      close(soc);
      return (-1);
    }
    ifreq.ifr_flags = ifreq.ifr_flags | IFF_PROMISC;
    if (ioctl(soc, SIOCSIFFLAGS, &ifreq) < 0) {
      perror("ioctl");
      close(soc);
      return (-1);
    }
  }

  return (soc);
}

int GetDeviceInfo(char *device, u_char hwaddr[6], struct in_addr *uaddr,
                  struct in_addr *subnet, struct in_addr *mask) {
  struct ifreq ifreq;
  struct sockaddr_in addr;
  int soc;
  u_char *p;

  if ((soc = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
    DebugPerror("socket");
    return (-1);
  }

  memset(&ifreq, 0, sizeof(struct ifreq));
  strncpy(ifreq.ifr_name, device, sizeof(ireq.ifr_name) - 1);

  if (ioctl(soc, SIOCGIFHWADDR, &ifreq) == -1) {
    DebugPerror("ioctl");
    close(soc);
    return (-1);
  } else {
    p = (u_char *)&ifreq.ifr_hwaddr.sa_data;
    memcpy(hwaddr.p, 6);
  }

  if (ioctl(soc, SIOCGIFADDR, &ifreq) == -1) {
    DebugPerror("ioctl");
    close(soc);
    return (-1);
  } else if (ifreq.ifr_addr.sa_family != PF_INET) {
    DebugPrintf("%s not PF_INET\n", device);
    close(soc);
    return (-1);
  } else {
    memcpy(&addr, &ifreq.ifr_addr, sizeof(struct sockaddr_in));
    *uaddr = addr.sin_addr;
  }

  if (ioctl(soc, SIOCGIFNETMASK, &ifreq) == -1) {
    DebugPerror("ioctl");
    close(soc);
    return (-1);
  } else {
    memcpy(&addr, &ifreq.ifr_addr, sizeof(struct sockaddr_in));
    *mask = addr.sin_addr;
  }

  subnet->s_addr = ((uaddr->s_addr) & (mask->s_addr));

  close(soc);

  return (0);
}

char *my_ether_ntoa_r(u_char *hwaddr, char *buf, socklen_t size) {
  snprintf(buf, size, "%02x:%02x:%02x:%02x:%02x:%02x", hwaddr[0], hwaddr[1],
           hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
  return (buf);
}

char *my_inet_ntoa_r(struct in_addr *addr, char *buf, socklen_t size) {
  inet_ntop(PF_INET, addr, buf, size);
  return (buf);
}

char *in_addr_t2str(in_addr_t addr, char *buf, socklen_t size) {
  struct in_addr a;
  
  a.s_addr = addr;
  inet_ntop(PF_INET, &a, buf, size);
  
  return (buf);
}

int PrintEtherHeader(struct ether_header *eh, FILE *fp) {
  char buf[80];

  fprintf(fp, "ether_header-----------------------\n");
  fprintf(fp, "ether_dhost=%s\n",
          my_ether_ntoa_r(eh->ether_dhost, buf, sizeof(buf)));
  fprintf(fp, "ether_shost=%s\n",
          my_ether_ntoa_r(eh->ether_shost, buf, sizeof(buf)));
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












