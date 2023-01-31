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
#include "Updater.h"



void dfuSetup() {

  J* req = notecard.newRequest("dfu.status");
  if (req != NULL) {
      JAddStringToObject(req, "version", firmwareVersion());
      notecard.sendRequest(req);
  }


  Update.onError([](uint8_t error) {
    notecard.logDebug(Update.getErrorString().c_str());
  });

  Update.onProgress([](size_t complete, size_t total) {
    notecard.logDebugf("DFU progress: %d/%d (%d%%)", complete, total, (complete/total)*100);
  });
}

// Process DFU
void dfuPoll(bool force)
{
  static uint32_t dfuCheckMs = 0;
  static uint32_t serviceIdleCheckMs = 0;

  // todo - why two tests?

  // Suppress how often we check
  if (!force && dfuCheckMs != 0 && millis() < dfuCheckMs + ms1Min)
    return;

  // Even if DFU is ready, only check the notehub status once every 10s max
  if (!force && serviceIdleCheckMs != 0 && millis() < serviceIdleCheckMs + 10 * ms1Sec)
    return;
  serviceIdleCheckMs = millis();

  // Check status, and determine both if there is an image ready, and if the image is NEW.
  bool imageIsReady = false;
  bool imageIsSameAsCurrent = false;
  char imageMD5[NOTE_MD5_HASH_STRING_SIZE] = {0};
  uint32_t imageLength = 0;
  if (J *rsp = notecard.requestAndResponse(notecard.newRequest("dfu.status")))
  {
    if (strcmp(JGetString(rsp, "mode"), "ready") == 0)
    {
      notecard.logDebug("dfu: image is ready.\n");
      imageIsReady = true;
      if (J *body = JGetObjectItem(rsp, "body"))
      {
        imageLength = JGetInt(body, "length");
        strlcpy(imageMD5, JGetString(body, "md5"), sizeof(imageMD5));
        imageIsSameAsCurrent = strcmp(JGetString(body, "version"), firmwareVersion()) == 0;
        notecard.logDebugf("dfu: image length %d\n", imageLength);
        if (!imageIsSameAsCurrent)
        {
          notecard.logDebugf("dfu: replacing current image: %s\n", productVersion());
          notecard.logDebugf("dfu:   with downloaded image: %s\n", JGetString(body, "name"));
        }
      }
      else {
        notecard.logDebugf("dfu; no 'body' element in response\n");
      }
    }
    notecard.deleteResponse(rsp);
  }

  // Exit if same version or no DFU to process
  if (!imageIsReady || imageIsSameAsCurrent || imageLength == 0)
  {
    dfuCheckMs = millis();
    notecard.logDebugf("dfu: no image is ready for firmware update: (%d, %d, %d)\n", imageIsReady, imageIsSameAsCurrent, imageLength);
    return;
  }

  notecard.logDebugf("dfu: entering DFU mode\n");
  // Enter DFU mode.  Note that the Notecard will automatically switch us back out of
  // DFU mode after 15m, so we don't leave the notecard in a bad state if we had a problem here.
  if (J *req = notecard.newRequest("hub.set"))
  {
    JAddStringToObject(req, "mode", "dfu");
    notecard.sendRequest(req);
  }

  // Proceed with DFU
  dfuCheckMs = millis();

  // Wait until we have successfully entered the mode.  The fact that this loop isn't
  // just an infinite loop is simply defensive programming.  If for some odd reason
  // we don't enter DFU mode, we'll eventually come back here on the next DFU poll.
  bool inDFUMode = false;
  int beganDFUModeCheck = millis();
  while (!inDFUMode && millis() < beganDFUModeCheck + (2 * ms1Min))
  {
    if (J *rsp = notecard.requestAndResponse(notecard.newRequest("dfu.get")))
    {
      if (!notecard.responseError(rsp))
        inDFUMode = true;
      notecard.deleteResponse(rsp);
    }
    if (!inDFUMode) {
      delay(2500);
      notecard.logDebug("dfu: waiting to enter DFU mode\n");
    }
  }

  // If we failed, leave DFU mode immediately
  if (!inDFUMode)
  {
    notecard.logDebug("dfu: timeout waiting for dfu mode.\n");
    if (J *req = notecard.newRequest("hub.set"))
    {
      JAddStringToObject(req, "mode", "dfu-completed");
      notecard.sendRequest(req);
    }
    return;
  }

  // The image is ready.  If the version is the same as what's in memory, then of course don't

  if (!Update.begin(imageLength, U_FLASH, LED_BUILTIN)) {
    notecard.logDebug("dfu: Update.begin failed.\n");
    return;
  }
  notecard.logDebugf("dfu: writing image length %d to flash\n", imageLength);

  notecard.logDebugf("dfu: beginning firmware update\n");

  // Loop over received chunks
  int offset = 0;
  int chunklen = 4096;
  int left = imageLength;
  NoteMD5Context md5Context;
  NoteMD5Init(&md5Context);
  while (left)
  {
    yield();
    // Read next chunk from card
    int thislen = chunklen;
    if (left < thislen)
      thislen = left;

    // If anywhere, this is the location of the highest probability of I/O error
    // on the I2C or serial bus, simply because of the amount of data being transferred.
    // As such, it's a conservative measure just to retry.
    uint8_t *payload = NULL;
    for (int retry = 0; retry < 5; retry++)
    {
      yield();
      notecard.logDebugf("dfu: reading chunk (offset:%d length:%d try:%d)\n", offset, thislen, retry + 1);

      // Request the next chunk from the notecard
      J *req = notecard.newRequest("dfu.get");
      if (req == NULL)
      {
        notecard.logDebugf("dfu: insufficient memory\n");
        return;
      }
      JAddNumberToObject(req, "offset", offset);
      JAddNumberToObject(req, "length", thislen);
      J *rsp = notecard.requestAndResponse(req);
      if (rsp == NULL)
      {
        notecard.logDebugf("dfu: insufficient memory\n");
        return;
      }
      if (notecard.responseError(rsp))
      {
        notecard.logDebugf("dfu: error on read: %s\n", JGetString(rsp, "err"));
      }
      else
      {
        char *payloadB64 = JGetString(rsp, "payload");
        if (payloadB64[0] == '\0')
        {
          notecard.logDebugf("dfu: no payload\n");
          notecard.deleteResponse(rsp);
          return;
        }
        // todo - use std::unique_ptr<int[]> p(new (nothrow) uint8_t[JB64DecodeLen(payloadB64)]);
        payload = (uint8_t*)malloc(JB64DecodeLen(payloadB64));
        if (payload == NULL)
        {
          notecard.logDebugf("dfu: can't allocate payload decode buffer\n");
          notecard.deleteResponse(rsp);
          return;
        }
        int actuallen = JB64Decode((char*)payload, payloadB64);
        const char *expectedMD5 = JGetString(rsp, "status");
        ;
        char chunkMD5[NOTE_MD5_HASH_STRING_SIZE] = {0};
        NoteMD5HashString((uint8_t *)payload, actuallen, chunkMD5, sizeof(chunkMD5));
        if (actuallen == thislen && strcmp(chunkMD5, expectedMD5) == 0)
        {
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
    if (payload == NULL)
    {
      notecard.logDebugf("dfu: unrecoverable error on read\n");
      return;
    }

    // MD5 the chunk
    NoteMD5Update(&md5Context, (uint8_t *)payload, thislen);

    // Write the chunk
    Update.write(payload, thislen);

    // Move to next chunk
    free(payload);

    if (Update.hasError()) {
      return;
    }

    notecard.logDebugf("dfu: successfully transferred offset:%d len:%d\n", offset, thislen);
    offset += thislen;
    left -= thislen;
  }

  // Exit DFU mode.  (Had we not done this, the Notecard exits DFU mode automatically after 15m.)
  if (J *req = notecard.newRequest("hub.set"))
  {
    JAddStringToObject(req, "mode", "dfu-completed");
    notecard.sendRequest(req);
  }

  // Done
  if (!Update.end()) {
    notecard.logDebugf("Update.end() failed!\n");
    return;
  }

  // Validate the MD5
  uint8_t md5Hash[NOTE_MD5_HASH_SIZE];
  NoteMD5Final(md5Hash, &md5Context);
  char md5HashString[NOTE_MD5_HASH_STRING_SIZE];
  NoteMD5HashToString(md5Hash, md5HashString, sizeof(md5HashString));
  notecard.logDebugf("dfu:    MD5 of image: %s\n", imageMD5);
  notecard.logDebugf("dfu: MD5 of download: %s\n", md5HashString);
  if (strcmp(imageMD5, md5HashString) != 0)
  {
    notecard.logDebugf("MD5 MISMATCH - ABANDONING DFU\n");
    return;
  }

  // Clear out the DFU image
  if (J *req = notecard.newRequest("dfu.status"))
  {
    JAddBoolToObject(req, "stop", true);
    notecard.sendRequest(req);
  }

  // Restart
  notecard.logDebugf("dfu: restart system\n");
  ESP.restart();  // cleaner than reset()
}