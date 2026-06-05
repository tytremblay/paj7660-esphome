#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace pag7660 {

class PAG7660TextSensor : public text_sensor::TextSensor,
                          public PollingComponent,
                          public i2c::I2CDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;

  void set_gesture_mode(uint8_t mode) { this->gesture_mode_ = mode; }

 protected:
  std::string decode_gesture_(const uint8_t *frame, size_t frame_len) const;
  bool clear_frame_ready_();

  uint8_t gesture_mode_{5};  // Seeed default: GESTURE_COMBINED_MODE
  std::string last_state_;
};

}  // namespace pag7660
}  // namespace esphome
