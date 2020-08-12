import json
import notecard
from periphery import Serial
import board
import busio
import time
import adafruit_bme680

productUID  = "com.your-company.your-project"

port = Serial("/dev/serial0", 9600)
card = notecard.OpenSerial(serial)

req = {"req": "hub.set"}
req["product"] = productUID
req["mode"] = "continuous"

print(json.dumps(req))

rsp = card.Transaction(req)
print(rsp)

# Configure the Adafruit BME680
i2c = busio.I2C(board.SCL, board.SDA)
sensor = adafruit_bme680.Adafruit_BME680_I2C(i2c)

while True:
    temp = sensor.temperature
    humidity = sensor.humidity
    print('\nTemperature: {} degrees C'.format(temp))
    print('Humidity: {}%'.format(humidity))

    time.sleep(15)
  
    req = {"req": "note.add"}
    req["file"] = "sensors.qo"
    req["start"] = True
    req["body"] = { "temp": temp, "humidity": humidity}
    req = card.Transaction(req)
  
    print(rsp)

    time.sleep(15)