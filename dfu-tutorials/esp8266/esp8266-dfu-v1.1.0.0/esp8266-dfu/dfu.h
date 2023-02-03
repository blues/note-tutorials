//
// Copyright 2023 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.
#pragma once

// C trickery to convert a number to a string
#define STRINGIFY(x) STRINGIFY_(x)
#define STRINGIFY_(x) #x

#include "dfu-version.h"
#include "dfu-updater.h"
#include <chrono>

using namespace std::literals::chrono_literals;

extern Notecard notecard;

struct DFUConfig {
 

    /**
     * @brief The number of attempts made to send a request to the Notecard. When the request cannot be sent within
     * this number of attempts, the update process is errored out.
     */
    static const uint8_t DEFAULT_REQUEST_ATTEMPTS = 5;
    constexpr static std::chrono::microseconds DEFAULT_REQUEST_INTERVAL = 250ms;
    constexpr static std::chrono::microseconds DEFAULT_CHECK_DFU_AVAILABLE_INTERVAL = 10min;
    constexpr static std::chrono::microseconds DEFAULT_AIT_FOR_DFU_TIMEOUT = 2min;


    /*
     * The number of times to retry a request if it fails.
    */
    uint8_t requestAttempts = DEFAULT_REQUEST_ATTEMPTS;

    /**
     * @brief The duration between polling 
     */
    std::chrono::microseconds requestInterval = DEFAULT_REQUEST_INTERVAL;

    std::chrono::microseconds checkDFUAvailableInterval = DEFAULT_CHECK_DFU_AVAILABLE_INTERVAL;

    std::chrono::microseconds waitForDFUTimeout = DEFAULT_AIT_FOR_DFU_TIMEOUT;

};

/**
 * @brief Perform initial setup of Host DFU.
 * 
 * @param config    Configuration parameters controlling the DFU process.
 * @return true     DFU setup was successful.
 * @return false    DFU setup was not successful.
 */
bool dfuSetup(const DFUConfig& config = DFUConfig());

/**
 * @brief "Feeds" the DFU task, which breaks up DFU operations into small pieces so that it is non-blocking. Typically, each
 * call to dfuPoll() sends one request to the Notecard.
 * 
 * @param force     Determines whether the DFU task should act periodically or immediately.
 *  When false, the DFU task will self-govern how often it communicates with the Notecard.
 *  When true, the request is sent immediately without any delay. This is typically done in response to user input, such
 *  as pressing a button, or some other infrequent event.
 * 
 * @return uint32_t Duration in milliseconds that the task will wait before proceeding with the next DFU related operation. This is advisory
 * only - you can call dfuPoll() as often as you wish. In cases where the system is running with Cooperative or Preemptive
 * multitasking (e.g. an RTOS) this value can be used to put the task to sleep for the given duration.
 */
uint32_t dfuPoll(bool force=false);

// org name is optional
#ifndef PRODUCT_ORG_NAME
#define PRODUCT_ORG_NAME ""
#endif

// display name must be defined
#ifndef PRODUCT_DISPLAY_NAME
#error PRODUCT_DISPLAY_NAME not defined
#endif

// description is optional
#ifndef PRODUCT_DESC
#define PRODUCT_DESC ""
#endif

#ifndef PRODUCT_FIRMWARE_ID
#error PRODUCT_FIRMWARE_ID not defined. Please define it to identify the firmware, e.g. my-product-v1
#endif

#ifndef PRODUCT_MAJOR
#error PRODUCT_MAJOR is not defined. This must be set to the major version of the firmware being built.
#endif

#ifndef PRODUCT_MINOR
#error PRODUCT_MINOR is not defined. This must be set to the major version of the firmware being built.
#endif

#ifndef PRODUCT_PATCH
#define PRODUCT_PATCH 0
#endif

#ifndef PRODUCT_BUILD
#define PRODUCT_BUILD 0
#endif

// PRODUCT_VERSION is optional. The version string is built from the version number components
#ifndef PRODUCT_VERSION
#define PRODUCT_VERSION         STRINGIFY(PRODUCT_MAJOR) "." STRINGIFY(PRODUCT_MINOR) "." STRINGIFY(PRODUCT_PATCH)
#endif

// PRODUCT_BUILT defaults to the current built date and time
#ifndef PRODUCT_BUILT
#define PRODUCT_BUILT           __DATE__ " " __TIME__
#endif

// PRODUCT_BUILDER is optional
#ifndef PRODUCT_BUILDER
#define PRODUCT_BUILDER         ""
#endif


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
    "," QUOTE("built") ":" QUOTE(PRODUCT_BUILT)             \
    "," QUOTE("ver_major") ":" STRINGIFY(PRODUCT_MAJOR)     \
    "," QUOTE("ver_minor") ":" STRINGIFY(PRODUCT_MINOR)     \
    "," QUOTE("ver_patch") ":" STRINGIFY(PRODUCT_PATCH)     \
    "," QUOTE("ver_build") ":" STRINGIFY(PRODUCT_BUILD)     \
    "," QUOTE("builder") ":" QUOTE(PRODUCT_BUILDER)         \
    "}"

// In the Arduino IDE, the ino is built regardless of whether or not it is modified.  As such, it's a perfect
// place to serve up the build version string because __DATE__ and __TIME__ are updated properly for each build.
constexpr const char *productVersion() {
    return ("Ver " PRODUCT_VERSION " " PRODUCT_BUILT);
}

// Return the firmware's version, which is both stored within the image and which is verified by DFU
constexpr const char *firmwareVersion() {
    return &FIRMWARE_VERSION[strlen(FIRMWARE_VERSION_HEADER)];
}
