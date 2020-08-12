import json
from periphery import Serial

import board
import busio
import time
import adafruit_bme680

productUID  = "com.your-company.your-project"

serial = Serial('/dev/ttyS0', 9600)
serial.write(b'\n')

req = {"req": "hub.set"}
req["product"] = productUID
req["mode"] = "continuous"
serial.write(bytearray(json.dumps(req), 'utf8'))
serial.write(b'\n')

print(json.dumps(req))

i2c = busio.I2C(board.SCL, board.SDA)
sensor = adafruit_bme680.Adafruit_BME680_I2C(i2c)

data = serial.read(32, 0.5)
if data is not None:
    data_string = ''.join([chr(b) for b in data])
    print(data_string, end="")
    print("Notecard configured...")
else:
    print('Notecard not connected...')

while True:
    temp = sensor.temperature
    humidity = sensor.humidity
    print('Temperature: {} degrees C'.format(temp))
    print('Humidity: {}%'.format(humidity))

    req = {"req": "note.add"}
    req["file"] = "sensors.qo"
    req["start"] = True
    req["body"] = { "temp": temp, "humidity": humidity }

    serial.write(bytearray(json.dumps(req), 'utf8'))
    serial.write(b'\n')

    data = serial.read(255, 0.5)
    if data is not None:
        data_string = ''.join([chr(b) for b in data])
        print(data_string, end="")

    time.sleep(15)