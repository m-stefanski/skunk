from machine import Pin, I2C
from time import sleep
import BME280

# ESP8266 - Pin assignment
i2c = I2C(scl=Pin(5), sda=Pin(4), freq=10000)
bme = BME280.BME280(i2c=i2c)

print("Measures started")

while True:
  print('Temperature: ', bme.temperature)
  print('Humidity: ', bme.humidity)
  print('Pressure: ', bme.pressure)
  sleep(5)
