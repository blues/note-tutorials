#include <Notecard.h>
#include <Wire.h>
#include <seeed_bme680.h>

#define serialDebug Serial
#define serialNotecard Serial1

#define productUID "com.veritas.delivery-fleet.tracker"

#define IIC_ADDR  uint8_t(0x76)
Seeed_BME680 bmeSensor(IIC_ADDR);

Notecard notecard;

bool notecardProductSet = false;

long previousMillis = 0;
long interval = 60000 * 10;

void resetNotecard();
bool productUIDMatch();
void setProductUID();
void configureHub();
void configureGPS();
void configureTracking();
void readFromSensors();

void setup()
{
  serialDebug.begin(115200);
  notecard.setDebugOutputStream(serialDebug);

  notecard.begin();

  // Query ProductUID. If the ProductUID does not match, perform a card.restore
  if (productUIDMatch()) {
    serialDebug.println("ProductUID matches, skipping reset...");
    notecardProductSet = true;
  } else {
    serialDebug.println("ProductUID does not match, performing a reset...");

    resetNotecard();

    // Wait 10 seconds for Notecard to come back online
    delay(10000);

    setProductUID();

    //Make sure the ProductUID is now set before continuing
    if (productUIDMatch()) {
      serialDebug.println("ProductUID matches, continuing configuration...");
      notecardProductSet = true;
    } else {
      serialDebug.println("ProductUID does not match, please perform a manual configuration.");
    }
  }

  if (notecardProductSet) {
    // Send Notecard Config Requests
    configureHub();
    configureGPS();
    configureTracking();

    if (!bmeSensor.init()) {
      serialDebug.println("Could not find a valid BME680 sensor...");
    } else {
      serialDebug.println("BME680 Connected...");
    }

    // Do an initial read
    readFromSensors();
  }
}

void loop()
{
	unsigned long currentMillis = millis();

  if ((currentMillis - previousMillis > interval) && notecardProductSet) {
    previousMillis = currentMillis;

    readFromSensors();
  }
}

void readFromSensors()
{
  if (bmeSensor.read_sensor_data()) {
    serialDebug.println("Failed to obtain a reading...");
  } else {
    serialDebug.print("Temperature = ");
    serialDebug.print(bmeSensor.sensor_result_value.temperature);
    serialDebug.println(" *C");

    serialDebug.print("Humidity = ");
    serialDebug.print(bmeSensor.sensor_result_value.humidity);
    serialDebug.println(" %");

    serialDebug.print("Pressure = ");
    serialDebug.print(bmeSensor.sensor_result_value.pressure / 1000.0);
    serialDebug.println(" KPa");

    serialDebug.print("Gas = ");
    serialDebug.print(bmeSensor.sensor_result_value.gas / 1000.0);
    serialDebug.println(" Kohms");

    J *req = notecard.newRequest("note.add");
    if (req != NULL) {
      JAddStringToObject(req, "file", "sensors.qo");

      J *body = JCreateObject();
      if (body != NULL) {
        JAddNumberToObject(body, "temp", bmeSensor.sensor_result_value.temperature);
        JAddNumberToObject(body, "humidity", bmeSensor.sensor_result_value.humidity);
        JAddNumberToObject(body, "pressure", bmeSensor.sensor_result_value.pressure / 1000.0);
        JAddNumberToObject(body, "gas", bmeSensor.sensor_result_value.gas / 1000.0);
        JAddItemToObject(req, "body", body);
      }

      notecard.sendRequest(req);
    }
  }
}

void resetNotecard()
{
  J *req = notecard.newRequest("card.restore");
  JAddBoolToObject(req, "delete", true);
  notecard.sendRequest(req);
}

bool productUIDMatch()
{
  bool isMatch = false;
  J *req = notecard.newRequest("hub.get");
  J *rsp = notecard.requestAndResponse(req);

  if (rsp != NULL) {

    if (notecard.responseError(rsp)) {
      notecard.deleteResponse(rsp);
      return isMatch;
    }

    // Get the note's body
    char *setProductUID = JGetString(rsp, "product");
    if (setProductUID != NULL) {
      NoteDebugf("Product UID: %s\n\n", setProductUID);

      if (strcmp(productUID, setProductUID) == 0) {
        isMatch = true;
      }
    }

  }
  notecard.deleteResponse(rsp);

  return isMatch;
}

void setProductUID()
{
  J *req = notecard.newRequest("hub.set");
  JAddStringToObject(req, "product", productUID);
  notecard.sendRequest(req);
}

void configureHub()
{
  J *req = notecard.newRequest("hub.set");
  JAddStringToObject(req, "mode", "periodic");
  JAddNumberToObject(req, "outbound", 60);
  JAddNumberToObject(req, "inbound", 120);
  notecard.sendRequest(req);
}

void configureGPS()
{
  J *req = notecard.newRequest("card.location.mode");
  JAddStringToObject(req, "mode", "periodic");
  JAddNumberToObject(req, "seconds", 3600);
  notecard.sendRequest(req);
}

void configureTracking()
{
  J *req = notecard.newRequest("card.location.track");
  JAddBoolToObject(req, "start", true);
  JAddBoolToObject(req, "heartbeat", true);
  JAddNumberToObject(req, "hours", 4);
  notecard.sendRequest(req);
}
