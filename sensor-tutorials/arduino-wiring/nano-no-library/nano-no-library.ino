#include <seeed_bme680.h>

#define IIC_ADDR  uint8_t(0x76)
Seeed_BME680 bmeSensor(IIC_ADDR);

#define serialDebug Serial
#define serialNotecard Serial1

#define productUID "com.your-company.your-project"

void setup() {
  serialDebug.begin(115200);
 
  serialDebug.println("Starting...");
  
  if (!bmeSensor.init()) {
      serialDebug.println("Could not find a valid BME680 sensor...");
  } else {
      serialDebug.println("BME680 Connected...");
  }

  serialNotecard.begin(9600);
  serialNotecard.println("\n");

  serialNotecard.println("{\"req\":\"hub.set\",\"product\":\"" productUID "\",\"mode\":\"continuous\"}");

  while (serialNotecard.available() > 0) {
    char incomingByte = serialNotecard.read();

    serialDebug.print(incomingByte);
  }
  serialDebug.println();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (bmeSensor.read_sensor_data()) {
      serialDebug.println("Failed to obtain a reading...");
      delay(15000);
      
      return;
  }

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
