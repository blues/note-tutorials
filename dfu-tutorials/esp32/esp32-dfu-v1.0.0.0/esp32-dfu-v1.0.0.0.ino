//
// Copyright 2020 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.
//
// This example extends the periodic communications example by adding support for
// DFU (Device Firmware Update) as well as WiFi triangulation, both of which
// utilize Espressif's esp-idf support library.
//

#include "main.h"
#include <Wire.h>
#include <seeed_bme680.h>

#ifndef ARDUINO_ARCH_ESP32
#error "this sketch exclusively targets the ESP32 because it uses esp-idf"
#endif

// C trickery to convert a number to a string
#define STRINGIFY(x) STRINGIFY_(x)
#define STRINGIFY_(x) #x

// Definitions used by firmware update
#define PRODUCT_ORG_NAME        ""
#define PRODUCT_DISPLAY_NAME    "Notecard ESP32 DFU Example"
#define PRODUCT_FIRMWARE_ID     "notecard-esp32-dfu-example-v1"
#define PRODUCT_DESC      ""
#define PRODUCT_MAJOR     1
#define PRODUCT_MINOR     0
#define PRODUCT_PATCH     0
#define PRODUCT_BUILD     0
#define PRODUCT_BUILT     __DATE__ " " __TIME__
#define PRODUCT_BUILDER     ""
#define PRODUCT_VERSION         STRINGIFY(PRODUCT_MAJOR) "." STRINGIFY(PRODUCT_MINOR) "." STRINGIFY(PRODUCT_PATCH)

// Define pin numbers based on the Feather and the Notecarrier-AF's user push button
#define buttonPin     21
#define buttonPressedState  LOW
#define ledPin        13

// Note that both of these definitions are optional; just prefix either line with // to remove it.
//  Remove serialNotecard if you wired your Notecard using I2C SDA/SCL pins instead of serial RX/TX
//  Remove serialDebug if you don't want the Notecard library to output debug information
//#define serialNotecard Serial1
#define serialDebugOut Serial

#define IIC_ADDR  uint8_t(0x76)
Seeed_BME680 bmeSensor(IIC_ADDR);

// This is the unique Product Identifier for your device.
#ifndef PRODUCT_UID
#define PRODUCT_UID ""
#endif
#define myProductID PRODUCT_UID
Notecard notecard;

// Button handling
#define BUTTON_IDLE     0
#define BUTTON_PRESS    1
#define BUTTON_DOUBLEPRESS  2
int buttonPress(void);

// One-time Arduino initialization
void setup() {

  // Initialize Arduino GPIO pins
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, buttonPressedState == LOW ? INPUT_PULLUP : INPUT);

  // During development, set up for debug output from the Notecard.  Note that the initial delay is
  // required by some Arduino cards before debug UART output can be successfully displayed in the
  // Arduino IDE, including the Adafruit Feather nRF52840 Express.
#ifdef serialDebugOut
    delay(2500);
    serialDebugOut.begin(115200);
    notecard.setDebugOutputStream(serialDebugOut);
  notecard.logDebugf("\n");
#endif

  // As the first thing, show the DFU partition information
  dfuShowPartitions();

  Wire.begin();
  
  if (!bmeSensor.init()) {
      notecard.logDebug("Could not find a valid BME680 sensor...");
  } else {
      notecard.logDebug("BME680 Connected...");
  }
  
  // Initialize the physical I/O channel to the Notecard
#ifdef serialNotecard
  notecard.begin(serialNotecard, 9600);
#else
  notecard.begin();
#endif

  // Configure for sync
  J *req = notecard.newRequest("hub.set");
  if (req != NULL) {
    if (myProductID[0]) {
      JAddStringToObject(req, "product", myProductID);
    }
    JAddStringToObject(req, "mode", "periodic");
    JAddNumberToObject(req, "outbound", 2);
    JAddNumberToObject(req, "inbound", 60);
    notecard.sendRequest(req);
  }

  // Notify the Notehub of our current firmware version
  req = notecard.newRequest("dfu.status");
  if (req != NULL) {
    JAddStringToObject(req, "version", firmwareVersion());
    notecard.sendRequest(req);
  }

}

// In the Arduino main loop which is called repeatedly, add outbound data every 15 seconds
void loop() {
  static unsigned long lastStatusMs = 0;

  // Wait for a button press, or perform idle activities
  int buttonState = buttonPress();
  switch (buttonState) {

  case BUTTON_IDLE:

    // Poll subsystems that need periodic servicing
    dfuPoll(false);
    
    // Display sync status on the debug console as a convenience, coming back
    // here after 2.5 seconds of sync inactivity
    if (notecard.debugSyncStatus(2500, 0))
      lastStatusMs = millis();

    // Periodically display a help message on the debug console
    if (millis() > lastStatusMs + 10000) {
      lastStatusMs = millis();
      notecard.logDebug("press button to simulate a sensor measurement; double-press to sync/dfu/wifi-scan\n");
    }

    return;

  case BUTTON_DOUBLEPRESS:
    digitalWrite(ledPin, HIGH);
    dfuPoll(true);
    notecard.requestAndResponse(notecard.newRequest("hub.sync"));
    digitalWrite(ledPin, LOW);
    return;
  }

  // Activity indicator
  digitalWrite(ledPin, HIGH);

  // The button was pressed, so we should begin a transaction
  notecard.logDebug("performing sensor measurement\n");
  lastStatusMs = millis();
  
  // Read the temp and humidity from the BME680
  if (bmeSensor.read_sensor_data()) {
    notecard.logDebug("Failed to obtain a reading...");
    return;
  }
 
  // Read the notecard's current voltage
  double voltage = 0;
  J *rsp = notecard.requestAndResponse(notecard.newRequest("card.voltage"));
  if (rsp != NULL) {
    voltage = JGetNumber(rsp, "value");
    notecard.deleteResponse(rsp);
  }

  // Enqueue the measurement to the Notecard for transmission to the Notehub.  These measurements
  // will be staged in the Notecard's flash memory until it's time to transmit them to the service.
  J *req = notecard.newRequest("note.add");
  if (req != NULL) {
    J *body = JCreateObject();
    if (body != NULL) {
      JAddNumberToObject(body, "notecard_voltage", voltage);
      JAddNumberToObject(body, "temp", bmeSensor.sensor_result_value.temperature);
      JAddNumberToObject(body, "humidity", bmeSensor.sensor_result_value.humidity);
      JAddItemToObject(req, "body", body);
    }
    notecard.sendRequest(req);
  }

  // Done with transaction
  digitalWrite(ledPin, LOW);

}

// Button handling
int buttonPress() {

  // Detect the "press" transition
  static bool buttonBeingDebounced = false;
  int buttonState = digitalRead(buttonPin);
  if (buttonState != buttonPressedState) {
    if (buttonBeingDebounced) {
      buttonBeingDebounced = false;
    }
    return BUTTON_IDLE;
  }
  if (buttonBeingDebounced)
    return BUTTON_IDLE;

  // Wait to see if this is a double-press
  bool buttonDoublePress = false;
  bool buttonReleased = false;
  unsigned long buttonPressedMs = millis();
  unsigned long ignoreBounceMs = 100;
  unsigned long doublePressMs = 750;
  while (millis() < buttonPressedMs+doublePressMs || digitalRead(buttonPin) == buttonPressedState) {
    if (millis() < buttonPressedMs+ignoreBounceMs)
      continue;
    if (digitalRead(buttonPin) != buttonPressedState) {
      if (!buttonReleased)
        buttonReleased = true;
      continue;
    }
    if (buttonReleased) {
      buttonDoublePress = true;
      if (digitalRead(buttonPin) != buttonPressedState)
        break;
    }
  }

  return (buttonDoublePress ? BUTTON_DOUBLEPRESS : BUTTON_PRESS);

}

// This is a product configuration JSON structure that enables the Notehub to recognize this
// firmware when it's uploaded, to help keep track of versions and so we only ever download
// firmware buildss that are appropriate for this device.
#define QUOTE(x) "\"" x "\""
#define FIRMWARE_VERSION_HEADER "firmware::info:"
#define FIRMWARE_VERSION FIRMWARE_VERSION_HEADER            \
    "{" QUOTE("org") ":" QUOTE(PRODUCT_ORG_NAME)            \
    "," QUOTE("product") ":" QUOTE(PRODUCT_DISPLAY_NAME)    \
    "," QUOTE("description") ":" QUOTE(PRODUCT_DESC)        \
    "," QUOTE("firmware") ":" QUOTE(PRODUCT_FIRMWARE_ID)    \
    "," QUOTE("version") ":" QUOTE(PRODUCT_VERSION)         \
    "," QUOTE("built") ":" QUOTE(PRODUCT_BUILT)       \
    "," QUOTE("ver_major") ":" STRINGIFY(PRODUCT_MAJOR)   \
    "," QUOTE("ver_minor") ":" STRINGIFY(PRODUCT_MINOR)   \
    "," QUOTE("ver_patch") ":" STRINGIFY(PRODUCT_PATCH)   \
    "," QUOTE("ver_build") ":" STRINGIFY(PRODUCT_BUILD)   \
    "," QUOTE("builder") ":" QUOTE(PRODUCT_BUILDER)   \
    "}"

// In the Arduino IDE, the ino is built regardless of whether or not it is modified.  As such, it's a perfect
// place to serve up the build version string because __DATE__ and __TIME__ are updated properly for each build.
const char *productVersion() {
    return ("Ver " PRODUCT_VERSION " " PRODUCT_BUILT);
}

// Return the firmware's version, which is both stored within the image and which is verified by DFU
const char *firmwareVersion() {
    return &FIRMWARE_VERSION[strlen(FIRMWARE_VERSION_HEADER)];
}
