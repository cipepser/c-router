# ルーター自作でわかるパケットの流れの写経

## MACアドレスを表示する

```sh
$ sudo ./ltest <I/F name>
```

## bridgeの使い方

Vagrantfileに以下を記載してNICを2つ用意する。

```
config.vm.network "private_network", ip: "192.168.0.129"
config.vm.network "private_network", ip: "192.168.0.130"
```

tcpdumpでicmpをフィルタする。

```sh
$ sudo tcpdump -i eth1 icmp
```

bridgeを立ち上げておく。

```sh
$ sudo ./bridge
```

これらの状態でホストOS(今回は`192.168.0.1`を持っている)から`ping 192.168.0.129`(130でもいい)を打つと無限ループする。  
※ bridgeは`eth1 -> eth2`と`eth2 -> eth1`でframeを転送するよう実装しているので期待する動作。

```sh
06:45:01.950942 IP 192.168.0.129 > 192.168.0.1: ICMP echo reply, id 47638, seq 0, length 64
06:45:01.952483 IP 192.168.0.1 > 192.168.0.130: ICMP echo request, id 7703, seq 0, length 64
06:45:01.952496 IP 192.168.0.130 > 192.168.0.1: ICMP echo reply, id 7703, seq 0, length 64
06:45:01.953876 IP 192.168.0.1 > 192.168.0.129: ICMP echo request, id 47638, seq 0, length 64
06:45:01.953893 IP 192.168.0.129 > 192.168.0.1: ICMP echo reply, id 47638, seq 0, length 64
06:45:01.954879 IP 192.168.0.1 > 192.168.0.130: ICMP echo request, id 7703, seq 0, length 64
06:45:01.954924 IP 192.168.0.130 > 192.168.0.1: ICMP echo reply, id 7703, seq 0, length 64
06:45:01.956247 IP 192.168.0.1 > 192.168.0.129: ICMP echo request, id 47638, seq 0, length 64
06:45:01.956261 IP 192.168.0.129 > 192.168.0.1: ICMP echo reply, id 47638, seq 0, length 64
06:45:01.957071 IP 192.168.0.1 > 192.168.0.130: ICMP echo request, id 7703, seq 0, length 64
06:45:01.957083 IP 192.168.0.130 > 192.168.0.1: ICMP echo reply, id 7703, seq 0, length 64
06:45:01.958595 IP 192.168.0.1 > 192.168.0.129: ICMP echo request, id 47638, seq 0, length 64
06:45:01.958606 IP 192.168.0.129 > 192.168.0.1: ICMP echo reply, id 47638, seq 0, length 64
06:45:01.959133 IP 192.168.0.1 > 192.168.0.130: ICMP echo request, id 7703, seq 0, length 64
06:45:01.959141 IP 192.168.0.130 > 192.168.0.1: ICMP echo reply, id 7703, seq 0, length 64
06:45:01.960341 IP 192.168.0.1 > 192.168.0.129: ICMP echo request, id 47638, seq 0, length 64
06:45:01.960354 IP 192.168.0.129 > 192.168.0.1: ICMP echo reply, id 47638, seq 0, length 64
06:45:01.961139 IP 192.168.0.1 > 192.168.0.130: ICMP echo request, id 7703, seq 0, length 64
06:45:01.961151 IP 192.168.0.130 > 192.168.0.1: ICMP echo reply, id 7703, seq 0, length 64
06:45:01.962368 IP 192.168.0.1 > 192.168.0.129: ICMP echo request, id 47638, seq 0, length 64
06:45:01.962381 IP 192.168.0.129 > 192.168.0.1: ICMP echo reply, id 47638, seq 0, length 64
06:45:01.964442 IP 192.168.0.1 > 192.168.0.130: ICMP echo request, id 7703, seq 0, length 64
06:45:01.964463 IP 192.168.0.130 > 192.168.0.1: ICMP echo reply, id 7703, seq 0, length 64
```

## Apendix

### vagrantでhost OSとguest OSの共有フォルダがmountできない

`vagrant vbguest`だけでは解決しなかったが、以下で解決した。

[LPIC対策にVirtual Box上のCentOS7にGuest Additonsを入れる際にハマった](https://qiita.com/YusakuS16/items/94c430a7ef735ed08a16)