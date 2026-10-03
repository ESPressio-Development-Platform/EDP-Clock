# src/clock/ClockEra.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `c3cb4436f0bd831c7800ca3669ce8342db9b1b72`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/c3cb4436f0bd831c7800ca3669ce8342db9b1b72/src/clock/ClockEra.hpp)

## Purpose and invariant

`ClockEra` is an exact 8-byte, trivially copyable, opaque temporal-provenance key. Zero is invalid/unspecified. EDP-Clock compares and carries the value but never derives or interprets its upstream provenance.

The upstream coordinator must maintain and conflict-check the complete mapping. Numeric order is deterministic container order, not chronology between unrelated Eras.

## State

- `_value` — private `uint64_t` opaque representation; zero means invalid.

## Construction and access

- default constructor — creates invalid Era zero.
- private value constructor — stores an already selected representation.
- `FromValue(uint64_t)` — constructs from upstream representation; zero remains invalid.
- `Value()` — returns the representation for canonical carriage/comparison.
- `IsValid()` — true exactly when non-zero.

## Comparison

`==`, `!=`, `<`, `<=`, `>`, and `>=` compare only the opaque representation. Every operation is constexpr, noexcept, allocation-free and independent of external lifetime.
