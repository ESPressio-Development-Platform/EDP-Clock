# src/clock/EraQualifiedClockCorrelationProjection.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `c3cb4436f0bd831c7800ca3669ce8342db9b1b72`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/c3cb4436f0bd831c7800ca3669ce8342db9b1b72/src/clock/EraQualifiedClockCorrelationProjection.hpp)

## Purpose and layout

Pairs one opaque Era with one immutable `ClockCorrelationProjection`. Exact size is 24 bytes and the Type is trivially copyable.

## State

- `_era` — Era in which the synchronized coordinate is meaningful.
- `_projection` — timestamp, uncertainty and projection status.

## Operations

- default constructor — unavailable Era and projection.
- value constructor — copies one Era and projection; performs no provenance validation.
- `Era()` — returns the Era.
- `Projection()` — returns the underlying immutable projection.
- `IsCorrelated()` — true only when Era is valid and projection status is correlated.

The value owns all bytes, has no external lifetime and does not apply application reliability policy.
