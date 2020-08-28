import board
import busio
import time
import adafruit_bme680
import json
import notecard
from periphery import Serial

productUID = "<com.your-company.your-product>"

serial = Serial('/dev/ttyS0', 9600)
serial.write(b'\n')

card = notecard.OpenSerial(serial)

req = {"req": "hub.set"}
req["product"] = productUID
req["mode"] = "continuous"

print(json.dumps(req))

rsp = card.Transaction(req)
print(rsp)

i2c = busio.I2C(board.SCL, board.SDA)
sensor = adafruit_bme680.Adafruit_BME680_I2C(i2c)

while True:
  temp = sensor.temperature
  humidity = sensor.humidity
  print('Temperature: {} degrees C'.format(temp))
  print('Humidity: {}%'.format(humidity))

  req = {"req": "note.add"}
  req["file"] = "sensors.qo"
  req["start"] = True
  req["body"] = { "temp": temp, "humidity": humidity }

  rsp = card.Transaction(req)
  print(rsp)

  time.sleep(15)
