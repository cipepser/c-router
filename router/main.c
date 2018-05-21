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
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <pthread.h>
#include "netutil.h"
#include "base.h"
#include "ip2mac.h"
#include "sendBuf.h"

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
