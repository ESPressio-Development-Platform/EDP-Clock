# src/clock/EraQualifiedClockCorrelation.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `c3cb4436f0bd831c7800ca3669ce8342db9b1b72`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/c3cb4436f0bd831c7800ca3669ce8342db9b1b72/src/clock/EraQualifiedClockCorrelation.hpp)

## Purpose and layout

Pairs one opaque Era with an immutable 32-byte `ClockCorrelation`. Exact size is 40 bytes; no identity or lineage is duplicated in each value.

## State

- `_era` — temporal provenance key.
- `_correlation` — accepted monotonic/reference mapping.

## Operations

- default constructor — unavailable Era-qualified mapping.
- value constructor — copies an Era and correlation.
- `Era()` / `Correlation()` — immutable access.
- `IsAvailable()` — requires both valid Era and available correlation.
- `Correlate(timestamp, captureUncertainty)` — projects through the mapping and retains Era. If either component is unavailable it returns default unavailable Era-qualified projection.

Projection is constexpr/noexcept, integer-only and allocation-free.
