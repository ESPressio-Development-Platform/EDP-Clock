# src/clock/TemporalQualityAssessment.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `c3cb4436f0bd831c7800ca3669ce8342db9b1b72`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/c3cb4436f0bd831c7800ca3669ce8342db9b1b72/src/clock/TemporalQualityAssessment.hpp)

## Purpose

One-byte vocabulary carrying the application policy's assessment of authenticated temporal evidence. Clock defines representation only and applies no admission policy.

## Values

- `Unreliable` (0) — admitted advisory evidence does not satisfy selected temporal quality; it is the fail-safe default.
- `NotRequired` (1) — application policy does not require time quality for this admitted information.
- `Reliable` (2) — evidence satisfies application policy.

Strict refusal is not an enum value because refused information is not admitted into a family context.
