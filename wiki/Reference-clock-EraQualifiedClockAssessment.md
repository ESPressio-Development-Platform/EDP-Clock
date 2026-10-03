# src/clock/EraQualifiedClockAssessment.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `c3cb4436f0bd831c7800ca3669ce8342db9b1b72`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/c3cb4436f0bd831c7800ca3669ce8342db9b1b72/src/clock/EraQualifiedClockAssessment.hpp)

## Purpose and layout

Compact immutable Era-qualified occurrence projection plus application temporal assessment. It is flattened to exactly 24 bytes rather than nesting a 24-byte projection and expanding to 32 bytes through alignment.

## State

- `_era` — opaque Era.
- `_timestamp` — projected synchronized coordinate.
- `_uncertainty` — conservative bound.
- `_projectionStatus` — correlation/range outcome; defaults unavailable.
- `_assessment` — application policy result; defaults `Unreliable`.
- `_reserved` — explicit zeroed 16-bit padding preserving deterministic representation.

## Construction

- default constructor — invalid Era, unavailable projection, saturated uncertainty and unreliable assessment.
- value constructor — copies fields from one Era-qualified projection and supplied assessment.

## Access

`Era()`, `Timestamp()`, `Uncertainty()`, `ProjectionStatus()`, and `Assessment()` return immutable fields. `IsCorrelated()` requires both valid Era and correlated status; it deliberately does not equate correlation with policy reliability.

The Type owns no external resource and is trivially copyable.
