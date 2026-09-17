# ESPressio Clock

`EDP-Clock` defines the physical and electronic Clock domain used by the ESPressio Development Platform.

The library deliberately models the scientific concept of time and the electronic concept of a clock. It does **not** define calendars, dates, time zones, civil time, RTC-device semantics, or any other human interpretation of time.

## Contents

The following Clock facilities are currently provided:

- [Monotonic Clock Foundation](./docs/MONOTONIC-CLOCK.MD)  
  Defines the application-selected `MonotonicTimebase`, generic `MonotonicClock`, canonical nanosecond `MonotonicTimestamp`, signed `Duration`, `Delta()` arithmetic, and application-wide `MonotonicNow()` convenience surface.

- [Synchronized Clock](./docs/SYNCHRONIZED-CLOCK.MD)  
  Derives a shared synchronized coordinate from `MonotonicClock`, reduces source-agnostic T1/T2/T3/T4 exchanges into conservative synchronization observations, performs bounded integer-only frequency/phase discipline, reports explicit uncertainty and lifecycle state, and exposes application-wide `SynchronizedNow()`.

- [Clock Correlation](./docs/CLOCK-CORRELATION.MD)  
  Captures immutable monotonic-to-reference mappings for retrospective hardware/event timestamp projection, including signed frequency correction, explicit capture uncertainty, and bounded projection quality without retaining Clock history.

## Dependencies

`EDP-Clock` depends on:

- `EDP-System` for the Composition Framework;
- `EDP-Platform` for the lock-free `AtomicWord32` contract and generic fixed-storage `ConcurrentSnapshot` used by synchronized-clock discipline state;
- standard C++ fixed-width integer/type facilities.

`EDP-Clock` contains no Arduino, ESP-IDF, FreeRTOS, RTC, operating-system, hardware-driver, or transport-specific synchronization implementation. Concrete `AtomicWord32` providers live in the appropriate `EDP-Platform-*` repository; concrete monotonic timebases and synchronization transports remain separately owned by their appropriate platform/domain libraries or the consuming application.

## Bootstrap Order

The intended initialization order is:

```text
System Identity
    |
    v
Platform Requirements
    |
    v
Clock
    |
    v
everything else
```

Clock therefore consumes already-selected Platform concurrency requirements; Platform does not depend on Clock.

## Development Status

The monotonic and synchronized Clock foundations, four-timestamp time transfer, and immutable Clock Correlation are implemented. Higher-level facilities such as Stopwatch, timers, scheduling, alarms, and human-time interpretation remain intentionally outside the current tranche.
