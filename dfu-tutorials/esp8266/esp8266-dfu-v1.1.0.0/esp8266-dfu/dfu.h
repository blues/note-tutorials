#pragma once

// C trickery to convert a number to a string
#define STRINGIFY(x) STRINGIFY_(x)
#define STRINGIFY_(x) #x

#include "dfu-version.h"


extern Notecard notecard;

// dfu.cpp
void dfuSetup();
void dfuPoll(bool force);


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
constexpr inline const char *productVersion() {
    return ("Ver " PRODUCT_VERSION " " PRODUCT_BUILT);
}

// Return the firmware's version, which is both stored within the image and which is verified by DFU
constexpr inline const char *firmwareVersion() {
    return &FIRMWARE_VERSION[strlen(FIRMWARE_VERSION_HEADER)];
}