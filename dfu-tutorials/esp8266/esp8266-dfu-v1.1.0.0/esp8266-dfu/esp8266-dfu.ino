//
// Copyright 2023 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.

#include <Notecard.h>
#include <notecard-cpp.h>
#include "dfu.h"


#if defined(ARDUINO_ARCH_ESP8266)
    static_assert(check_version(version(ARDUINO_ESP8266_MAJOR, ARDUINO_ESP8266_MINOR), version(3, 1)), "ESP8266 version should be 3.1.0 or newer");
    #define buttonPin           NOT_A_PIN
    #define buttonPressedState  LOW
#elif defined(ARDUINO_ARCH_ESP32)
//static_assert(check_version(version(ARDUINO_ESPRESSIF32_MAJOR, ARDUINO_ESPRESSIF_MINOR), version(3, 1)), "ESP32 version should be 3.1.0 or newer");
    #define buttonPin           21
    #define buttonPressedState  LOW
#else
    #error "this sketch exclusively targets the ESP8266 or the ESP32."
#endif

#define DFU_ENABLED true

// Define pin numbers based on the Feather and the Notecarrier-AF's user push button
#define ledPin              LED_BUILTIN

// Note that both of these definitions are optional; just prefix either line with // to remove it.
//  Remove serialNotecard if you wired your Notecard using I2C SDA/SCL pins instead of serial RX/TX
//  Remove serialDebug if you don't want the Notecard library to output debug information
//#define serialNotecard Serial1
#define serialDebugOut Serial

// This is the unique Product Identifier for your device.
#ifndef PRODUCT_UID
#define PRODUCT_UID ""
#endif

Notecard notecard;

// Button handling
#define BUTTON_IDLE         0
#define BUTTON_PRESS        1
#define BUTTON_DOUBLEPRESS  2
int buttonPress(void);

// One-time Arduino initialization
void setup() {
    // Initialize Arduino GPIO pins
    pinMode(ledPin, OUTPUT);
    if (buttonPin!=NOT_A_PIN) {
        pinMode(buttonPin, buttonPressedState == LOW ? INPUT_PULLUP : INPUT);
    }
    // During development, set up for debug output from the Notecard.  Note that the initial delay is
    // required by some Arduino cards before debug UART output can be successfully displayed in the
    // Arduino IDE, including the Adafruit Feather nRF52840 Express.
#ifdef serialDebugOut
    delay(2500);
    serialDebugOut.begin(115200);
    notecard.setDebugOutputStream(serialDebugOut);
    notecard.logDebugf("\n");
#endif

    // Initialize the physical I/O channel to the Notecard
#ifdef serialNotecard
    notecard.begin(serialNotecard, 9600);
#else
    notecard.begin();
#endif

    // Configure for sync
    J *req = notecard.newRequest("hub.set");
    if (req != NULL) {
        if (PRODUCT_UID[0]) {
            JAddStringToObject(req, "product", PRODUCT_UID);
        }
        JAddStringToObject(req, "mode", "periodic");
        JAddNumberToObject(req, "outbound", 1);
        JAddNumberToObject(req, "inbound", 1);
        notecard.sendRequest(req);
    }

    // Notify the Notehub of our current firmware version
#if DFU_ENABLED
    DFUConfig config = {
        .checkDFUAvailableInterval = 24h
    };
    dfuSetup(config);
#endif
}


// In the Arduino main loop which is called repeatedly, add outbound data every 15 seconds
void loop() {
    static unsigned long lastStatusMs = 0;
    // Wait for a button press, or perform idle activities
    int buttonState = buttonPress();
    switch (buttonState) {

    case BUTTON_IDLE:
        // Poll subsystems that need periodic servicing
#if DFU_ENABLED
        dfuPoll(false);

        // Display sync status on the debug console as a convenience, coming back
        // here after 2.5 seconds of sync inactivity
        if (notecard.debugSyncStatus(2500, 0))
            lastStatusMs = millis();

        // Periodically display a help message on the debug console
        if (millis() > lastStatusMs + 10000) {
            lastStatusMs = millis();
            notecard.sendRequest(notecard.newRequest("dfu.status")); // (just to show current status in debug output)
            notecard.logDebug("press button to simulate a sensor measurement; double-press to sync/dfu/wifi-scan\n");
        }
#endif
        break;

    case BUTTON_DOUBLEPRESS:
        digitalWrite(ledPin, HIGH);
#if DFU_ENABLED
        dfuPoll(true);
#endif
        notecard.requestAndResponse(notecard.newRequest("hub.sync"));
        digitalWrite(ledPin, LOW);
        return;

    }

    // Activity indicator
    digitalWrite(ledPin, HIGH);

    static uint32_t lastEvent = millis();

    if ((millis()-lastEvent)<5000) 
        return;
    lastEvent = millis();

	// Count the simulated measurements that we send to the cloud, and stop the demo before long.
    static unsigned eventCounter = 0;
    if (eventCounter++ > 25)
        return;

    // Read the notecard's current temperature and voltage, as simulated sensor measurements
    double temperature = 0;
    J *rsp = notecard.requestAndResponse(notecard.newRequest("card.temp"));
    if (rsp != NULL) {
        temperature = JGetNumber(rsp, "value");
        notecard.deleteResponse(rsp);
    }
#if PRODUCT_MINOR >= 1
    double voltage = 0;
    rsp = notecard.requestAndResponse(notecard.newRequest("card.voltage"));
    if (rsp != NULL) {
        voltage = JGetNumber(rsp, "value");
        notecard.deleteResponse(rsp);
    }
#endif
    // Enqueue the measurement to the Notecard for transmission to the Notehub.	 These measurements
    // will be staged in the Notecard's flash memory until it's time to transmit them to the service.
    J *req = notecard.newRequest("note.add");
    if (req != NULL) {
        J *body = JCreateObject();
        if (body != NULL) {
            JAddNumberToObject(body, "temp", temperature);
            JAddNumberToObject(body, "count", eventCounter);
#if PRODUCT_MINOR >= 1
            JAddNumberToObject(body, "voltage", voltage);
#endif
            JAddItemToObject(req, "body", body);
        }
        notecard.sendRequest(req);
        notecard.logDebug("note added.\n");
    }
    // Done with transaction
    digitalWrite(ledPin, LOW);
}

// Button handling
int buttonPress() {
    if (buttonPin==NOT_A_PIN) {
        return BUTTON_IDLE;
    }

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
    const unsigned long ignoreBounceMs = 100;
    const unsigned long doublePressMs = 750;
    while (millis() < buttonPressedMs+doublePressMs || digitalRead(buttonPin) == buttonPressedState) {
        yield();
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
