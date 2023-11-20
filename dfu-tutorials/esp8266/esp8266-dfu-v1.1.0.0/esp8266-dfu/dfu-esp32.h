//
// Copyright 2023 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.
#pragma once

#if defined(ARDUINO_ARCH_ESP32)

#include <Arduino.h>
#include <Notecard.h>
#include "esp_partition.h"
#include "esp_system.h"
#include "esp_ota_ops.h"
#include "esp_flash_partitions.h"
#include "notecard-host-dfu.h"


class ArduinoESP32DFUUpdater {

    esp_ota_handle_t update_handle;
    esp_err_t err;
    const esp_partition_t *update_partition;

public:
    bool setup() {
        const esp_partition_t* partition = nullptr;

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
        return true;
    }

    bool begin(size_t imageSize) {
        err = ESP_OK;
        update_handle = 0;
        update_partition = nullptr;
        const esp_partition_t *configured = esp_ota_get_boot_partition();
        const esp_partition_t *running = esp_ota_get_running_partition();
        if (configured != running) {
            notecard.logDebugf("dfu: configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x\n",
                    configured->address, running->address);
            notecard.logDebugf("     (This can happen if either the OTA boot data or preferred boot image become corrupted.)\n");
        }
        notecard.logDebugf("dfu: running partition type %d subtype %d (offset 0x%08x)\n", running->type, running->subtype, running->address);

        update_partition = esp_ota_get_next_update_partition(NULL);
        if (update_partition == NULL) { // simply being defensive
            return false;
        }  
            
        notecard.logDebugf("dfu: writing to partition subtype %d at offset 0x%x\n", update_partition->subtype, update_partition->address);

        // Begin the update
        esp_err_t err = esp_ota_begin(update_partition, imageSize, &update_handle);
        if (err != ESP_OK) {
            notecard.logDebugf("esp_ota_begin failed (%s)\n", esp_err_to_name(err));
            return false;
        }
        return true;
    }

    bool write(const uint8_t* buffer, size_t length) {
        err = esp_ota_write(update_handle, (const void *)buffer, length);
        return err==ESP_OK;
    }

    bool end() {// Set the boot partition and reboot
        err = esp_ota_set_boot_partition(update_partition);
        if (err != ESP_OK) {
            notecard.logDebugf("dfu: restart failure\n");
            return false;
        }
        return true;
    }

    bool hasError() {
        return err!=ESP_OK;
    }

    String getError() {
        const char* err_cstr = esp_err_to_name(err);
        return String(err_cstr);
    }

    bool executeImage() {
        esp_restart();
        return true;
    }
};

typedef ArduinoESP32DFUUpdater DFUUpdater;

#endif