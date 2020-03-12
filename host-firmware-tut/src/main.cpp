#include <Arduino.h>
#include "Notecard.h"

#ifndef PRODUCT_ID
#define PRODUCT_ID ""
#endif
#pragma message ( "PRODUCT ID:" PRODUCT_ID )

#define LEVEL_0   0
#define LEVEL_ALL -1

void checkConfig(void);
void getSensorMeasurements(double*, double*);
bool buttonPress(void);

void setup() {
  // Configure Serial Debug
  delay(2500);
  Serial.begin(115200);
  NoteSetDebugOutputStream(Serial);

  // Configure Note-C Interface to Notecard
  NoteInitSerial(Serial1, 9600);

  // Configure Product ID
  NoteSetProductID(PRODUCT_ID);

  // Configure Sync
  NoteSetUploadMode("minimum", 0 , false);

  // Run checks that the app is configured correctly
  checkConfig();

}

bool forceSync = false;

void loop() {
  // Display sync status on debug Output Stream
  int verbosity = LEVEL_0;      // <-- basic network sync messages
  // int verbosity = LEVEL_ALL  // <-- all available network sync messages
  NoteDebugSyncStatus(2500, verbosity);
  
  // Process Button Presses
  if(!buttonPress())
    return;
  
  if (forceSync){
    // Request Notecard to sync
    NoteRequest(NoteNewRequest("service.sync"));
    
    forceSync = false;
    return;
  }

  // Perform Sensor Measurement
  double temperature, humidity;
  getSensorMeasurements(&temperature, &humidity);

  // Add sensor data to outbound queue on Notecard
  J* req = NoteNewRequest("note.add");
  if (req == NULL)
    return;
  
  JAddStringToObject(req, "file", "sensors.qo");
  J* body = JCreateObject();
  if (body == NULL)
    return;
  
  JAddNumberToObject(body, "temperature", temperature);
  JAddNumberToObject(body, "humidity", humidity);
  JAddItemToObject(req, "body", body);

  NoteRequest(req);

  // Check Timer-based Tasks


}

void getSensorMeasurements(double* temperature, double* humidity){

  // Simulated Sensor Measurements
  bool status = NoteGetTemperature(temperature);
  // [TODO] Status always returns false: bug
  status = NoteGetVoltage(humidity);

  *humidity *= 10;

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


#define BUTTON_PIN      7     // GPIO pin connected to button
#define BUTTON_PRESSED  LOW   //When button is pressed, is pin state HIGH or LOW

bool clickDetected(){
  const unsigned int IDLE = 0;
  const unsigned int WAIT = 1;
  const unsigned int debounceIntervalMs = 100;

  static unsigned int state = IDLE;
  static unsigned int debounceThreshold = 0;

  bool button = digitalRead(BUTTON_PIN);
  if(button != BUTTON_PRESSED && state == IDLE)
    return(false);
  
  if(button == BUTTON_PRESSED && state == IDLE){
    debounceThreshold = millis() + debounceIntervalMs;
    state = WAIT;
    return(false);
  }
  
  if(button == BUTTON_PRESSED && state == WAIT)
    return(false);

  if(button != BUTTON_PRESSED && state == WAIT && millis() < debounceThreshold){
    state = IDLE;
    return(false);
  }

  state = IDLE;
  return(true);

}

#define CLICK_NONE   0
#define CLICK_SINGLE 1
#define CLICK_DOUBLE 2

unsigned int getClickType(){
  const unsigned int doubleClickIntervalMs = 750;

  static bool isWaitingForDoubleClick = false;
  static unsigned int doubleClickThreshold = 0;

  if (isWaitingForDoubleClick && millis() >= doubleClickThreshold){
    isWaitingForDoubleClick = false;
    return(CLICK_SINGLE);
  }

  bool isClick = clickDetected();
  if(!isClick)
    return(CLICK_NONE);
  
  if (isWaitingForDoubleClick){
    isWaitingForDoubleClick = false;
    return(CLICK_DOUBLE);
  }

  isWaitingForDoubleClick = true;
  doubleClickThreshold = millis() + doubleClickIntervalMs;
  return(CLICK_NONE);
  
}

bool buttonPress(){
  unsigned int c = getClickType();
  if (c == CLICK_NONE)
    return(false);
  
  forceSync = (c == CLICK_DOUBLE);
  
  return(true);

}


