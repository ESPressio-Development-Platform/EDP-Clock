# ESPressio Clock

`EDP-Clock` defines the physical and electronic Clock domain used by the ESPressio Development Platform.

The library deliberately models the scientific concept of time and the electronic concept of a clock. It does **not** define calendars, dates, time zones, civil time, RTC-device semantics, or any other human interpretation of time.

## Contents

The following Clock facilities are currently provided:

- [Monotonic Clock Foundation](./docs/MONOTONIC-CLOCK.MD)  
  Defines the application-selected `MonotonicTimebase`, generic `MonotonicClock`, canonical nanosecond `MonotonicTimestamp`, signed `Duration`, `Delta()` arithmetic, and application-wide `MonotonicNow()` convenience surface.

## Dependencies

The Clock foundation depends only on the `EDP-System` Composition Framework and standard C++ fixed-width integer/type facilities.

It has no Arduino, ESP-IDF, FreeRTOS, RTC, operating-system, hardware-driver, or other platform-specific dependency. Concrete timebase providers belong in the appropriate platform, hardware, RTC, or application library.

## Development Status

`EDP-Clock` is being rebuilt progressively from a clean foundation. Higher-level facilities such as synchronized time, Stopwatch, timers, scheduling, and clock discipline will be introduced only after their lower-level Clock contracts have been designed and validated.
