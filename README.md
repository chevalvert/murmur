# Murmur [<img src="https://github.com/chevalvert.png?size=100" align="right">](http://chevalvert.fr/)

<br>

## Installation

### Requirements

- Raspberry Pi 4 B
- Raspberry Pi OS Lite (64-bit)
- Mac mini with ethernet port

### Wiring

See [`firmware/README.md`](firmware/README.md).

### Static IP setup

<b>IMPORTANT:</b> macos should have _Internet Sharing_ enabled.

Then in macos network ethernet settings, set `IPv4 Configured` to `Using DHCP with Manual Adress`, with the IP address to `<ROUTER_IP>`.

```
$ sudo apt-get install dhcpcd5
$ sudo service dhcpcd start
$ sudo systemctl enable dhcpcd
$ sudo nano /etc/dhcpcd.conf
```

###### `/etc/dhcpcd.conf`
```
interface <INTERFACE>
static_routers=<ROUTER IP>
static domain_name_servers=<DNS IP>
static ip_address=<STATIC IP ADDRESS YOU WANT>/24
```

### Software

See releases for the app binary, or clone this repo and compile the package (see Development below).

## Development

```console
$ git clone https://github.com/chevalvert/murmur
$ cd murmur
$ yarn install
$ yarn start
$ yarn version
```

```console
$ yarn pkg
```

## Credits

- [At Aero](https://arillatype.studio/font/at-aero) by Arillatype.Studio®
- JSX and state utils heavily based on [**pqml**](https://github.com/pqml)’s work.

## License
[MIT.](https://tldrlegal.com/license/mit-license)
