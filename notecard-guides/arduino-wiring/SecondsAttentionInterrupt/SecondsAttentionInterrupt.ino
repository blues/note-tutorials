#include <Notecard.h>

#ifdef B0
#undef B0
#endif
#define B0 21

#ifdef D5
#undef D5
#endif
#define D5 14

#define LOOP_HZ 20
#define LOOP_DELAY_MS (1000/LOOP_HZ)

#define serialDebug Serial

Notecard notecard;

volatile bool notecard_request_to_arm = false;

void IRAM_ATTR armInterrupt() {
  // Take no action when already armed
  if (digitalRead(D5)) {
    notecard_request_to_arm = true;
  }
}

void IRAM_ATTR attention() {
  // Visualize the attention pin state
  digitalWrite(LED_BUILTIN, digitalRead(D5));
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

  // Attach Notecard Interrupt
  pinMode(D5, INPUT);
  attachInterrupt(digitalPinToInterrupt(D5), attention, RISING);

  // Attach Button Interrupt
  pinMode(B0, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(B0), armInterrupt, RISING);

  // Debug LED (mirrors `ATTN`)
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, digitalRead(D5));
}

void loop() {
  // Process arming request
  if (notecard_request_to_arm) {
    notecard_request_to_arm = false;

    // Arm ATTN Interrupt
    if (J *req = NoteNewRequest("card.attn")) {
      JAddStringToObject(req, "mode", "arm");
      JAddNumberToObject(req, "seconds", 3);
      if (notecard.sendRequest(req)) {
        // Visualize the attention pin state
        digitalWrite(LED_BUILTIN, digitalRead(D5));
      } else {
        notecard.logDebug("ERROR: Failed to arm ATTN interrupt!\n");
      }
    }
  }

  // Loop at `LOOP_HZ` Hz
  delay(LOOP_DELAY_MS);
}
