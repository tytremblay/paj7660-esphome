# PAG7660 ESPHome External Component

This repository is structured for ESPHome `github://` external component imports
and implements a `pag7660` text sensor platform based on Seeed's
Grove_Gesture initialization and gesture-decoding examples.

## Repository structure

```text
paj7660-esphome/
├── components/
│   └── pag7660/
│       ├── __init__.py
│       ├── text_sensor.py
│       ├── pag7660.h
│       └── pag7660.cpp
├── pag7660-gesture.yaml
└── secrets.example.yaml
```

## Use from another ESPHome project

```yaml
external_components:
  - source: github://tytremblay/paj7660-esphome
    components: [pag7660]

i2c:
  sda: GPIO21
  scl: GPIO22

text_sensor:
  - platform: pag7660
    name: "Gesture"
    gesture_mode: combined   # combined | thumb | cursor
```

## Local development

1. Copy secrets:

   ```bash
   cp secrets.example.yaml secrets.yaml
   ```

2. Validate:

   ```bash
   esphome config pag7660-gesture.yaml
   ```

3. Build/upload:

   ```bash
   esphome run pag7660-gesture.yaml
   ```
