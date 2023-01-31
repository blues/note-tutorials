#pragma once

/**
 * @brief This file should be included in your project to describe the firmware and version being compiled.
 * 
 */

// Definitions used by firmware update
#define PRODUCT_ORG_NAME        ""
#define PRODUCT_DISPLAY_NAME    "Notecard ESP8266 Example"
#define PRODUCT_FIRMWARE_ID     "notecard-esp8266-example"
#define PRODUCT_DESC            ""
#define PRODUCT_MAJOR           1
#define PRODUCT_MINOR           0
#define PRODUCT_PATCH           0
#define PRODUCT_BUILD           0
#define PRODUCT_BUILT           __DATE__ " " __TIME__
#define PRODUCT_BUILDER         ""
#define PRODUCT_VERSION         STRINGIFY(PRODUCT_MAJOR) "." STRINGIFY(PRODUCT_MINOR) "." STRINGIFY(PRODUCT_PATCH)

