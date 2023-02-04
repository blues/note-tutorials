//
// Copyright 2023 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.
#pragma once

#ifdef ARDUINO_ARCH_ESP8266

#include "notecard-host-dfu.h"
#include "Updater.h"
#include "check_version.h"


class ArduinoESP8266DFUUpdater {
public:
    bool setup() {
        static_assert(check_version(version(ARDUINO_ESP8266_MAJOR, ARDUINO_ESP8266_MINOR), version(3, 1)), "ESP8266 version should be 3.1.0 or newer");

        notecard.logDebugf("dfu: free space %d bytes\n", ESP.getFreeSketchSpace());

        // install callbacks to the updater
        Update.onError([](uint8_t error) {
            notecard.logDebug(Update.getErrorString().c_str());
            notecard.logDebug("\n");
        });

        Update.onProgress([](size_t complete, size_t total) {
            notecard.logDebugf("DFU progress: %d/%d (%d%%)\n", complete, total, (complete / total) * 100); 
        });
        return true;
    }

    bool begin(size_t imageSize) {
        return Update.begin(imageSize, U_FLASH);
    }

    bool write(const uint8_t* buffer, size_t length) {
        // first parameter should be const
        return Update.write((uint8_t*)buffer, length);
    }

    bool end() {
        return Update.end();
    }

    bool hasError() {
        return Update.hasError();
    }

    String getError() {
        return Update.getErrorString();
    }

    bool executeImage() {
        ESP.restart(); // cleaner than ESP.reset()
        return true;
    }
};

typedef ArduinoESP8266DFUUpdater DFUUpdater;

#endif