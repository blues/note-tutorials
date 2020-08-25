#include <bme680.h>
#include <Adafruit_BME680.h>
#include <bme680_defs.h>

#define serialDebug Serial
#define serialNotecard Serial1

#define NOTE_PRODUCT_UID "<com.your-company.your-product>"

Adafruit_BME680 bmeSensor;

void setup() {
  // Initialize Serial Debug
  serialDebug.begin(115200);
  while (!serialDebug) {
    ; // wait for serial port to connect. Needed for native USB
  }
  serialDebug.println("Starting...");

  // Initialize Notecard Serial
  serialNotecard.begin(9600);
  serialNotecard.println("\n");

  // Configure the Notecard
  serialNotecard.println("{\"req\":\"hub.set\",\"product\":\"" NOTE_PRODUCT_UID "\",\"mode\":\"continuous\"}");

  // Await and log Notecard configuration response
  delay(50); // wait for the Notecard to respond
  while (serialNotecard.available() > 0) {
    char incomingByte = serialNotecard.read();
    if (incomingByte != '\r' && incomingByte != '\n') {
      serialDebug.print(incomingByte);
    }
  }
  serialDebug.println();

  // Initialize and configure the BME680
  if (!bmeSensor.begin()) {
    serialDebug.println("Could not find a valid BME680 sensor...");
    while(false); // halt sketch
  } else {
    serialDebug.println("BME680 Connected...");
  }
  bmeSensor.setTemperatureOversampling(BME680_OS_8X);
  bmeSensor.setHumidityOversampling(BME680_OS_2X);
}

void loop() {
  // Sample the BME680 sensor
  if (!bmeSensor.performReading()) {
    serialDebug.println("Failed to obtain a reading...");
    return;
  }
  
  // Print readings to the console
  serialDebug.print("Temperature = ");
  serialDebug.print(bmeSensor.temperature);
  serialDebug.println(" *C");
  serialDebug.print("Humidity = ");
  serialDebug.print(bmeSensor.humidity);
  serialDebug.println(" %");

  // Create the JSON request string
  char json_request[150];
  snprintf(json_request, sizeof(json_request),
    "{"
      "\"req\":\"note.add\","
      "\"file\":\"sensors.qo\","
      "\"start\":true,"
      "\"body\":{"
        "\"temp\":%d.%02d,"
        "\"humidity\":%d.%02d"
      "}"
    "}",
    (int)bmeSensor.temperature, abs(((int)(bmeSensor.temperature*100.0)%100)),
    (int)bmeSensor.humidity, (int)(bmeSensor.humidity*100.0)%100
  );

  // Send request to Notecard
  serialNotecard.println(json_request);
  delay(50); // wait for the Notecard to respond
  while (serialNotecard.available() > 0) {
    char incomingByte = serialNotecard.read();
    if (incomingByte != '\r' && incomingByte != '\n') {
      serialDebug.print(incomingByte);
    }
  }
  serialDebug.println();

  delay(15000);
}
