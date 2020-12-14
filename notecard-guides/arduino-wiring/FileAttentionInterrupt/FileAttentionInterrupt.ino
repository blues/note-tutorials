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
#define productUID "com.your-company.you:your-project"

Notecard notecard;

volatile bool notecard_request_to_arm = false;
volatile int notehub_note_queued = 0;

void IRAM_ATTR armInterrupt() {
  // Take no action when already armed
  if (digitalRead(D5)) {
    notecard_request_to_arm = true;
  }
}

void IRAM_ATTR attention() {
  ++notehub_note_queued;
  
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
      JAddStringToObject(req, "mode", "arm,files");
      if (J *files = JAddArrayToObject(req, "files")) {
        JAddItemToArray(files, JCreateString("attention.qi"));
        if (notecard.sendRequest(req)) {
          // Visualize the attention pin state
          digitalWrite(LED_BUILTIN, digitalRead(D5));
        } else {
          notecard.logDebug("ERROR: Failed to arm ATTN interrupt!\n");
        }
      }
    }
  }

  // Process Queued Notes
  if (notehub_note_queued) {
    --notehub_note_queued;

    // Pop Note from `attention.qi` Queue
    if (J *req = NoteNewRequest("note.get")) {
      JAddStringToObject(req, "file", "attention.qi");
      JAddBoolToObject(req, "delete", true);
      J *resp = notecard.requestAndResponse(req);
      if (JHasObjectItem(resp,"err")) {
        notecard.logDebug("ERROR: Failed to dequeue Note!\n");
      } else {
        // Process Dequeued Note
        if (JHasObjectItem(resp,"time")) {
          double time_value;
          J* time_obj = JGetObjectItem(resp, "time");

          if (!JIsNumber(time_obj)) {
            time_value = (double)NAN;
          } else {
            time_value = time_obj->valuenumber;
          }
      
          notecard.logDebugf("Note sent at %.0lf\n", time_value);
        } else {
          notecard.logDebug("ERROR: Unrecognized Note format!\n");
        }
      }
    }
  }

  // Loop at `LOOP_HZ` Hz
  delay(LOOP_DELAY_MS);
}
