#!/usr/bin/bash

# 以下パラメータでルータをコンパイル
# PARAM Param = {"RT_veth0", "RT_veth1", 1, "192.168.0.254"};

sudo ip netns add host
sudo ip netns add RT
sudo ip netns add NextRouter

sudo ip link add host_veth1 type veth peer name RT_veth0
sudo ip link add RT_veth1 type veth peer name NR_veth0
sudo ip link add NR_veth1 type veth peer name Linux_veth0

sudo ip link set host_veth1 netns host
sudo ip link set RT_veth0 netns RT
sudo ip link set RT_veth1 netns RT
sudo ip link set NR_veth0 netns NextRouter
sudo ip link set NR_veth1 netns NextRouter

sudo ip netns exec host ip addr add 192.168.0.1/24 dev host_veth1
sudo ip netns exec RT ip addr add 192.168.0.254/24 dev RT_veth0
sudo ip netns exec RT ip addr add 192.168.1.1/24 dev RT_veth1
sudo ip netns exec NextRouter ip addr add 192.168.1.254/24 dev NR_veth0
sudo ip netns exec NextRouter ip addr add 192.168.2.1/24 dev NR_veth1
sudo ip addr add 192.168.2.254/24 dev Linux_veth0

sudo ip netns exec host ip link set lo up
sudo ip netns exec RT ip link set lo up
sudo ip netns exec NextRouter ip link set lo up

sudo ip netns exec host ip link set host_veth1 up
sudo ip netns exec RT ip link set RT_veth0 up
sudo ip netns exec RT ip link set RT_veth1 up
sudo ip netns exec NextRouter ip link set NR_veth0 up
sudo ip netns exec NextRouter ip link set NR_veth1 up
sudo ip link set Linux_veth0 up

sudo ip netns exec host ip route add default via 192.168.0.254
sudo ip netns exec RT ip route add default via 192.168.1.254
sudo ip netns exec NextRouter ip route add default via 192.168.2.254
sudo ip netns exec NextRouter ip route add 192.168.0.0/24 via 192.168.1.1
sudo ip route add 192.168.0.0/24 via 192.168.2.1
sudo ip route add 192.168.1.0/24 via 192.168.2.1

# sudo ip netns exec host sysctl -w net.ipv4.ip_forward=1
# sudo ip netns exec RT sysctl -w net.ipv4.ip_forward=1 # ./routerを実行するとオフになる
# sudo ip netns exec NextRouter sysctl -w net.ipv4.ip_forward=1
sudo sysctl -w net.ipv4.ip_forward=1 # host, NextRouter, HOST OSではforwadingさせる
# RTについては、`net.ipv4.ip_forward=0`とした`/etc/sysctl.conf`を`/etc/netns/RT/sysctl.conf`としてコピーしておく。
# `/etc/netns/RT`が優先的に読み込まれるので、`DisableIpForward()`を実行すればこちらだけが`0`となるのでファイルさえコピーしておけばよい。
