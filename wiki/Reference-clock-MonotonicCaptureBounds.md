# src/clock/MonotonicCaptureBounds.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `c3cb4436f0bd831c7800ca3669ce8342db9b1b72`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/c3cb4436f0bd831c7800ca3669ce8342db9b1b72/src/clock/MonotonicCaptureBounds.hpp)

## Purpose and invariant

`MonotonicCaptureBounds` is an exact 16-byte, trivially copyable closed canonical local-monotonic interval containing one occurrence. It contains no provider-native counter or wrap state. Reversed coordinates encode unavailable evidence and are never reordered.

## State

- `_earliest` — earliest possible canonical `MonotonicTimestamp`; default sentinel is 1 ns.
- `_latest` — latest possible canonical `MonotonicTimestamp`; default sentinel is 0 ns.

The default pair is deliberately reversed, making availability representable without another byte/padding.

## Construction

- default constructor — unavailable evidence.
- private interval constructor — stores validated endpoints.
- `Between(earliest, latest)` — returns the interval when ordered, otherwise unavailable.
- `Exact(timestamp)` — produces a zero-width interval.

## Access and reduction

- `IsAvailable()` — tests earliest <= latest.
- `Earliest()` / `Latest()` — return closed endpoints; callers still check availability.
- `Midpoint()` — returns `earliest + floor(width/2)` without overflow; unavailable returns origin sentinel.
- `Uncertainty()` — returns `ceil(width/2)`, saturating through `SynchronizationUncertainty`; unavailable is saturated.

All operations are constexpr/noexcept and allocate nothing.
