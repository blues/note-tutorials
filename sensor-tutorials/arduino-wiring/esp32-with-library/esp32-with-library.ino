#include <Notecard.h>

#include <bme680.h>
#include <Adafruit_BME680.h>
#include <bme680_defs.h>

#define serialDebug Serial
#define serialNotecard Serial1

#define productUID "com.blues.brandon.arduino-temp"

Adafruit_BME680 bmeSensor;

void setup() {
  delay(2500);
  serialDebug.begin(115200);
  NoteSetDebugOutputStream(serialDebug);

  NoteInitSerial(serialNotecard, 9600);

  J *req = NoteNewRequest("service.set");
  JAddStringToObject(req, "product", productUID);
  JAddStringToObject(req, "mode", "continuous");
  NoteRequest(req);

  if (!bmeSensor.begin()) {
    serialDebug.println("Could not find a valid BME680 sensor...");
  } else {
    serialDebug.println("BME680 Connected...");
  }

  bmeSensor.setTemperatureOversampling(BME680_OS_8X);
  bmeSensor.setHumidityOversampling(BME680_OS_2X);
}

void loop() {
  if (! bmeSensor.performReading()) {
    serialDebug.println("Failed to obtain a reading...");
    delay(15000);
    return;
  }

  serialDebug.print("Temperature = ");
  serialDebug.print(bmeSensor.temperature);
  serialDebug.println(" *C");

  serialDebug.print("Humidity = ");
  serialDebug.print(bmeSensor.humidity);
  serialDebug.println(" %");

  J *req = NoteNewRequest("note.add");
  if (req != NULL) {
    JAddStringToObject(req, "file", "sensors.qo");
    JAddBoolToObject(req, "start", true);
    
    J *body = JCreateObject();
    if (body != NULL) {
      JAddNumberToObject(body, "temp", bmeSensor.temperature);
      JAddNumberToObject(body, "humidity", bmeSensor.humidity);
       JAddItemToObject(req, "body", body);
    }
    
    NoteRequest(req);
  }

  delay(15000);
}
