#include <Notecard.h>
#include <seeed_bme680.h>

#define serialDebug Serial
#define serialNotecard Serial1

#define productUID "com.your-company.your-project"

#define IIC_ADDR  uint8_t(0x76)
Seeed_BME680 bmeSensor(IIC_ADDR);

void setup() {
  delay(2500);
  serialDebug.begin(115200);
  NoteSetDebugOutputStream(serialDebug);

  NoteInitSerial(serialNotecard, 9600);

  J *req = NoteNewRequest("hub.set");
  JAddStringToObject(req, "product", productUID);
  JAddStringToObject(req, "mode", "continuous");
  NoteRequest(req);

  if (!bmeSensor.init()) {
    serialDebug.println("Could not find a valid BME680 sensor...");
  } else {
    serialDebug.println("BME680 Connected...");
  }
}

void loop() {
  if (bmeSensor.read_sensor_data()) {
    serialDebug.println("Failed to obtain a reading...");
    delay(60000);
    return;
  }

  serialDebug.print("Temperature = ");
  serialDebug.print(bmeSensor.sensor_result_value.temperature);
  serialDebug.println(" *C");

  serialDebug.print("Humidity = ");
  serialDebug.print(bmeSensor.sensor_result_value.humidity);
  serialDebug.println(" %");

  J *req = NoteNewRequest("note.add");
  if (req != NULL) {
    JAddStringToObject(req, "file", "sensors.qo");
    JAddBoolToObject(req, "start", true);
    
    J *body = JCreateObject();
    if (body != NULL) {
      JAddNumberToObject(body, "temp", bmeSensor.sensor_result_value.temperature);
      JAddNumberToObject(body, "humidity", bmeSensor.sensor_result_value.humidity);
      JAddItemToObject(req, "body", body);
    }
    
    NoteRequest(req);
  }

  delay(60000);
}
