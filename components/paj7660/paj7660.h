#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"
// Include the official Seeed Studio header you pulled from their GitHub
#include "Gesture.h"

namespace esphome
{
  namespace paj7660
  {

    // Inherit from PollingComponent to run an update loop,
    // and TextSensor to push string states to Home Assistant
    class PAJ7660Component : public PollingComponent, public text_sensor::TextSensor
    {
    public:
      // Set the polling interval (e.g., check the sensor every 100ms)
      PAJ7660Component() : PollingComponent(100) {}

      // The Seeed library object
      pag7660 Gesture;

      void setup() override
      {
        // Called once when the ESP32-C6 boots
        if (!Gesture.init())
        {
          ESP_LOGE("paj7660", "Failed to initialize PAJ7660 sensor!");
          this->mark_failed();
          return;
        }
        ESP_LOGI("paj7660", "PAJ7660 initialized successfully");
      }

      void update() override
      {
        // Called repeatedly based on the polling interval
        pag7660_gesture_t result;

        // Check if a new gesture is detected by the hardware
        if (Gesture.getResult(result))
        {
          // Map the Seeed integer/struct result to a string for Home Assistant
          std::string gesture_name = map_gesture_to_string(result);

          if (!gesture_name.empty())
          {
            ESP_LOGD("paj7660", "Gesture detected: %s", gesture_name.c_str());
            this->publish_state(gesture_name);
          }
        }
      }

    protected:
      // Helper function to translate Seeed's raw register outputs to simple strings
      std::string map_gesture_to_string(const pag7660_gesture_t &result)
      {
        // You will expand this switch statement based on the Seeed library's definitions
        // for pinch, grab, N-finger push, etc.
        switch (result.type)
        {
        // Example cases (you will need to match the exact enums from Gesture.h)
        // case GESTURE_UP: return "up";
        // case GESTURE_PINCH: return "pinch";
        default:
          return "";
        }
      }
    };

  } // namespace paj7660
} // namespace esphome