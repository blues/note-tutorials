#include <Arduino.h>
#include <Wire.h>
#include <Notecard.h>

#include "person_sensor.h"

#define serialDebugOut Serial
// Uncomment to view Note requests from the Host
// #define DEBUG_NOTECARD

// How long to wait between reading the sensor. The sensor can be read as
// frequently as you like, but the results only change at about 5FPS, so
// waiting for 200ms is reasonable.
const int32_t SAMPLE_DELAY_MS = 200;

// We look for a single unrecognized face visible for at least five frames
// to add new recognition IDs. The calibration process takes about four
// seconds.
const int unrecognized_threshold = 5;
int unrecognized_frame_count = 0;
int next_unused_id = 1;
const int calibration_threshold = 20;
int calibration_frame_count = 0;

Notecard notecard;

void setup() {
  // You need to make sure you call Wire.begin() in setup, or the I2C access
  // below will fail.
  Wire.begin();
  Serial.begin(115200);

#ifdef DEBUG_NOTECARD
  notecard.setDebugOutputStream(serialDebugOut);
#endif

  notecard.begin();

  J *req = notecard.newRequest("hub.set");
  if (req != NULL) {
    JAddStringToObject(req, "mode", "continuous");
    JAddBoolToObject(req, "sync", true);
    notecard.sendRequest(req);
  }
}

void loop() {
  person_sensor_results_t results = {};
  // Perform a read action on the I2C address of the sensor to get the
  // current face information detected.
  if (!person_sensor_read(&results)) {
    Serial.println("No person sensor results found on the i2c bus");
    delay(SAMPLE_DELAY_MS);
    return;
  }

  Serial.println("********");
  Serial.print(results.num_faces);
  Serial.println(" faces found");

  for (int i = 0; i < results.num_faces; ++i) {
    const person_sensor_face_t* face = &results.faces[i];

    if (face->id_confidence > 0) {
        Serial.printf("Recognized face %d as person %d with confidence %d\n", 
            i, face->id, face->id_confidence);
    } else {
        Serial.printf("Unrecognized face %d\n", i);
    }

    if (face->box_confidence > 90) {
      J *req = notecard.newRequest("note.add");
      if (req != NULL)
      {
        JAddStringToObject(req, "file", "face.qo");
        JAddBoolToObject(req, "sync", true);
        J *body = JCreateObject();
        if (body != NULL)
        {
          JAddNumberToObject(body, "face_confidence", face->box_confidence);
          if (face->id_confidence > 0) {
            JAddNumberToObject(body, "id_confidence", face->id_confidence);
            JAddNumberToObject(body, "id", face->id);
          }
          JAddBoolToObject(body, "facing_camera", face->is_facing);

          J *box_body = JCreateObject();
          if (body != NULL)
          {
            JAddNumberToObject(box_body, "left", face->box_left);
            JAddNumberToObject(box_body, "right", face->box_right);
            JAddNumberToObject(box_body, "top", face->box_top);
            JAddNumberToObject(box_body, "bottom", face->box_bottom);

            JAddItemToObject(body, "box_body", box_body);
          }

          JAddItemToObject(req, "body", body);
          notecard.sendRequest(req);
        }
      }
    }

    /*
    Serial.print("Face #");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(face->box_confidence);
    Serial.print(" confidence, (");
    Serial.print(face->box_left);
    Serial.print(", ");
    Serial.print(face->box_top);
    Serial.print("), (");
    Serial.print(face->box_right);
    Serial.print(", ");
    Serial.print(face->box_bottom);
    Serial.print("), ");
    if (face->is_facing) {
      Serial.println("facing");
    } else {
      Serial.println("not facing");
    }
    */
  }

  // Calibration Process for face recognition
  if (calibration_frame_count > 0) {
      // Let the sensor handle calibration for a few seconds if it has
      // been started.
      calibration_frame_count -= 1;
      if (calibration_frame_count == 0) {
          Serial.println("Done calibrating\n");
      }
  } else if ((results.num_faces == 1) &&
      (results.faces[0].box_confidence >= 95) &&
      (results.faces[0].id_confidence <= 90) &&
      (results.faces[0].is_facing == 1) &&
      (next_unused_id < PERSON_SENSOR_MAX_IDS_COUNT))
  {
      // If we have an unrecognized face for a long enough time, start
      // the calibration process.
      unrecognized_frame_count += 1;
      if (unrecognized_frame_count >= unrecognized_threshold) {
          Serial.println("Calibrating\n");
          person_sensor_write_reg(
              PERSON_SENSOR_REG_CALIBRATE_ID, next_unused_id);
          calibration_frame_count = calibration_threshold;
          next_unused_id += 1;
      }
  } else {
      unrecognized_frame_count = 0;
  }

  delay(SAMPLE_DELAY_MS);
}
