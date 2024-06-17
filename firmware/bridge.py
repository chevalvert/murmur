import atexit
import board
import socket
import time
import signal
import sys
from rpi_ws281x import PixelStrip, Color, ws

UDP = ("0.0.0.0", 8888)
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(UDP)
sock.settimeout(2)

# SEE https://github.com/rpi-ws281x/rpi-ws281x-python/blob/master/examples/SK6812_strandtest.py
LED_COUNT = 300
LED_PIN = 18          # GPIO pin connected to the pixels (must support PWM!).
LED_FREQ_HZ = 800000  # LED signal frequency in hertz (usually 800khz)
LED_DMA = 10          # DMA channel to use for generating signal (try 10)
LED_BRIGHTNESS = 255  # Set to 0 for darkest and 255 for brightest
LED_INVERT = False    # True to invert the signal (when using NPN transistor level shift)
LED_CHANNEL = 0
LED_STRIP = ws.SK6812_STRIP_RGBW
strip = PixelStrip(
  LED_COUNT,
  LED_PIN,
  LED_FREQ_HZ,
  LED_DMA,
  LED_INVERT,
  LED_BRIGHTNESS,
  LED_CHANNEL,
  ws.SK6812_STRIP_RGBW
)
strip.begin()

print("listenning on: %s:%s" % UDP)

def clear():
  for i in range(strip.numPixels()):
    strip.setPixelColor(i, Color(0, 0, 0))
  strip.show()

def clean_exit(_signo, _stack_frame):
  clear()
  sys.exit(0)

# Turn off the strip when quitting
atexit.register(clear)
signal.signal(signal.SIGTERM, clean_exit)

# Blink
for j in range(0, 3):
  for i in range(LED_COUNT):
    strip.setPixelColor(i, Color(0, 0, 30, 0))
  strip.show()
  time.sleep(0.5)
  clear()
  time.sleep(0.5)

while 1:
  try:
    start = time.time()
    data, addr = sock.recvfrom(LED_COUNT * 4)
    for i in range(LED_COUNT):
      d = i*4
      strip.setPixelColor(i, Color(data[d+1], data[d+0], data[d+2], data[d+3]))

    strip.show()
    print('%sms' % int((time.time() - start) * 1000))

  except Exception as e:
    print(e)
    clear()
    time.sleep(1)
