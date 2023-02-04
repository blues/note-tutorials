// Copyright 2019-2023 Blues Inc.  All rights reserved.
// Use of this source code is governed by licenses granted by the
// copyright holder including that found in the LICENSE file.

// Device Firmware Update support.  (Note that this is only currently supported on ESP32.)
// As a note to the reader: sometimes firmware update is referred to as "OTA" (Over The Air) or
// "FOTA" (Firmware Over The Air).  Technically, this code is reading the already-downloaded and
// already-verified firmware from the Notecard's storage in a fully-offline manner, which is why
// we use the more generic DFU (Device Firmware Update) term of art.
#include <chrono>
#include <Arduino.h>
#include <Notecard.h>
#include "notecard-host-dfu.h"
#include "notecard-cpp.h"
#include "TicksTimer.h"
#include "dfu-updater.h"

/**
 * @brief Implements host firmware updates.
 *
 */
class DFU
{

  enum class State
  {
    /**
     * @brief In-application DFU updates are not available.
     */
    UNAVAILABLE,

    /**
     * @brief The initial state.
     */
    INITIAL,

    /**
     * @brief Report the current version to Notehub.
     * Transitions to CHECK_DFU_READY once the current firmware version has been sent to Notehub.
     */
    REPORT_CURRENT_VERSION,

    /**
     * @brief Query the notecard for an available DFU image.
     */
    CHECK_DFU_AVAILABLE,

    /**
     * @brief DFU image is available. Send a notification to the app.
     * Sends a notification to the app, or automatically transitions to REQUEST_DFU_MODE.
     */
    DFU_AVAILABLE,

    /**
     * @brief Request that the Notecard enters DFU mode.
     */
    REQUEST_DFU_MODE,

    /**
     * @brief Check that the Notecard has entered DFU mode.
     */
    CHECK_DFU_MODE,

    /**
     * @brief Prepare the device to receive the firmware image.
     */
    BEGIN_UPDATE,

    /**
     * @brief Retrieve a chunk of the firmware image and check integrity.
     */
    RETRIEVE_CHUNK,

    /**
     * @brief Store the retrieved chunk to the host's memory.
     */
    STORE_CHUNK,

    /**
     * @brief When all chunks have been received
     */
    FINALIZE_TRANSFER,

    /**
     * @brief Instructs the Notecard to leave DFU mode and resume the previous mode.
     */
    LEAVE_DFU_MODE,

    /**
     * @brief Remove the DFU image stored on the Notecard.
     */
    DELETE_DFU_IMAGE,

    /**
     * @brief Causes the downloaded image to be executed.
     */
    EXECUTE_IMAGE
  };

  /**
   * @brief The current state the DFU process is in.
   */
  State state = State::INITIAL;

  /**
   * @brief Set to true when a new state transition occurs. This is used to skip the delay interval on first
   * transitioning to the state.
   */
  bool newState;

  /**
   * @brief How many retries. This is set when transitioning to the state.
   */
  unsigned stateRetries;

  /**
   * @brief How many retries remain for the current state. When 0 and `stateRetries` is non-zero
   * this is considered a fatal error and the DFU update is cancelled.
   */
  unsigned retries;

  /**
   * @brief When to timeout for the given state. When duration is 0, a timeout not enforced.
   */
  ArduinoTicksTimer retryTimeout;

  /**
   * @brief How long to wait until dfuPoll takes the next step.
   */
  ArduinoTicksTimer next;

  /**
   * @brief Indicates when the Notecard has received a DFU image.
   */
  bool imageIsReady;

  /**
   * @brief Indicates if the image is the same version as presently on the host.
   */
  bool imageIsSameAsCurrent;

  /**
   * @brief The expected MD5 of the entire image.
   */
  char imageMD5[NOTE_MD5_HASH_STRING_SIZE];

  /**
   * @brief The size of the firmware image in bytes.
   */
  uint32_t imageLength;

  /**
   * @brief The offset of the next chunk to retrieve.
   */
  uint32_t offset;

  /**
   * @brief The maximum chunk size to retrieve from the notecard.
   */
  const uint32_t chunkLength = 4096;

  /**
   * @brief Accumulated MD5 checksum for the whole image.
   * Individual chunks are integrity-checked using CRC32.
   */
  NoteMD5Context md5Context;

  /** The current chunk retrieved from the Notecard. Allocated in RETRIEVE_CHUNK and released in STORE_CHUNK */
  scoped_malloc_t currentChunk = scoped_malloc();

  /** The length of the chunk in `currentChunk` */
  uint32_t currentChunkLength;

  /**
   * @brief true when the image has been successfully transferred and validated.
   */
  bool imageTransferred;

  /**
   * @brief true when the DFU image should be deleted.
   */
  bool discardDFUImage;

  /**
   * @brief Implementation of the DFUUpdater to use
   */
  DFUUpdater updater;

  unsigned waitForDFUModeTimeout;
  unsigned requestInterval;
  unsigned checkDFUAvailableInterval;
  unsigned requestAttempts;
public:
  DFU() {}

  bool setup(const DFUConfig& config) {
    waitForDFUModeTimeout = config.waitForDFUTimeout.count();
    requestInterval = config.requestInterval.count();
    checkDFUAvailableInterval = config.checkDFUAvailableInterval.count();
    requestAttempts = config.requestAttempts;

    bool ok = updater.setup();
    if (!ok) {
      transitionTo(State::UNAVAILABLE);
    }
    return ok;
  }

  uint32_t poll(bool force)
  {
    int32_t remaining = next.remaining();
    if (!newState && !force && remaining > 0)
    {
      return remaining;
    }
    newState = false;
    notecard.logDebugf("running state %d\n", state);
    runState();

    remaining = next.remaining();
    if (remaining < 0)
    {
      remaining = 0;
    }
    notecard.logDebugf("poll %d\n", remaining);
    return uint32_t(remaining);
  }

private:
  /**
   * @brief Retries the current state, or transitions to the given state when the number of retries or timeout has been reached.
   *
   * @param state
   */
  bool retry(State state)
  {
    bool retryFail = false;

    if (retryTimeout.duration && retryTimeout.hasElapsed())
    {
      notecard.logDebugf("timeout");
      retryFail = true;
    }
    else if (stateRetries > 0)
    {
      retries += 1;
      if (retries >= stateRetries)
      {
        retryFail = true;
      }
    }

    if (retryFail)
    {
      transitionTo(state);
    }
    else
    {
      next.set(next.duration);
    }

    return retryFail;
  }

  /**
   * @brief Transition to a new state. Transient errors result in retries, fatal errors
   *
   * @param state     The target state to transition to.
   * @param retries   The number of retries. When 0, the state is not left unless it succeeds or duration has elapsed (if specified.)
   * @param timeout   How long to remain in this state. When 0 then the state remains until it succeeds or the specified number of retries have been attempted.
   */
  void _transitionTo(State state, unsigned retries, unsigned retryInterval, unsigned timeout)
  {
    this->state = state;
    this->stateRetries = retries;
    this->retries = 0;
    this->next.set(retryInterval);
    this->retryTimeout.set(timeout);
  }

  /**
   * @brief Transition to a state with a number of retries, retry interval and timeout applicable to that state
   *
   * @param state The state to transition to.
   */
  void transitionTo(State state)
  {
    newState = (state!=this->state);

    switch (state)
    {
    case State::UNAVAILABLE:
      // stay in this state, with a long timeout between each poll
      _transitionTo(state, 0, 24*60*60*1000 /* 1 day */, 0);
      break;

    case State::CHECK_DFU_AVAILABLE:
      _transitionTo(state, 0, checkDFUAvailableInterval, 0);
      break;

    case State::CHECK_DFU_MODE:
      _transitionTo(state, 0, 2500, waitForDFUModeTimeout);
      break;

    // the remaining states attempt a retry
    case State::INITIAL:
    case State::REPORT_CURRENT_VERSION:
    case State::DFU_AVAILABLE:
    case State::REQUEST_DFU_MODE:
    case State::BEGIN_UPDATE:
    case State::RETRIEVE_CHUNK:
    case State::STORE_CHUNK:
    case State::FINALIZE_TRANSFER:
    case State::LEAVE_DFU_MODE:
    case State::DELETE_DFU_IMAGE:
    case State::EXECUTE_IMAGE:
      _transitionTo(state, requestAttempts, requestInterval, 0);
      break;
    }
  }

  void runState()
  {
    switch (state)
    {
    case State::INITIAL:
      transitionTo(State::REPORT_CURRENT_VERSION);
      break;

    case State::REPORT_CURRENT_VERSION:
      if (reportFirmwareVersion())
      {
        // no limit on retries or timeout
        transitionTo(State::CHECK_DFU_AVAILABLE);
      }
      else
      {
        retry(State::REPORT_CURRENT_VERSION);
      }
      break;

    case State::CHECK_DFU_AVAILABLE:
      if (checkDFUReady() && isUpdateAvailable())
      {
        transitionTo(State::DFU_AVAILABLE);
      }
      else
      {
        retry(State::CHECK_DFU_AVAILABLE);
      }
      break;

    case State::DFU_AVAILABLE:
      // todo - notify the app that DFU is available.
      transitionTo(State::REQUEST_DFU_MODE);
      break;

    case State::REQUEST_DFU_MODE:
      if (requestDFUMode())
      {
        transitionTo(State::CHECK_DFU_MODE);
      }
      else
      {
        retry(State::CHECK_DFU_AVAILABLE);
      }
      break;
      
    case State::CHECK_DFU_MODE:
      if (checkDFUMode())
      {
        transitionTo(State::BEGIN_UPDATE);
      }
      else
      {
        retry(State::CHECK_DFU_AVAILABLE);
      }
      break;

    case State::BEGIN_UPDATE:
      imageTransferred = false;
      discardDFUImage = false;
      if (beginUpdate())
      {
        transitionTo(State::RETRIEVE_CHUNK);
      }
      else
      { // typically because the app is too large to fit into memory.
        discardDFUImage = true;
        transitionTo(State::LEAVE_DFU_MODE);
      }
      break;

    case State::RETRIEVE_CHUNK:
      if (retrieveChunk())
      {
        transitionTo(State::STORE_CHUNK);
      }
      else
      {
        retry(State::LEAVE_DFU_MODE);
      }
      break;

    case State::STORE_CHUNK:
      if (storeChunk())
      {
        if (isUpdateComplete())
        {
          transitionTo(State::FINALIZE_TRANSFER);
        }
        else
        { // fetch the next chunk
          transitionTo(State::RETRIEVE_CHUNK);
        }
      }
      else
      {
        transitionTo(State::LEAVE_DFU_MODE);
      }
      break;

    case State::FINALIZE_TRANSFER:
      if (validateImage() && activateImage())
      {
        imageTransferred = true;
        discardDFUImage = true;
      }
      transitionTo(State::LEAVE_DFU_MODE);
      break;

    case State::LEAVE_DFU_MODE:
      if (exitDFUMode())
      {
        transitionTo(imageTransferred || discardDFUImage ? State::DELETE_DFU_IMAGE : State::CHECK_DFU_AVAILABLE);
      }
      else
      {
        retry(State::LEAVE_DFU_MODE);
      }
      break;

    case State::DELETE_DFU_IMAGE:
      if (!discardDFUImage || this->deleteDFUImage())
      {
        transitionTo(imageTransferred ? State::EXECUTE_IMAGE : State::CHECK_DFU_AVAILABLE);
      }
      else
      {
        retry(State::DELETE_DFU_IMAGE);
      }
      break;

    case State::EXECUTE_IMAGE:
      executeImage();
      transitionTo(State::CHECK_DFU_AVAILABLE);
      break;

    case State::UNAVAILABLE:
      transitionTo(state);
      break;
    }

  }

  /**
   * @brief Report the current firmware version to Notehub.
   *
   * @return true On success.
   * @return false On failure.
   */
  bool reportFirmwareVersion()
  {
    // register the current firmware version with notehub
    J *req = notecard.newRequest("dfu.status");
    if (req != NULL)
    {
      JAddStringToObject(req, "version", firmwareVersion());
      notecard.sendRequest(req);
    }
    return req;
  }

  /**
   * @brief Determines whether the Notecard has a DFU image available.
   *
   * @return true When a DFU image is ready to apply to the host.
   * @return false When no image is available or an error occurs.
   */
  bool checkDFUReady()
  {
    imageIsReady = false;
    imageLength = 0;
    imageIsSameAsCurrent = true;

    // Check status, and determine both if there is an image ready, and if the image is NEW.
    J *rsp = notecard.requestAndResponse(notecard.newRequest("dfu.status"));
    if (rsp && strcmp(JGetString(rsp, "mode"), "ready") == 0)
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
      else
      {
        notecard.logDebugf("dfu: no 'body' element in response\n");
      }
      notecard.deleteResponse(rsp);
    }
    return rsp;   // signal that the request was sent
  }

  bool isUpdateAvailable() {
        // Exit if same version or no DFU to process
    bool ready = imageIsReady && !imageIsSameAsCurrent && imageLength != 0;
    if (!ready)
    {
      notecard.logDebugf("dfu: no image is ready for firmware update: (%d, %d, %d)\n", imageIsReady, imageIsSameAsCurrent, imageLength);
    }
    return ready;
  }

  /**
   * @brief Requests that the Notecard enters DFU mode.
   *
   * @return true   The request was successfully sent.
   * @return false  An error occurred.
   */
  bool requestDFUMode()
  {
    notecard.logDebugf("dfu: entering DFU mode\n");
    // Enter DFU mode.  Note that the Notecard will automatically switch us back out of
    // DFU mode after 15m, so we don't leave the notecard in a bad state if we had a problem here.
    J *req = notecard.newRequest("hub.set");
    if (req)
    {
      JAddStringToObject(req, "mode", "dfu");
      if (!notecard.sendRequest(req))
      {
        req = nullptr;
      }
    }
    return req;
  }

  /**
   * @brief Checks that the Notecard has entered DFU mode.
   *
   * @return true The Notecard has entered DFU mode.
   * @return false The notecard has not yet entered DFU mode or an error occurs.
   */
  bool checkDFUMode()
  {
    bool inDFUMode = false;
    J *rsp = notecard.requestAndResponse(notecard.newRequest("dfu.get"));
    if (rsp && !notecard.responseError(rsp))
      inDFUMode = true;
    notecard.deleteResponse(rsp);
    return inDFUMode;
  }

  /**
   * @brief Requests that the notecard exits DFU mode and returns to the previously set mode.
   *
   * @return true The request was successfully sent.
   * @return false An error occurred.
   */
  bool exitDFUMode()
  {
    J *req = notecard.newRequest("hub.set");
    if (req)
    {
      JAddStringToObject(req, "mode", "dfu-completed");
      if (!notecard.sendRequest(req))
      {
        req = nullptr;
      }
    }
    return req;
  }

  /**
   * @brief
   *
   * @return true The host is ready to receive the update.
   * @return false The host is unable to receive the update. This is a fatal error.
   */
  bool beginUpdate()
  {
    if (!updater.begin(imageLength))
    {
      notecard.logDebug("dfu: Update.begin failed.\n");
      return false;
    }
    notecard.logDebugf("dfu: writing image length %d to flash\n", imageLength);

    notecard.logDebugf("dfu: beginning firmware update\n");

    offset = 0;
    NoteMD5Init(&md5Context);
    return true;
  }

  bool isUpdateComplete()
  {
    return imageLength && offset == imageLength;
  }

  /**
   * @brief Retrieve a chunk of the firmware image from the Notecard.
   *
   * @return true The chunk was successfully retrieved. The
   * @return false
   */
  bool retrieveChunk()
  {
    uint32_t left = imageLength - offset;
    uint32_t thislen = left > chunkLength ? chunkLength : left;
    notecard.logDebugf("dfu: reading chunk (offset:%d length:%d try:%d)\n", offset, thislen, retries + 1);
    currentChunk.release();
    currentChunkLength = 0;

    // Request the next chunk from the notecard
    J *req = notecard.newRequest("dfu.get");
    if (req == NULL)
    {
      notecard.logDebugf("dfu: insufficient memory\n");
      return false;
    }
    JAddNumberToObject(req, "offset", offset);
    JAddNumberToObject(req, "length", thislen);
    auto rsp = scoped_response(notecard.requestAndResponse(req));
    if (!rsp)
    {
      notecard.logDebugf("dfu: insufficient memory\n");
      return false;
    }
    if (notecard.responseError(rsp))
    {
      notecard.logDebugf("dfu: error on read: %s\n", JGetString(rsp, "err"));
      return false;
    }
    char *payloadB64 = JGetString(rsp, "payload");
    if (payloadB64[0] == '\0')
    {
      notecard.logDebugf("dfu: no payload\n");
      return false;
    }
    auto payload = scoped_malloc(JB64DecodeLen(payloadB64));
    if (!payload)
    {
      notecard.logDebugf("dfu: can't allocate payload decode buffer\n");
      return false;
    }

    uint32_t actuallen = uint32_t(JB64Decode(payload, payloadB64));
    const char *expectedMD5 = JGetString(rsp, "status");
    char chunkMD5[NOTE_MD5_HASH_STRING_SIZE] = {0};
    NoteMD5HashString(payload, actuallen, chunkMD5, sizeof(chunkMD5));

    if (thislen != actuallen)
    {
      notecard.logDebugf("dfu: decoded data not the correct length (%d != actual %d)\n", thislen, actuallen);
      return false;
    }
    else if (strcmp(chunkMD5, expectedMD5))
    {
      notecard.logDebugf("dfu: %d-byte decoded data MD5 mismatch (%s != actual %s)\n", actuallen, expectedMD5, chunkMD5);
      return false;
    }
    currentChunk = std::move(payload);
    currentChunkLength = thislen;
    return true;
  }

  bool storeChunk()
  {
    auto payload = std::move(currentChunk); // transfer ownership so the mem
    uint32_t thislen = currentChunkLength;
    currentChunkLength = 0;

    // MD5 the chunk
    NoteMD5Update(&md5Context, payload, thislen);

    // Write the chunk
    updater.write(payload, thislen);
    if (updater.hasError())
    {
      return false;
    }

    notecard.logDebugf("dfu: successfully transferred offset:%d len:%d\n", offset, thislen);
    offset += thislen;
    return true;
  }

  bool validateImage()
  {
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
      return false;
    }
    return true;
  }

  /**
   * @brief Clears the DFU image from the Notecard.
   *
   * @return true   When the request was successfully sent.
   * @return false  When the request could not be sent.
   */
  bool deleteDFUImage()
  {
    J *req = notecard.newRequest("dfu.status");
    JAddBoolToObject(req, "stop", true);
    return notecard.sendRequest(req);
  }

  /**
   * @brief Mark the image as active next time the device boots.
   *
   * @return true
   * @return false
   */
  bool activateImage()
  {
    if (!updater.end())
    {
      notecard.logDebugf("update.end() failed!\n");
      return false;
    }
    return true;
  }

  bool executeImage()
  {
    notecard.logDebugf("dfu: restart system\n");
    updater.executeImage();
    return true;
  }
};



/**
 * @brief Use a function to access the global DFU instance. When DFU is not used, it is removed from the executable by the linker.
 *
 * @return DFU& The singleton DFU instance.
 */
DFU &dfu()
{
  static DFU dfu;
  return dfu;
}

bool dfuSetup(const DFUConfig& config)
{
  return dfu().setup(config);
}

uint32_t dfuPoll(bool force)
{
  return dfu().poll(force);
}
