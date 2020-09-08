# Export network neighbors learned via LLDP as a YANG model

Use information obtained from
[`systemd-networkd`](https://www.freedesktop.org/software/systemd/man/systemd.network.html)'s
LLDP support, and publish that via [sysrepo](https://www.sysrepo.org/) as a YANG model.
Assumes an already-configured `systemd-networkd`, like this:

```ini
[Match]
Name=eth0

[Network]
Bridge=br0
LLDP=true
EmitLLDP=nearest-bridge
```
