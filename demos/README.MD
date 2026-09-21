# Demos

Buildable demonstrations are provided for the logically distinct Clock facilities currently implemented:

- `monotonic-clock-basic`
- `synchronized-clock-basic`
- `four-timestamp-estimator-basic`
- `clock-correlation-basic`
- `stopwatch-basic`

Each demo contains `Arduino_IDE`, `PlatformIO_Arduino`, and `PlatformIO_ESP-IDF` targets. Synchronized Clock and Clock Correlation select the appropriate concrete Platform atomic provider for the target.

## C++ Language Baseline

All PlatformIO-compatible demos require C++20 and use the maintained PIOArduino ESP32 platform package rather than the outdated Arduino-ESP32 package bundled by the stock PlatformIO Espressif32 platform. PlatformIO Core/IDE remains usable because each demo selects PIOArduino explicitly in its `platformio.ini`.
