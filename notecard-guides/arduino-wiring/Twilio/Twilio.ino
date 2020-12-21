#include <Notecard.h>

#ifdef B0
#undef B0
#endif
#define B0 21

#define LOOP_HZ 20
#define LOOP_DELAY_MS (1000/LOOP_HZ)

#define serialDebug Serial
#define productUID "com.your-company.you:your-project"

Notecard notecard;

volatile bool notecard_sms_alert = false;
size_t last_request_ms = 0;

void IRAM_ATTR sendSmsAlert () {
  notecard_sms_alert = true;
}

void setup() {
  // Initialize Debug Output
  serialDebug.begin(115200);
  while (!serialDebug) {
    ; // wait for serial port to connect. Needed for native USB
  }

  // Initialize Notecard
  notecard.begin();
  notecard.setDebugOutputStream(serialDebug);

  // Configure Notecard
  if (J *req = notecard.newRequest("hub.set")) {
    JAddStringToObject(req, "product", productUID);
    JAddStringToObject(req, "mode", "continuous");
    JAddBoolToObject(req, "sync", true);
    if (!notecard.sendRequest(req)) {
      notecard.logDebug("FATAL: Failed to configure Notecard!\n");
      while(1);
    }
  }

  // Attach Button Interrupt
  pinMode(B0, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(B0), sendSmsAlert, RISING);
}

void loop() {
  // Process SMS Alert Request

  size_t loop_ms = millis();
  if (notecard_sms_alert && ((loop_ms-last_request_ms) > 1000) ) {
    notecard_sms_alert = false;
    last_request_ms = loop_ms;

    // Send SMS Alert
    if (J *req = NoteNewRequest("note.add")) {
      JAddStringToObject(req, "file", "twilio.qo");
      JAddBoolToObject(req, "sync", true);
      if (J *body = JAddObjectToObject(req, "body")) {
        JAddStringToObject(body, "body", "Hello from Notecard!");
        JAddStringToObject(body, "from", "+12515773231");
        JAddStringToObject(body, "to", "+13143591190");
        if (notecard.sendRequest(req)) {
          // SMS alert sent
        } else {
          notecard.logDebug("ERROR: Failed to send SMS alert!\n");
        }
      }
    }
  }

  // Loop at `LOOP_HZ` Hz
  delay(LOOP_DELAY_MS);
}
