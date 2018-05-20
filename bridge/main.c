#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include "netutil.h"

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
    DebugPrintf("[%d]:lest(%d) < sizeof(struct ether_header)\n", deviceNo, lest);
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