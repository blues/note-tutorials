import board
import busio
import time
import json
import adafruit_bme680
import notecard

productUID  = "com.your-company.your-project"

# Configure the serial connection to the Notecard
serial = busio.UART(board.TX, board.RX, baudrate=9600)
card = notecard.OpenSerial(serial)

req = {"req": "hub.set"}
req["product"] = productUID
req["mode"] = "continuous"
rsp = card.Transaction(req)

print(rsp)

# Configure the Adafruit BME680
i2c = busio.I2C(board.SCL, board.SDA)
bmeSensor = adafruit_bme680.Adafruit_BME680_I2C(i2c)

while True:
  temp = bmeSensor.temperature
  humidity = bmeSensor.humidity
  print("\nTemperature: %0.1f C" % temp)
  print("Humidity: %0.1f %%" % humidity)
  
  req = {"req": "note.add"}
  req["file"] = "sensors.qo"
  req["start"] = True
  req["body"] = { "temp": temp, "humidity": humidity}
  req = card.Transaction(req)
  
  print(rsp)

  time.sleep(15)