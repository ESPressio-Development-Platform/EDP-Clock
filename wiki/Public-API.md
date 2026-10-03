# Public API

The aggregation entry point is `<ESPressio_Clock.hpp>`.

## Monotonic and duration values

`MonotonicTimestamp` is the canonical local 8-byte nanosecond coordinate. `Duration` is signed; `Delta()` permits subtraction only inside the same coordinate domain. `MonotonicCaptureBounds` is a 16-byte closed interval known to contain one local occurrence; reversed intervals become unavailable, midpoint arithmetic cannot overflow, and ceil-half-width uncertainty saturates conservatively.

## Synchronized discipline

`SynchronizedTimestamp`, `SynchronizationUncertainty`, `SynchronizationState`, `SynchronizedReading`, `SynchronizationObservation`, the four-timestamp exchange/result/status values and `SynchronizedClockProvider` form the source-neutral discipline API.

One upstream writer serializes `Observe()`, `BeginSameEraTransition()` and `BeginCrossEraTransition()`. Readers may call `Now()` and `Correlation()` concurrently through fixed Platform snapshot publication.

## Correlation and temporal provenance

`ClockCorrelation` (32 bytes) and `ClockCorrelationProjection` (16 bytes) project captured monotonic occurrences through one immutable accepted-reference mapping.

`ClockEra` is an opaque zero-invalid 8-byte comparable key. `EraQualifiedClockCorrelation` (40 bytes), `EraQualifiedClockCorrelationProjection` (24 bytes) and flattened `EraQualifiedClockAssessment` (24 bytes) preserve provenance and application assessment without containing Device identity, lineage or native provider ticks.

`TemporalQualityAssessment` represents `NotRequired`, `Reliable` or admitted `Unreliable` evidence. Strict policy refusal is outside the admitted value.

## Stopwatch and convenience bindings

`Stopwatch`, `MonotonicNow()` and `SynchronizedNow()` retain their existing fixed, Bootstrap-bound contracts.

Exact declarations and invariants are indexed in [Reference Index](Reference-Index).
