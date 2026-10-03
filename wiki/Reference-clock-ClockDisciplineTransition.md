# src/clock/ClockDisciplineTransition.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `c3cb4436f0bd831c7800ca3669ce8342db9b1b72`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/c3cb4436f0bd831c7800ca3669ce8342db9b1b72/src/clock/ClockDisciplineTransition.hpp)

## Purpose

Defines the source-neutral choices/results for explicit synchronized discipline transitions. Application/upstream policy chooses when and which operation to invoke; Clock owns only discipline effects.

## `CrossEraTransitionMode`

An exact one-byte enum.

- `PreservePublishedTimeline`: invalidate old reference correlation but retain the current public coordinate as a non-regressing, saturated-uncertainty timeline for new-Era slew.
- `ReconstructTimeline`: discard prior discipline and return to `NeverSynchronized`, allowing the next observation to establish a fresh coordinate.

## `ClockDisciplineTransitionStatus`

An exact one-byte result enum.

- `Applied`: requested transition state was published.
- `NoPublishedTimeline`: preservation/same-Era handling found no previously authoritative timeline; provisional state was cleared.

Neither enum owns resources or performs work. Operations consuming them share the synchronized provider's single-writer contract.
