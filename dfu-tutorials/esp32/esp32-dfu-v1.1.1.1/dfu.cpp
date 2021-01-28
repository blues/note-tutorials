// Copyright 2019 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.

// Device Firmware Update support.  (Note that this is only currently supported on ESP32.)
// As a note to the reader: sometimes firmware update is referred to as "OTA" (Over The Air) or
// "FOTA" (Firmware Over The Air).  Technically, this code is reading the already-downloaded and
// already-verified firmware from the Notecard's storage in a fully-offline manner, which is why
// we use the more generic DFU (Device Firmware Update) term of art.

#include "main.h"
#include <Arduino.h>
#include <Notecard.h>
#include "esp_partition.h"
#include "esp_system.h"
#include "esp_ota_ops.h"
#include "esp_flash_partitions.h"

// Display DFU partition information
void dfuShowPartitions() {
    const esp_partition_t *partition;

    notecard.logDebugf("ESP32 PARTITION SCHEME (should be two partitions to support OTA)\n");

    partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, "app0");
    if (partition == NULL)
        notecard.logDebugf("   partition app0: not found\n");
    else
        notecard.logDebugf("   partition that should be 'app0' is '%s' at 0x%08x (%d bytes)\n", partition->label, partition->address, partition->size);

    partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, "app1");
    if (partition == NULL)
        notecard.logDebugf("   partition app1: not found\n");
    else
        notecard.logDebugf("   partition that should be 'app1' is '%s' at 0x%08x (%d bytes)\n", partition->label, partition->address, partition->size);

}

// Process DFU
void dfuPoll(bool force) {
    static uint32_t dfuCheckMs = 0;
    static uint32_t serviceIdleCheckMs = 0;

    // Suppress how often we check
    if (!force && dfuCheckMs != 0 && millis() < dfuCheckMs + ms1Hour)
        return;

    // Even if DFU is ready, only check the notehub status once every 10s max
    if (!force && serviceIdleCheckMs != 0 && millis() < serviceIdleCheckMs + 10*ms1Sec)
        return;
    serviceIdleCheckMs = millis();

    // Check status, and determine both if there is an image ready, and if the image is NEW.
    bool imageIsReady = false;
    bool imageIsSameAsCurrent = false;
    char imageMD5[NOTE_MD5_HASH_STRING_SIZE] = {0};
    uint32_t imageLength = 0;
    if (J *rsp = notecard.requestAndResponse(notecard.newRequest("dfu.status"))) {
        if (strcmp(JGetString(rsp, "mode"), "ready") == 0) {
            imageIsReady = true;
            if (J *body = JGetObjectItem(rsp, "body")) {
                imageLength = JGetInt(body, "length");
                strlcpy(imageMD5, JGetString(body, "md5"), sizeof(imageMD5));
                imageIsSameAsCurrent = strcmp(JGetString(body, "version"), firmwareVersion()) == 0;
                if (!imageIsSameAsCurrent) {
                    notecard.logDebugf("dfu: replacing current image: %s\n", productVersion());
                    notecard.logDebugf("dfu:   with downloaded image: %s\n", JGetString(body, "name"));
                }
            }
        }
        notecard.deleteResponse(rsp);
    }

    // Exit if same version or no DFU to process
    if (!imageIsReady || imageIsSameAsCurrent || imageLength == 0) {
        dfuCheckMs = millis();
        notecard.logDebugf("dfu: no image is ready for firmware update\n");
        return;
    }

    // Enter DFU mode.  Note that the Notecard will automatically switch us back out of
    // DFU mode after 15m, so we don't leave the notecard in a bad state if we had a problem here.
    if (J *req = notecard.newRequest("hub.set")) {
        JAddStringToObject(req, "mode", "dfu");
        notecard.sendRequest(req);
    }

    // Proceed with DFU
    dfuCheckMs = millis();

    // Wait until we have successfully entered the mode.  The fact that this loop isn't
    // just an infinitely loop is simply defensive programming.  If for some odd reason
    // we don't enter DFU mode, we'll eventually come back here on the next DFU poll.
    bool inDFUMode = false;
    int beganDFUModeCheck = millis();
    while (!inDFUMode && millis() < beganDFUModeCheck + (2 * ms1Min)) {
        if (J *rsp = notecard.requestAndResponse(notecard.newRequest("dfu.get"))) {
            if (!notecard.responseError(rsp))
                inDFUMode = true;
            notecard.deleteResponse(rsp);
        }
        if (!inDFUMode)
            delay(2500);
    }

    // If we failed, leave DFU mode immediately
    if (!inDFUMode) {
        if (J *req = notecard.newRequest("hub.set")) {
            JAddStringToObject(req, "mode", "dfu-completed");
            notecard.sendRequest(req);
        }
        return;
    }

    // The image is ready.  If the version is the same as what's in memory, then of course don't
    // bother to do the update.
    esp_err_t err;
    // update handle : set by esp_ota_begin(), must be freed via esp_ota_end()
    esp_ota_handle_t update_handle = 0 ;
    const esp_partition_t *update_partition = NULL;
    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();
    if (configured != running) {
        notecard.logDebugf("dfu: configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x\n",
                configured->address, running->address);
        notecard.logDebugf("     (This can happen if either the OTA boot data or preferred boot image become corrupted.)\n");
    }
    notecard.logDebugf("dfu: running partition type %d subtype %d (offset 0x%08x)\n", running->type, running->subtype, running->address);

    update_partition = esp_ota_get_next_update_partition(NULL);
    if (update_partition == NULL)   // simply being defensive
        return;

    notecard.logDebugf("dfu: writing to partition subtype %d at offset 0x%x\n", update_partition->subtype, update_partition->address);

    // Begin the update
    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        notecard.logDebugf("esp_ota_begin failed (%s)\n", esp_err_to_name(err));
        return;
    }

    notecard.logDebugf("dfu: beginning firmware update\n");

    // Loop over received chunks
    int offset = 0;
    int chunklen = 4096;
    int left = imageLength;
    NoteMD5Context md5Context;
    NoteMD5Init(&md5Context);
    while (left) {

        // Read next chunk from card
        int thislen = chunklen;
        if (left < thislen)
            thislen = left;

        // If anywhere, this is the location of the highest probability of I/O error
        // on the I2C or serial bus, simply because of the amount of data being transferred.
        // As such, it's a conservative measure just to retry.
        char *payload = NULL;
        for (int retry=0; retry<5; retry++) {
            notecard.logDebugf("dfu: reading chunk (offset:%d length:%d try:%d)\n", offset, thislen, retry+1);

            // Request the next chunk from the notecard
            J *req = notecard.newRequest("dfu.get");
            if (req == NULL) {
                notecard.logDebugf("dfu: insufficient memory\n");
                return;
            }
            JAddNumberToObject(req, "offset", offset);
            JAddNumberToObject(req, "length", thislen);
            J *rsp = notecard.requestAndResponse(req);
            if (rsp == NULL) {
                notecard.logDebugf("dfu: insufficient memory\n");
                return;
            }
            if (notecard.responseError(rsp)) {
                notecard.logDebugf("dfu: error on read: %s\n", JGetString(rsp, "err"));
            } else {
                char *payloadB64 = JGetString(rsp, "payload");
                if (payloadB64[0] == '\0') {
                    notecard.logDebugf("dfu: no payload\n");
                    notecard.deleteResponse(rsp);
                    return;
                }
                payload = (char *) malloc(JB64DecodeLen(payloadB64));
                if (payload == NULL) {
                    notecard.logDebugf("dfu: can't allocate payload decode buffer\n");
                    notecard.deleteResponse(rsp);
                    return;
                }
                int actuallen = JB64Decode(payload, payloadB64);
                const char *expectedMD5 = JGetString(rsp, "status");;
                char chunkMD5[NOTE_MD5_HASH_STRING_SIZE] = {0};
                NoteMD5HashString((uint8_t *)payload, actuallen, chunkMD5, sizeof(chunkMD5));
                if (actuallen == thislen && strcmp(chunkMD5, expectedMD5) == 0) {
                    notecard.deleteResponse(rsp);
                    break;
                }

                free(payload);
                payload = NULL;

                if (thislen != actuallen)
                    notecard.logDebugf("dfu: decoded data not the correct length (%d != actual %d)\n", thislen, actuallen);
                else
                    notecard.logDebugf("dfu: %d-byte decoded data MD5 mismatch (%s != actual %s)\n", actuallen, expectedMD5, chunkMD5);
            }

            notecard.deleteResponse(rsp);
        }
        if (payload == NULL) {
            notecard.logDebugf("dfu: unrecoverable error on read\n");
            return;
        }

        // MD5 the chunk
        NoteMD5Update(&md5Context, (uint8_t *)payload, thislen);

        // Write the chunk
        err = esp_ota_write(update_handle, (const void *)payload, thislen);
        if (err != ESP_OK) {
            free(payload);
            return;
        }

        // Move to next chunk
        free(payload);
        notecard.logDebugf("dfu: successfully transferred offset:%d len:%d\n", offset, thislen);
        offset += thislen;
        left -= thislen;
    }

    // Exit DFU mode.  (Had we not done this, the Notecard exits DFU mode automatically after 15m.)
    if (J *req = notecard.newRequest("hub.set")) {
        JAddStringToObject(req, "mode", "dfu-completed");
        notecard.sendRequest(req);
    }

    // Done
    if (esp_ota_end(update_handle) != ESP_OK) {
        notecard.logDebugf("esp_ota_end failed!\n");
        return;
    }

    // Validate the MD5
    uint8_t md5Hash[NOTE_MD5_HASH_SIZE];
    NoteMD5Final(md5Hash, &md5Context);
    char md5HashString[NOTE_MD5_HASH_STRING_SIZE];
    NoteMD5HashToString(md5Hash, md5HashString, sizeof(md5HashString));
    notecard.logDebugf("dfu:    MD5 of image: %s\n", imageMD5);
    notecard.logDebugf("dfu: MD5 of download: %s\n", md5HashString);
    if (strcmp(imageMD5, md5HashString) != 0) {
        notecard.logDebugf("MD5 MISMATCH - ABANDONING DFU\n");
        return;
    }

    // Set the boot partition and reboot
    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        notecard.logDebugf("dfu: restart failure\n");
        return;
    }

    // Clear out the DFU image
    if (J *req = notecard.newRequest("dfu.status")) {
        JAddBoolToObject(req, "stop", true);
        notecard.sendRequest(req);
    }

    // Restart
    notecard.logDebugf("dfu: restart system\n");
    esp_restart();
}
