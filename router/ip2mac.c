#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>
#include "netutil.h"
#include "base.h"
#include "ip2mac.h"
#include "sendBuf.h"

extern int DebugPrintf(char *fmt, ...);

#define IP2MAC_TIMEOUT_SEC 60
#define IP2MAC_NG_TIMEOUT_SEC 1

struct {
  IP2MAC *data;
  int size;
  int no;
} Ip2Macs[2];

extern DEVICE Device[2];
extern int ArpSoc[2];

extern int EndFlag;
