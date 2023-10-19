#include <Notecard.h>

// Notefile Definitions
#define	DATA_NOTEFILE		     "data.qo"
#define	DATA_PORT            1
#define INBOUND_NOTEFILE     "poll.qi"
#define INBOUND_PORT         2

#define FIELD_TEMPERATURE    "temperature"
#define FIELD_VOLTAGE        "voltage"
#define FIELD_POLL           "poll"

#define INBOUND_NOTEFILE_POLL_SECS 10
#define ENV_POLL_SECS              10
#define DATA_SEND_MINS             5

// Environment variable definitions
#define	VAR_MINUTES	"minutes"

// Cached values of the above environment variable
uint32_t envHeartbeatMins;
uint32_t environmentModifiedTime = 0;

// Time when the notecard was last rebooted
uint32_t notecardBootTime = 0;

HardwareSerial debug(PIN_VCP_RX, PIN_VCP_TX);

// This is the unique Product Identifier for your device
#ifndef PRODUCT_UID
#define PRODUCT_UID "" // "com.my-company.my-name:my-project"
#pragma message "PRODUCT_UID is not defined in this example. Please ensure your Notecard has a product identifier set before running this example or define it in code here. More details at https://dev.blues.io/tools-and-sdks/samples/product-uid"
#endif

Notecard notecard;

// One-time Arduino initialization
void setup()
{
  debug.begin(115200);
  while (!debug);
  debug.println("*** " __DATE__ " " __TIME__ " ***");
  
  Wire.begin();

  notecard.setDebugOutputStream(debug);
  notecard.begin();

  if (J *req = notecard.newRequest("hub.set")) {
    JAddStringToObject(req, "product", PRODUCT_UID);
    JAddStringToObject(req, "mode", "continuous");
    JAddBoolToObject(req, "sync", true);
    JAddNumberToObject(req, "inbound", 1);
    JAddNumberToObject(req, "outbound", 15);
    notecard.sendRequestWithRetry(req, 5);
  }

  // The LoRa Notecard Requires Environment Variables to be templated
  if (J *req = notecard.newRequest("env.template")) {
    if (J *body = JAddObjectToObject(req, "body")) {
      JAddNumberToObject(body, VAR_MINUTES, TUINT16);
      if (!notecard.sendRequest(req)) {
        debug.println("unable to set env template");
      }
    } else {
      JDelete(req);
    }
  }

  /// Notefile Template for Data
  if (J *req = notecard.newRequest("note.template")) {
    JAddStringToObject(req, "file", DATA_NOTEFILE);
    JAddNumberToObject(req, "port", DATA_PORT);
    if (J *body = JAddObjectToObject(req, "body")) {
      JAddNumberToObject(body, FIELD_TEMPERATURE, TFLOAT32);
      JAddNumberToObject(body, FIELD_VOLTAGE, TFLOAT32);
      JAddNumberToObject(body, "_time", TINT32);
      if (!notecard.sendRequest(req)) {
        debug.println("unable to set data template");
      }
    } else {
      JDelete(req);
    }
  }

  // Notefile Template for Inbound
  req = notecard.newRequest("note.template");
  JAddStringToObject(req, "file", INBOUND_NOTEFILE);
  JAddNumberToObject(req, "port", INBOUND_PORT);

  body = JCreateObject();
  JAddBoolToObject(body, FIELD_POLL, true);

  JAddItemToObject(req, "body", body);
  if (!notecard.sendRequest(req)) {
    debug.println("unable to set inbound template");
  }

  // Load the environment vars for the first time
  J *rsp = notecard.requestAndResponse(notecard.newRequest("env.get"));
	if (rsp != NULL) {
		refreshEnvironmentVarCache(rsp);
    notecard.deleteResponse(rsp);
	}
}

// In the Arduino main loop which is called repeatedly, add outbound data every
// 15 seconds
void loop()
{
  static unsigned long nextDataSendMs = 0;
  if (millis() > nextDataSendMs)
  {
    nextDataSendMs = millis() + (DATA_SEND_MINS * 60 * 1000);

    sendDataToNotecard();
  }

  static unsigned long nextEnvPollMs = 0;
  if (millis() > nextEnvPollMs) 
  {
    nextEnvPollMs = millis() + (ENV_POLL_SECS * 1000);

    J *req = notecard.newRequest("env.get");
	  JAddNumberToObject(req, "time", environmentModifiedTime);
	  J *rsp = notecard.requestAndResponse(req);
		if (rsp != NULL) {
			if (!notecard.responseError(rsp)) {
				refreshEnvironmentVarCache(rsp);
			}
			notecard.deleteResponse(rsp);
		}
  }

  // On a periodic basis, check the inbound queue for messages.  In a
  // real-world application, this would be checked using a frequency
  // commensurate with the required inbound responsiveness. For the most
  // common "periodic" mode applications, this might be daily or weekly.  In
  // this example, where we are using "continuous" mode, we check quite often
  // for demonstratio purposes.
  static unsigned long nextInboundPollMs = 0;
  if (millis() > nextInboundPollMs)
  {
      nextInboundPollMs = millis() + (INBOUND_NOTEFILE_POLL_SECS * 1000);

      // Process all pending inbound requests
      while (true)
      {
          // Get the next available note from our inbound queue notefile,
          // deleting it
          J *req = notecard.newRequest("note.get");
          JAddStringToObject(req, "file", INBOUND_NOTEFILE);
          JAddBoolToObject(req, "delete", true);
          J *rsp = notecard.requestAndResponse(req);
          if (rsp != NULL)
          {
              // If an error is returned, this means that no response is
              // pending.  Note that it's expected that this might return
              // either a "note does not exist" error if there are no pending
              // inbound notes, or a "file does not exist" error if the
              // inbound queue hasn't yet been created on the service.
              if (notecard.responseError(rsp))
              {
                  notecard.deleteResponse(rsp);
                  break;
              }

              // Get the note's body
              J *body = JGetObject(rsp, "body");
              if (body != NULL)
              {

                  // Simulate Processing the response here
                  notecard.logDebug("INBOUND REQUEST: ");
                  notecard.logDebug(JGetString(body, FIELD_POLL));
                  notecard.logDebug("\n\n");
              }
          }
          notecard.deleteResponse(rsp);
      }
  }
}

// Re-load all env vars, returning the modified time
void refreshEnvironmentVarCache(J *rsp)
{
  // Update the env modified time
  environmentModifiedTime = JGetNumber(rsp, "time");

  // Update the environment
  J *body = JGetObject(rsp, "body");
  if (body == NULL) {
		return;
	}

	// Update heartbeat period
	envHeartbeatMins = JAtoN(JGetString(body, VAR_MINUTES), NULL);
	if (envHeartbeatMins == 0) {
		envHeartbeatMins = DATA_SEND_MINS;
	}

	// See if the notecard has been rebooted, and reboot ourselves if so
  rsp = notecard.requestAndResponse(notecard.newRequest("card.status"));
	uint32_t lastBootTime = JGetNumber(rsp, "time");
	if (lastBootTime != 0) {
	  if (notecardBootTime != 0 && lastBootTime != notecardBootTime) {
			NVIC_SystemReset();
		}
		notecardBootTime = lastBootTime;
	}
  notecard.deleteResponse(rsp);
}

void sendDataToNotecard() {
  // Get simulated data from the card.temp and voltage
  double temperature = 0;
  J *rsp = notecard.requestAndResponse(notecard.newRequest("card.temp"));
  if (rsp != NULL)
  {
    temperature = JGetNumber(rsp, "value");
    notecard.deleteResponse(rsp);
  }

  double voltage = 0;
  rsp = notecard.requestAndResponse(notecard.newRequest("card.voltage"));
  if (rsp != NULL)
  {
    voltage = JGetNumber(rsp, "value");
    notecard.deleteResponse(rsp);
  }

  J *req = notecard.newRequest("note.add");
  if (req != NULL)
  {
    JAddBoolToObject(req, "sync", true);
    JAddStringToObject(req, "file", DATA_NOTEFILE);
    JAddNumberToObject(req, "port", DATA_PORT);
    J *body = JAddObjectToObject(req, "body");
    if (body != NULL)
    {
      JAddNumberToObject(body, "temperature", temperature);
      JAddNumberToObject(body, "voltage", voltage);
    }
    notecard.sendRequest(req);
  }
}
