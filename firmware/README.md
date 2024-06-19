# Murmur firmware

## Requirements

- Raspberry Pi 4 B
- Raspberry Pi OS Lite (64-bit)

## Setup

```shell
ssh murmur@murmur.local
```

```shell
$ sudo apt-get update
$ sudo apt-get -y upgrade
$ sudo apt-get install python3-pip
$ sudo apt install --upgrade python3-setuptools
$ sudo apt install python3-venv
$ python3 -m venv env --system-site-packages
$ source env/bin/activate
$ cd ~
$ pip3 install --upgrade adafruit-python-shell
$ wget https://raw.githubusercontent.com/adafruit/Raspberry-Pi-Installer-Scripts/master/raspi-blinka.py
$ sudo -E env PATH=$PATH python3 raspi-blinka.py
```

```shell
$ ls /dev/i2c* /dev/spi*
> /dev/i2c-1 /dev/spidev0.0 /dev/spidev0.1
```

###### `blinktatest.py`
```python
import board
import digitalio
import busio

print("Hello, blinka!")

# Try to create a Digital input
pin = digitalio.DigitalInOut(board.D4)
print("Digital IO ok!")

# Try to create an I2C device
i2c = busio.I2C(board.SCL, board.SDA)
print("I2C ok!")

# Try to create an SPI device
spi = busio.SPI(board.SCLK, board.MOSI, board.MISO)
print("SPI ok!")

print("done!")
```

```shell
$ source env/bin/activate
$ python3 blinkatest.py
> Hello, blinka!
> Digital IO ok!
> I2C ok!
> SPI ok!
> done!
```

### Neopixel setup

```shell
$ source env/bin/activate
$ pip3 install rpi_ws281x adafruit-circuitpython-neopixel
$ python3 -m pip install --force-reinstall adafruit-blinka
```

```shell
$ sudo /home/murmur/env/bin/python3 bridge.py
```

### Boot setup

```shell 
$ sudo nano /etc/rc.local
```

###### `/etc/rc.local`
```bash
# Run murmur firmware
sudo /home/murmur/env/bin/python3 /home/murmur/bridge.py &
exit 0
```

## Killing firmware

```shell
$ ssh murmur@murmur.local
$ ps -ef | grep python
> root        <PID>      1  0 17:08 ?        00:00:00 sudo /home/murmur/env/bin/python3 /home/murmur/bridge.py
> root         808     803  0 17:08 ?        00:00:00 /home/murmur/env/bin/python3 /home/murmur/bridge.py
> murmur       869     843  0 17:20 pts/0    00:00:00 grep --color=auto python
$ kill <PID>
```

## Ressources

- [Installing Blinka on Raspberry Pi](https://learn.adafruit.com/circuitpython-on-raspberrypi-linux/installing-circuitpython-on-raspberry-pi)
- [NeoPixels on Raspberry Pi](https://learn.adafruit.com/neopixels-on-raspberry-pi/python-usage)
- [Basic Venv Usage](https://learn.adafruit.com/python-virtual-environment-usage-on-raspberry-pi/basic-venv-usage)
