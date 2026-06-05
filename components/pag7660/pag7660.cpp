#include "pag7660.h"

#include <cstdio>

#include "esphome/core/log.h"

namespace esphome {
namespace pag7660 {

static const char *const TAG = "pag7660.text_sensor";

static constexpr uint8_t REG_ID_L = 0x00;
static constexpr uint8_t REG_ID_H = 0x01;
static constexpr uint8_t REG_FRAME_READY = 0x04;
static constexpr uint8_t REG_ENABLE_CPU = 0x0A;
static constexpr uint8_t REG_OPERATION_MODE = 0x10;
static constexpr uint8_t REG_GESTURE_MODE = 0x22;
static constexpr uint8_t REG_OUTPUT_START = 0x3C;

// Matches Seeed's pag7660_reg_out_t packed size.
static constexpr size_t OUTPUT_FRAME_SIZE = 38;

void PAG7660TextSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up PAG7660 text sensor...");

  uint8_t id_l = 0;
  uint8_t id_h = 0;
  if (!this->read_byte(REG_ID_L, &id_l) || !this->read_byte(REG_ID_H, &id_h)) {
    ESP_LOGE(TAG, "Unable to read PAG7660 chip ID");
    this->mark_failed();
    return;
  }

  const uint16_t chip_id = static_cast<uint16_t>(id_h << 8) | id_l;
  if (chip_id != 0x7660) {
    ESP_LOGE(TAG, "Unexpected PAG7660 chip ID 0x%04X", chip_id);
    this->mark_failed();
    return;
  }

  // Seeed init sequence from Gesture.cpp
  if (!this->write_byte(REG_OPERATION_MODE, 0x04) ||
      !this->write_byte(REG_GESTURE_MODE, this->gesture_mode_) ||
      !this->write_byte(REG_ENABLE_CPU, 0x01)) {
    ESP_LOGE(TAG, "Failed to write PAG7660 init registers");
    this->mark_failed();
    return;
  }

  delay(250);
  this->status_clear_warning();
}

void PAG7660TextSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "PAG7660 Text Sensor:");
  LOG_TEXT_SENSOR("  ", "Gesture", this);
  LOG_UPDATE_INTERVAL(this);
  LOG_I2C_DEVICE(this);
  ESP_LOGCONFIG(TAG, "  Gesture mode: %u", this->gesture_mode_);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication failed during setup");
  }
}

void PAG7660TextSensor::update() {
  uint8_t frame_ready = 0;
  if (!this->read_byte(REG_FRAME_READY, &frame_ready)) {
    ESP_LOGW(TAG, "Failed reading frame-ready register");
    this->status_set_warning();
    return;
  }

  if ((frame_ready & 0x02U) == 0) {
    return;
  }

  uint8_t frame[OUTPUT_FRAME_SIZE] = {0};
  if (!this->read_bytes(REG_OUTPUT_START, frame, OUTPUT_FRAME_SIZE)) {
    ESP_LOGW(TAG, "Failed reading output frame");
    this->status_set_warning();
    return;
  }

  if (!this->clear_frame_ready_()) {
    ESP_LOGW(TAG, "Failed clearing frame-ready flag");
    this->status_set_warning();
    return;
  }

  this->status_clear_warning();
  const std::string decoded = this->decode_gesture_(frame, OUTPUT_FRAME_SIZE);
  if (decoded.empty() || decoded == this->last_state_) {
    return;
  }

  this->last_state_ = decoded;
  this->publish_state(decoded);
}

bool PAG7660TextSensor::clear_frame_ready_() { return this->write_byte(REG_FRAME_READY, 0x00); }

std::string PAG7660TextSensor::decode_gesture_(const uint8_t *frame, size_t frame_len) const {
  if (frame == nullptr || frame_len < OUTPUT_FRAME_SIZE) {
    return "";
  }

  // Layout follows Seeed's pag7660_reg_out_t in pag7660.h
  const uint8_t result_0 = frame[27];
  const uint8_t result_1 = frame[28];
  const uint8_t thumb_down_raw = frame[31];
  const int16_t rotate = static_cast<int16_t>(static_cast<uint16_t>(frame[33]) << 8 | frame[32]);

  const uint8_t gesture_type = (result_0 >> 3) & 0x1F;
  const uint8_t cursor_type = (result_0 >> 1) & 0x03;
  const bool thumb_up = (result_0 & 0x01) != 0;
  const bool cursor_select = (result_1 & 0x01) != 0;
  const bool thumb_down = (thumb_down_raw & 0x01) != 0;

  if (thumb_up) {
    return "thumb_up";
  }
  if (thumb_down) {
    return "thumb_down";
  }

  switch (gesture_type) {
    case 0:
      if (cursor_select) {
        if (cursor_type == 1) return "tap";
        if (cursor_type == 2) return "grab";
        if (cursor_type == 3) return "pinch";
      }
      break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5: {
      char buffer[16];
      std::snprintf(buffer, sizeof(buffer), "%u_finger", gesture_type);
      return std::string(buffer);
    }
    case 6: {
      char buffer[32];
      std::snprintf(buffer, sizeof(buffer), "rotate_right_%d", rotate);
      return std::string(buffer);
    }
    case 7: {
      char buffer[32];
      std::snprintf(buffer, sizeof(buffer), "rotate_left_%d", rotate);
      return std::string(buffer);
    }
    case 8:
      return "swipe_left";
    case 9:
      return "swipe_right";
    case 19:
    case 20:
    case 21:
    case 22:
    case 23: {
      char buffer[24];
      std::snprintf(buffer, sizeof(buffer), "%u_finger_push", (gesture_type - 19U) + 1U);
      return std::string(buffer);
    }
    default:
      break;
  }

  return "";
}

}  // namespace pag7660
}  // namespace esphome
