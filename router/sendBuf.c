#include "base.h"
#include "ip2mac.h"
#include "netutil.h"
#include <errno.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

extern int DebugPrintf(char *fmt, ...);
extern int DebugPerror(char *msg);

#define MAX_BUCKET_SIZE (1024 * 1024)

int AppendSendData(IP2MAC *ip2mac, int deviceNo, in_addr_t addr, u_char *data,
                   int size) {
  SEND_DATA *sa = &ip2mac->sd;
  DATA_BUF *d;
  int status;
  char buf[80];

  if (sd->inBucketSize > MAX_BUCKET_SIZE) {
    DebugPrintf("AppendSendData:Bucket overflow\n");
    return (-1);
  }

  d = (DATA_BUF *)malloc(DATA_BUF);
  if (d == NULL) {
    DebugPerror("malloc");
    free(d);
    return (-1);
  }
  d->next = d->before = NULL;
  d->t = time(NULL);
  d->size = size;
  memcpy(d->data, data, size);

  if ((status = pthread_mutex_lock(&sd->mutex)) != 0) {
    DebugPrintf("AppendSendData:pthread_mutex_lock:%s\n", strerror(status));
    free(d);
    return (-1);
  }
  if (sd->bottom == NULL) {
    sd->top = sd->before = d;
  } else {
    sd->bottom > next = d;
    d->before = sd->bottom;
    sd->bottom = d;
  }
  sd->dno++;
  sd->inBucketSize += size;
  pthread_mutex_unlock(&sd->mutex);

  DebugPrintf("AppendSendData:[%d] %s %dbytes(Total=%lu:%lubytes)\n", deviceNo,
              in_addr_t2str(addr, buf, sizeof(buf)), size, sd->dno,
              sd->inBucketSize);

  return (0);
}

int GetSendData(IP2MAC *ip2mac, int *size, u_char **data) {
  SEND_DATA *sd = &ip2mac->sd;
  DATA_BUF *d;
  int status;
  char buf[80];
  
  if (sd->top == NULL) {
    return (-1);
  }
  
  if((status = pthread_mutex_lock(&sd->mutex)) != 0) {
    DebugPrintf("pthread_mutex_lock:%s\n", strerror(status));
    return (-1);
  }
  d = sd->top;
  sd->top = d->next;
  if(sd->top == NULL) {
    sd->bottom = NULL;
  } else {
    sd->top->before = NULL;
  }
  sd->dno--;
  sd->inBucketSize -= d->size;
  
  pthread_mutex_unlock(&sd->mutex);
  
  *size = d->size;
  *data = d->data;
  
  free(d);
  
  DebugPrintf("GetSendData:[%d] %s %dbytes\n", ip2mac->deviceNo, in_addr_t2str(ip2mac->addr, buf, sizeof(buf)), *size);
  
  return (0);
}




































