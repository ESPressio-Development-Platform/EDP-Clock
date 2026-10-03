# src/clock/Clock.hpp

**Primary classification:** PUBLIC API aggregation header

**Source baseline:** `c3cb4436f0bd831c7800ca3669ce8342db9b1b72`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/c3cb4436f0bd831c7800ca3669ce8342db9b1b72/src/clock/Clock.hpp)

This header exports the complete Clock surface and declares no independent symbols. Consumers normally include `ESPressio_Clock.hpp`, which includes this aggregation header.

## Direct includes

- `ClockComposition.hpp`
- `ClockCorrelation.hpp`
- `ClockCorrelationProjection.hpp`
- `ClockCorrelationProjectionStatus.hpp`
- `ClockCorrelationState.hpp`
- `ClockDisciplineTransition.hpp`
- `ClockEra.hpp`
- `Delta.hpp`
- `Duration.hpp`
- `EraQualifiedClockAssessment.hpp`
- `EraQualifiedClockCorrelation.hpp`
- `EraQualifiedClockCorrelationProjection.hpp`
- `FourTimestampEstimator.hpp`
- `FourTimestampExchange.hpp`
- `FourTimestampExchangeResult.hpp`
- `FourTimestampExchangeStatus.hpp`
- `MonotonicCaptureBounds.hpp`
- `MonotonicClockProvider.hpp`
- `MonotonicNow.hpp`
- `MonotonicTimebaseProvider.hpp`
- `MonotonicTimestamp.hpp`
- `Stopwatch.hpp`
- `SynchronizationObservation.hpp`
- `SynchronizationObservationStatus.hpp`
- `SynchronizationState.hpp`
- `SynchronizationUncertainty.hpp`
- `SynchronizedClockProvider.hpp`
- `SynchronizedNow.hpp`
- `SynchronizedReading.hpp`
- `SynchronizedTimestamp.hpp`
- `TemporalQualityAssessment.hpp`

The aggregation order is not an ownership hierarchy. In particular, source/provider selection and full Era provenance remain upstream responsibilities.
