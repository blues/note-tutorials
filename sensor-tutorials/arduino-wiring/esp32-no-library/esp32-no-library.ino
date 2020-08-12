
#include <bme680.h>
#include <Adafruit_BME680.h>
#include <bme680_defs.h>

#define serialDebug Serial
#define serialNotecard Serial1

#define productUID "com.your-company.your-project"

Adafruit_BME680 bmeSensor;

void setup() {
  delay(2500);
  serialDebug.begin(115200);
    
  serialNotecard.begin(9600);
  serialNotecard.println("\n");

  serialDebug.println("Connected to Notecard!");

  if (!bmeSensor.begin()) {
    serialDebug.println("Could not find a valid BME680 sensor...");
  } else {
    serialDebug.println("BME680 Connected...");
  }

  
  bmeSensor.setTemperatureOversampling(BME680_OS_8X);
  bmeSensor.setHumidityOversampling(BME680_OS_2X);
  
  serialNotecard.println("{\"req\":\"hub.set\",\"product\":\"" productUID "\",\"mode\":\"continuous\"}");

  while (serialNotecard.available() > 0) {
    char incomingByte = serialNotecard.read();

    serialDebug.print(incomingByte);
  }
  serialDebug.println();
}

void loop() {
  if (! bmeSensor.performReading()) {
    serialDebug.println("Failed to obtain a reading...");
    return;
  }

  serialDebug.print("Temperature = ");
  serialDebug.print(bmeSensor.temperature);
  serialDebug.println(" *C");

  serialDebug.print("Humidity = ");
  serialDebug.print(bmeSensor.humidity);
  serialDebug.println(" %");

  char message[150];
  snprintf(message, sizeof(message),
     "{"
     "\"req\":\"note.add\""
     ","
     "\"file\":\"sensors.qo\""
     ","
     "\"start\":true"
     ","
     "\"body\":{\"temp\":%d.%02d,\"humidity\":%d.%02d}"
     "}",
     (int)bmeSensor.temperature, abs(((int)(bmeSensor.temperature*100.0)%100)),
     (int)bmeSensor.humidity, (int)(bmeSensor.humidity*100.0)%100);
  serialNotecard.println(message);

  while (serialNotecard.available() > 0) {
    char incomingByte = serialNotecard.read();

    serialDebug.print(incomingByte);
  }
  serialDebug.println();

  delay(15000);
}
