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

- [Stopwatch](./docs/STOPWATCH.MD)  
  Measures non-negative elapsed physical time from an application-selected `MonotonicClock`, with start/resume, stop, reset, restart, deterministic saturation, and no dependency on synchronization or scheduling facilities.

## Dependencies

```text
EDP-Clock
├── EDP-System [mandatory]
└── EDP-Platform [mandatory]
    └── EDP-System [mandatory, transitive]
```

`EDP-Clock` currently has no optional ESPressio library dependencies.

`SynchronizedClockProvider` declares its concrete Platform `AtomicWord32` need as an explicit external Composition `Requirement`. A consuming Architecture must therefore contain exactly one qualified provider; the concrete provider repository remains an application capability choice rather than an additional library dependency of `EDP-Clock`.

See [DEPENDENCY_GRAPH.MD](./DEPENDENCY_GRAPH.MD) for the maintained dependency graph and target-provider selection notes.

## Demos

Buildable demonstrations are provided under [`./demos`](./demos) for:

- `monotonic-clock-basic`;
- `synchronized-clock-basic`;
- `four-timestamp-estimator-basic`;
- `clock-correlation-basic`;
- `stopwatch-basic`.

Each demo contains Arduino IDE, PlatformIO Arduino, and PlatformIO ESP-IDF targets. Synchronized Clock and Clock Correlation use the target-appropriate concrete `AtomicWord32` provider.

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

The monotonic and synchronized Clock foundations, four-timestamp time transfer, immutable Clock Correlation, and Stopwatch are implemented. Timers, scheduling, alarms, RTC-device semantics, and human-time interpretation remain intentionally outside the current Clock foundation.

## C++ Language Baseline

ESPressio requires C++20. For PlatformIO-compatible ESP32 development, PIOArduino is the recommended platform baseline because it tracks a current Arduino-ESP32/ESP-IDF toolchain. Stock PlatformIO Core/IDE may still be used when the project explicitly selects the PIOArduino platform package and compiles as C++20.

See the EDP-System [C++20 Toolchain Baseline](https://github.com/ESPressio-Development-Platform/EDP-System/blob/main/docs/CPP20-TOOLCHAIN.MD) for the canonical configuration.
