#include <Arduino.h>
#include "Notecard.h"


#ifndef PRODUCT_ID
#define PRODUCT_ID ""
#endif
#pragma message ( "PRODUCT ID:" PRODUCT_ID )

#define LEVEL_0   0
#define LEVEL_ALL -1
#define DEFAULT_BLINK_PERIOD_MS 1000
#define LED_PIN 13

void checkConfig(void);
void blinkLED(void);

uint32_t blinkPeriodMs = DEFAULT_BLINK_PERIOD_MS;

void setup() {
  // Configure GPIO Pin to control LED
  pinMode(LED_PIN, OUTPUT);

  // Configure Serial Debug
  delay(2500);
  Serial.begin(115200);
  NoteSetDebugOutputStream(Serial);

  // Configure Note-C Interface to Notecard
  NoteInitSerial(Serial1, 9600);

  // Configure Product ID
  NoteSetProductID(PRODUCT_ID);

  // Configure Sync
  int syncPeriodMinutes = 1;
  NoteSetUploadMode("continuous", syncPeriodMinutes, false);

  // Run checks that the app is configured correctly
  checkConfig();

}

#define POLL_INTERVAL_SEC 10

void loop() {
  // Toggle the LED as necessary
  blinkLED();

  // Display sync status on debug Output Stream
  int verbosity = LEVEL_0;      // <-- basic network sync messages
  // int verbosity = LEVEL_ALL  // <-- all available network sync messages
  NoteDebugSyncStatus(2500, verbosity);

  static uint32_t nextPollTime = 0;
  // Check if time period for polling has expired
  if (millis() < nextPollTime)
    return;
  
  nextPollTime = millis() + POLL_INTERVAL_SEC * 1000;

  // Process all notes in the inbound queue
  while(true){
    // Build request for note in queue
    J* req = NoteNewRequest("note.get");
    JAddStringToObject(req, "file", "led-command.qi");
		JAddBoolToObject(req, "delete", true);

    // Request note from Notecard
    J* rsp = NoteRequestResponse(req);
    if(rsp == NULL)
      return;
    
    //Check for error in the response
    if (NoteResponseError(rsp)) {
      // If the Response contains an error, 
      // assume there are no more notes in the queue
      NoteDeleteResponse(rsp);
      return;
    }

    // Get body of the note
    J* body = JGetObject(rsp, "body");
    if (body == NULL){
      NoteDeleteResponse(rsp);
      return;
    }
    
    // Extract value of "blinkPeriod" from body
    if (JIsPresent(body, "blinkPeriod")){
      blinkPeriodMs = JGetInt(body, "blinkPeriod");
    
      // Display value on debug stream
      NotePrintf("Blink Period: %dms\n\n", blinkPeriodMs);
    }

    // Delete Response Object
    NoteDeleteResponse(rsp);

  }

}

bool getProductId(char* productId, unsigned int bufferLength)
{
  if (productId == NULL)
    return(false);
  
  NoteSuspendTransactionDebug();
  J *rsp = NoteRequestResponse(NoteNewRequest("service.get"));
  NoteResumeTransactionDebug();

  if (rsp == NULL)
    return(false);
  
  if (NoteResponseError(rsp))
  {
    NoteDeleteResponse(rsp);
    return(false);
  }

  strlcpy(productId, JGetString(rsp, "product"), bufferLength);

  NoteDeleteResponse(rsp);
  return(true);

}

void startTrap(void)
{
  while(true){delay(200);};
}

void checkConfig(void)
{
  char productId[128] = {0};


  bool success = getProductId(productId, sizeof(productId));
  if(!success || productId[0] == '\0')
  {
    NoteFnDebugMsg("Product ID for Notecard is not configured.\nHalting execution!\n");
    startTrap();
  }
  NoteFnDebug("Product Id: %s\n", productId);

}

void blinkLED(void){
  static uint32_t nextBlinkTime = 0;
  if( millis() < nextBlinkTime)
    return;
  nextBlinkTime = millis() + blinkPeriodMs/2;
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));

}