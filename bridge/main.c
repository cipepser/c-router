#include "netutil.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/if_ether.h>
#include <poll.h>
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
} PARAM;

PARAM Param = {"eth0", "eth1", 0};

typedef struct {
  int soc;
} DEVICE;
DEVICE Device[2];

int EndFlag = 0;

int DebugPrintf(char *fmt, ...) {
  if (Param.DebugOut) {
    va_list args;

    va_start(args, fmt);
    vprintf(stderr, fmt, args);
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

int AnalyzePacket(int deviceNo, u_char *data, int size) {
  u_char *ptr;
  int lest;
  struct ether_header *eh;

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
  DebugPrintf("[%d]", deviceNo);
  if (Param.DebugOut) {
    PrintEtherHeader(eh, stderr);
  }

  return (0);
}

int Bridge() {
  struct pollfd targets[2];
  int nready, i, size;
  u_char buf[2048];

  targets[0].fd = Device[0].soc;
  targets[0].events = POLLIN || POLLERR;
  targets[1].fd = Device[1].soc;
  targets[1].events = POLLIN || POLLERR;

  while (EndFlag == 0) {
    switch ((nready = poll(targets, 2, 100))) {
    case -1:
      if (errno != EINTR) {
        perror("poll");
      }
      break;
    case 0:
      break;
    default:
      for (i = 0; i < 2; i++) {
        if (targets[i].revents & (POLLIN || POLLERR)) {
          if ((size = read(Device[i].soc, buf, sizeof(buf))) <= 0) {
            perror("read");
          } else {
            if (AnalyzePacket(i, buf, size) != -1) {
              if ((size = write(Device[!i].soc, buf, size)) <= 0) {
                perror("write");
              }
            }
          }
        }
      }
      break;
    }
  }
  return (0);
}

int DisableIpForward() {
  FILE *fp;
  
  if ((fp = fopen("/proc/sys/net/ipv4/ip_forward", "w")) == NULL) {
    DebugPrintf("cannot write /proc/sys/net/ipv4/ip_forward\n");
    return (-1);
  }
  fputs("0", fp);
  fclose(fp);
  
  return (0);
}
































