from machine import Pin, I2C
import network
import usocket as socket
from time import sleep_ms

import BME280
import web_templates

WIFI_SSID='skunks'
WIFI_PASSWORD='1234567890'

# Cleaning up the screen
print("\x1B\x5B2J", end="")
print("\x1B\x5BH", end="")
print("[skunk]: Starting app")

print("[bme280]: Setting up I2C connection")
i2c = I2C(scl=Pin(5), sda=Pin(4), freq=10000)
bme = BME280.BME280(i2c=i2c)
print("[bme280]: I2C connection ready")

print("[wifi]: Setting up AP")

ap = network.WLAN(network.AP_IF)
ap.active(True)
ap.config(essid=WIFI_SSID, password=WIFI_PASSWORD)

while ap.active() == False:
  pass

ip = ap.ifconfig()[0]
print("[wifi]: AP created - SSID: {}, pass: {}, gateway: {}".format(WIFI_SSID, WIFI_PASSWORD, ip))

print("[skunk]: Startup finished")

def web_page():
  return web_templates.parse_template('index.html', temp=bme.temperature, humid=bme.humidity, press=bme.pressure)

webs = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
webs.bind(('', 80))
webs.listen(5)

while True:
    conn, addr = webs.accept()
    print('[web]: new connection from {}'.format(str(addr)))
    request = conn.recv(1024)
    print('[web]: request: {}'.format(request.decode('utf-8').split('\r\n')[0]))
    response = web_page()
    conn.send(response)
    print('[web]: response sent')
    conn.close()
    print('[web]: connection closed')
    sleep_ms(500)
