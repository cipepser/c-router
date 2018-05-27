# 自作ルータを試す環境を構築する旅路

## 環境

### ホストOS

```sh
$ ifconfig
eth0      Link encap:Ethernet  HWaddr 08:00:27:2D:82:3A
          inet addr:10.0.2.15  Bcast:10.0.2.255  Mask:255.255.255.0
          inet6 addr: fe80::a00:27ff:fe2d:823a/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:1192 errors:0 dropped:0 overruns:0 frame:0
          TX packets:757 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:101743 (99.3 KiB)  TX bytes:81218 (79.3 KiB)

eth1      Link encap:Ethernet  HWaddr 08:00:27:60:F3:40
          inet addr:192.168.0.129  Bcast:192.168.0.255  Mask:255.255.255.0
          inet6 addr: fe80::a00:27ff:fe60:f340/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:533 errors:0 dropped:0 overruns:0 frame:0
          TX packets:24 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:118276 (115.5 KiB)  TX bytes:1746 (1.7 KiB)

eth2      Link encap:Ethernet  HWaddr 08:00:27:6C:79:35
          inet addr:192.168.1.130  Bcast:192.168.1.255  Mask:255.255.255.0
          inet6 addr: fe80::a00:27ff:fe6c:7935/64 Scope:Link
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:533 errors:0 dropped:0 overruns:0 frame:0
          TX packets:20 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:118276 (115.5 KiB)  TX bytes:1496 (1.4 KiB)

lo        Link encap:Local Loopback
          inet addr:127.0.0.1  Mask:255.0.0.0
          inet6 addr: ::1/128 Scope:Host
          UP LOOPBACK RUNNING  MTU:65536  Metric:1
          RX packets:6 errors:0 dropped:0 overruns:0 frame:0
          TX packets:6 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0
          RX bytes:560 (560.0 b)  TX bytes:560 (560.0 b)
```

## いろいろ試してみる

nsを作る

```sh
$ sudo ip netns list
$ sudo ip netns add testns
$ sudo ip netns list
testns
```

nsに入る

```sh
$ sudo ip netns exec testns bash
```

loをupする

```sh
ip link set lo up
```

vethを追加する(defaultでveth0, veth1が作られる)

```sh
ip link add type veth
```

linkの状態を見てみる。
`lo`は`up`していて、他は`down`している。
MACアドレスしかないのを見ればわかるが、L2 I/Fがlinkの相当するんだと思う。
ちなみにhost OSのほうで見てみると`ip link list`と`ifconfig`のMACは一致する。

```sh
[root@localhost router]# ip link list
5: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
6: veth0: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN qlen 1000
    link/ether 96:3c:b6:b8:7a:26 brd ff:ff:ff:ff:ff:ff
7: veth1: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN qlen 1000
    link/ether 3a:d6:c7:a2:16:90 brd ff:ff:ff:ff:ff:ff
```
この状態で`ifconfig`を見てみると`lo`しかない

```sh
[root@localhost router]# ifconfig
lo        Link encap:Local Loopback
          inet addr:127.0.0.1  Mask:255.0.0.0
          inet6 addr: ::1/128 Scope:Host
          UP LOOPBACK RUNNING  MTU:65536  Metric:1
          RX packets:4 errors:0 dropped:0 overruns:0 frame:0
          TX packets:4 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0
          RX bytes:336 (336.0 b)  TX bytes:336 (336.0 b)
```

`veth0`を`up`させてから、`ifconfig`を見ると`veth0`が増えている。ただし、IPアドレスは振っていないので、L2 I/Fでしかない。

```sh
[root@localhost router]# ip link set veth0 up
[root@localhost router]# ifconfig
lo        Link encap:Local Loopback
          inet addr:127.0.0.1  Mask:255.0.0.0
          inet6 addr: ::1/128 Scope:Host
          UP LOOPBACK RUNNING  MTU:65536  Metric:1
          RX packets:4 errors:0 dropped:0 overruns:0 frame:0
          TX packets:4 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0
          RX bytes:336 (336.0 b)  TX bytes:336 (336.0 b)

veth0     Link encap:Ethernet  HWaddr 96:3C:B6:B8:7A:26
          UP BROADCAST MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:0 (0.0 b)  TX bytes:0 (0.0 b)
```

ip(`192.168.3.1/24`)を振ってみる。

```sh
[root@localhost router]# ip addr add 192.168.3.1/24 dev veth0
[root@localhost router]# ifconfig
lo        Link encap:Local Loopback
          inet addr:127.0.0.1  Mask:255.0.0.0
          inet6 addr: ::1/128 Scope:Host
          UP LOOPBACK RUNNING  MTU:65536  Metric:1
          RX packets:4 errors:0 dropped:0 overruns:0 frame:0
          TX packets:4 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0
          RX bytes:336 (336.0 b)  TX bytes:336 (336.0 b)

veth0     Link encap:Ethernet  HWaddr 96:3C:B6:B8:7A:26
          inet addr:192.168.3.1  Bcast:0.0.0.0  Mask:255.255.255.0
          UP BROADCAST MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:0 (0.0 b)  TX bytes:0 (0.0 b)
```

### ホストOS側でも設定する

```sh
[vagrant@localhost router]$ sudo ip link add type veth
[vagrant@localhost router]$ ip link list
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 08:00:27:2d:82:3a brd ff:ff:ff:ff:ff:ff
3: eth1: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 08:00:27:60:f3:40 brd ff:ff:ff:ff:ff:ff
4: eth2: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 08:00:27:6c:79:35 brd ff:ff:ff:ff:ff:ff
8: veth0: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN qlen 1000
    link/ether 5e:62:22:12:a9:49 brd ff:ff:ff:ff:ff:ff
9: veth1: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN qlen 1000
    link/ether 06:94:18:2f:7a:de brd ff:ff:ff:ff:ff:ff
```

```sh
[vagrant@localhost router]$ sudo ip link set veth0 up
[vagrant@localhost router]$ sudo ip addr add 192.168.3.254/24 dev veth0
[vagrant@localhost router]$ ifconfig
(略)
veth0     Link encap:Ethernet  HWaddr 5E:62:22:12:A9:49
          inet addr:192.168.3.254  Bcast:0.0.0.0  Mask:255.255.255.0
          UP BROADCAST MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:0 (0.0 b)  TX bytes:0 (0.0 b)
```

nsが異なるので、通信できない。

```sh
[vagrant@localhost router]$ ping 192.168.3.1
PING 192.168.3.1 (192.168.3.1) 56(84) bytes of data.
From 192.168.3.254 icmp_seq=1 Destination Host Unreachable
From 192.168.3.254 icmp_seq=2 Destination Host Unreachable
From 192.168.3.254 icmp_seq=3 Destination Host Unreachable
^C
--- 192.168.3.1 ping statistics ---
5 packets transmitted, 0 received, +3 errors, 100% packet loss, time 4011ms
pipe 3
```

### 同じnsに所属させたい

このまま追加しようとしても怒られる。

```sh
[vagrant@localhost router]$ sudo ip link set veth0 netns testns
RTNETLINK answers: File exists
```

一度消す。これだとうまくいかなかった。

```sh
[vagrant@localhost router]$ sudo ip link delete veth0
```


## 新しいpairで試す

事前状態(host側)

```sh
[vagrant@localhost router]$ ip link show
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 08:00:27:2d:82:3a brd ff:ff:ff:ff:ff:ff
3: eth1: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 08:00:27:60:f3:40 brd ff:ff:ff:ff:ff:ff
4: eth2: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 08:00:27:6c:79:35 brd ff:ff:ff:ff:ff:ff
16: veth0: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN qlen 1000
    link/ether 7a:09:4d:bf:05:dd brd ff:ff:ff:ff:ff:ff
17: veth1: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN qlen 1000
    link/ether 9e:64:a0:26:2c:ab brd ff:ff:ff:ff:ff:ff
18: veth2: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN qlen 1000
    link/ether 52:61:c1:6d:f8:3e brd ff:ff:ff:ff:ff:ff
19: veth3: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN qlen 1000
    link/ether 66:c3:ff:12:43:64 brd ff:ff:ff:ff:ff:ff
```

`veth2`を`testns`に入れる。

```sh
[vagrant@localhost router]$ sudo ip link set veth2 netns testns
```

host側では`veth2`が見えなくなる。

```sh
[vagrant@localhost router]$ sudo ip link show
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 08:00:27:2d:82:3a brd ff:ff:ff:ff:ff:ff
3: eth1: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 08:00:27:60:f3:40 brd ff:ff:ff:ff:ff:ff
4: eth2: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 08:00:27:6c:79:35 brd ff:ff:ff:ff:ff:ff
16: veth0: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN qlen 1000
    link/ether 7a:09:4d:bf:05:dd brd ff:ff:ff:ff:ff:ff
17: veth1: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN qlen 1000
    link/ether 9e:64:a0:26:2c:ab brd ff:ff:ff:ff:ff:ff
19: veth3: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN qlen 1000
    link/ether 66:c3:ff:12:43:64 brd ff:ff:ff:ff:ff:ff
```

`testns`側で現れる。

```sh
[root@localhost vagrant]# ip link show
5: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
6: veth0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc pfifo_fast state DOWN qlen 1000
    link/ether 96:3c:b6:b8:7a:26 brd ff:ff:ff:ff:ff:ff
7: veth1: <BROADCAST,MULTICAST> mtu 1500 qdisc pfifo_fast state DOWN qlen 1000
    link/ether 3a:d6:c7:a2:16:90 brd ff:ff:ff:ff:ff:ff
[root@localhost vagrant]# ip link show
5: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
6: veth0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc pfifo_fast state DOWN qlen 1000
    link/ether 96:3c:b6:b8:7a:26 brd ff:ff:ff:ff:ff:ff
7: veth1: <BROADCAST,MULTICAST> mtu 1500 qdisc pfifo_fast state DOWN qlen 1000
    link/ether 3a:d6:c7:a2:16:90 brd ff:ff:ff:ff:ff:ff
18: veth2: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN qlen 1000
    link/ether 52:61:c1:6d:f8:3e brd ff:ff:ff:ff:ff:ff
```

vethはクロスケーブルでつながったNICとして認識するのがよさそう。

`veth2`と`veth3`で同じセグメントのIPを振る必要はあるのだろうか。

`netns`側の設定

```sh
[root@localhost vagrant]# sudo ip addr add 192.168.6.1/24 dev veth2
[root@localhost vagrant]# sudo ip link set veth2 up
```

ホスト側の設定

```sh
[vagrant@localhost router]$ sudo ip addr add 192.168.5.254/24 dev veth3
[vagrant@localhost router]$ sudo ip link set veth3 up
```


```sh
[root@localhost vagrant]# ping 192.168.5.254 -I veth2
PING 192.168.5.254 (192.168.5.254) from 192.168.6.1 veth2: 56(84) bytes of data.
From 192.168.6.1 icmp_seq=1 Destination Host Unreachable
From 192.168.6.1 icmp_seq=2 Destination Host Unreachable
From 192.168.6.1 icmp_seq=3 Destination Host Unreachable
From 192.168.6.1 icmp_seq=4 Destination Host Unreachable
From 192.168.6.1 icmp_seq=5 Destination Host Unreachable
From 192.168.6.1 icmp_seq=6 Destination Host Unreachable
^C
--- 192.168.5.254 ping statistics ---
8 packets transmitted, 0 received, +6 errors, 100% packet loss, time 7617ms
pipe 4
[root@localhost vagrant]# sudo ip addr change 192.168.5.1/24 dev veth2
[root@localhost vagrant]# ping 192.168.5.254 -I veth2
PING 192.168.5.254 (192.168.5.254) from 192.168.5.1 veth2: 56(84) bytes of data.
64 bytes from 192.168.5.254: icmp_seq=1 ttl=64 time=0.021 ms
64 bytes from 192.168.5.254: icmp_seq=2 ttl=64 time=0.028 ms
64 bytes from 192.168.5.254: icmp_seq=3 ttl=64 time=0.028 ms
64 bytes from 192.168.5.254: icmp_seq=4 ttl=64 time=0.027 ms
^C
--- 192.168.5.254 ping statistics ---
4 packets transmitted, 4 received, 0% packet loss, time 3258ms
rtt min/avg/max/mdev = 0.021/0.026/0.028/0.003 ms
```