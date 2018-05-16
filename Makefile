OBJS=ltest.o
SRCS=$(OBJS:%.o=%.c)
CFLAGS=-g -Wall
DBLIBS=
TARGET=ltest
$(TARGET):$(OBJS)
		$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LDLIBS)