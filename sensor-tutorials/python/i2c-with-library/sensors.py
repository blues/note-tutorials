import json
import notecard
from periphery import I2C
import time
import bme680

bme_sensor = bme680.BME680(bme680.I2C_ADDR_PRIMARY)

bme_sensor.set_humidity_oversample(bme680.OS_2X)
bme_sensor.set_temperature_oversample(bme680.OS_8X)

bme_sensor.get_sensor_data()

productUID = "com.[your-company].[your-product]"

port = I2C("/dev/i2c-1")
card = notecard.OpenI2C(port, 0, 0)

req = {"req": "service.set"}
req["product"] = productUID
req["mode"] = "continuous"

print(json.dumps(req))

rsp = card.Transaction(req)
print(rsp)

while True:
    bme_sensor.get_sensor_data()

    temp = bme_sensor.data.temperature
    humidity = bme_sensor.data.humidity

    print('Temperature: {} degrees C'.format(temp))
    print('Humidity: {}%'.format(humidity))

    req = {"req": "note.add"}
    req["file"] = "sensors.qo"
    req["start"] = True
    req["body"] = { "temp": temp, "humidity": humidity}
    rsp = card.Transaction(req)

    print(rsp)

    time.sleep(15)
