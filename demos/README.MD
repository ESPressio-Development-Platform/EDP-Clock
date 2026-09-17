# Demos

Buildable demonstrations are provided for the logically distinct Clock facilities currently implemented:

- `monotonic-clock-basic`
- `synchronized-clock-basic`
- `four-timestamp-estimator-basic`
- `clock-correlation-basic`
- `stopwatch-basic`

Each demo contains `Arduino_IDE`, `PlatformIO_Arduino`, and `PlatformIO_ESP-IDF` targets. Synchronized Clock and Clock Correlation select the appropriate concrete Platform atomic provider for the target.
