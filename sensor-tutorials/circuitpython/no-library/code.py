import board
import busio
import time
import json
import adafruit_bme680

productUID  = "com.your-company.your-project"

# Configure the serial connection to the Notecard
serial = busio.UART(board.TX, board.RX, baudrate=9600)

# Flush the Notecard buffer and set the ProductUID
serial.write(b'\n')

req = {"req": "hub.set"}
req["product"] = productUID
req["mode"] = "continuous"
serial.write(bytearray(json.dumps(req)))
serial.write(b'\n')

print(json.dumps(req))

#Read the response to ensure a successful connection
data = serial.read(32)

if data is not None:
  data_string = ''.join([chr(b) for b in data])

  print(data_string, end="")
  print("Notecard configured...")
else:
  print('Notecard not connected...')

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
  serial.write(bytearray(json.dumps(req)))
  serial.write(b'\n')

  data = serial.read(255)

  if data is not None:
    data_string = ''.join([chr(b) for b in data])
    print(data_string, end="")

  time.sleep(15)