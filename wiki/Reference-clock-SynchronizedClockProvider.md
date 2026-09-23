# src/clock/SynchronizedClockProvider.hpp

**Primary classification:** PUBLIC PROVIDER / EXTENSION API

**Source baseline:** `6a9ab9ffab8c90901f48b6c2c70767942fe43380`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/6a9ab9ffab8c90901f48b6c2c70767942fe43380/src/clock/SynchronizedClockProvider.hpp)

## Direct includes

- `algorithm`
- `cstddef`
- `cstdint`
- `limits`
- `type_traits`
- `utility`
- `ESPressio_Platform.hpp`
- `ClockComposition.hpp`
- `ClockCorrelation.hpp`
- `MonotonicTimestamp.hpp`
- `SynchronizationObservation.hpp`
- `SynchronizationObservationStatus.hpp`
- `SynchronizationState.hpp`
- `SynchronizationUncertainty.hpp`
- `SynchronizedReading.hpp`
- `SynchronizedTimestamp.hpp`

## Documented declarations

### `TMonotonicClockProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Validates the MonotonicClock provider used as the physical basis of synchronized time.

```cpp
template<class TMonotonicClockProvider>
        struct SynchronizedMonotonicClockTraits
```

### `Properties`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Properties advertised by the selected MonotonicClock provider.

```cpp
using Properties = typename TMonotonicClockProvider::CompositionOffers::template PropertiesFor<MonotonicClock>;
```

### `ResolutionNanoseconds`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Conservative whole-nanosecond resolution of the selected monotonic clock.

```cpp
static constexpr std::uint64_t ResolutionNanoseconds =
                Properties::template Value<ClockResolutionNanoseconds>;
```

### `NowResult`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Return type produced by the selected provider's Now operation.

```cpp
using NowResult = decltype(
                std::declval<const TMonotonicClockProvider&>().Now()
            );
```

### `SynchronizedClockState`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Compact mutable mapping state published coherently to synchronized-clock readers.

```cpp
struct SynchronizedClockState final
```

### `MonotonicAnchorNanoseconds`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Monotonic coordinate at which the current mapping state is anchored.

```cpp
std::uint64_t MonotonicAnchorNanoseconds;
```

### `SynchronizedAnchorNanoseconds`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Published synchronized coordinate at the mapping anchor.

```cpp
std::uint64_t SynchronizedAnchorNanoseconds;
```

### `ReferenceAnchorNanoseconds`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Most recent accepted raw reference coordinate at the mapping anchor.

```cpp
std::uint64_t ReferenceAnchorNanoseconds;
```

### `RemainingPhaseCorrectionNanoseconds`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Remaining signed phase correction to apply through bounded slew.

```cpp
std::int64_t RemainingPhaseCorrectionNanoseconds;
```

### `FrequencyCorrectionPartsPerBillion`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Long-term signed frequency correction applied to monotonic elapsed time.

```cpp
std::int32_t FrequencyCorrectionPartsPerBillion;
```

### `FrequencyUncertaintyPartsPerBillion`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Conservative uncertainty of the current frequency estimate in parts per billion.

```cpp
std::uint32_t FrequencyUncertaintyPartsPerBillion;
```

### `UncertaintyAtAnchorNanoseconds`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Conservative synchronization uncertainty at the mapping anchor.

```cpp
std::uint32_t UncertaintyAtAnchorNanoseconds;
```

### `State`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Last explicitly established lifecycle state for this mapping.

```cpp
SynchronizationState State;
```

### `HasObservation`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Indicates whether at least one observation has been accepted in this runtime.

```cpp
std::uint8_t HasObservation;
```

### `Reserved`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Reserved zeroed storage keeps the 48-byte snapshot representation fully explicit.

```cpp
std::uint16_t Reserved;
```

### `ScaledFractionResult`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Exact quotient/remainder of `(numerator * scale) / denominator` for a proper fraction.

```cpp
struct ScaledFractionResult final
```

### `std::uint64_t Quotient{0U};`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Integer quotient of the scaled fraction.

```cpp
std::uint64_t Quotient{0U};
```

### `std::uint64_t Remainder{0U};`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Remainder after dividing the scaled numerator by the denominator.

```cpp
std::uint64_t Remainder{0U};
```

### `SynchronizedProjection`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Projected synchronized coordinate and uncertainty derived from one immutable state snapshot.

```cpp
struct SynchronizedProjection final
```

### `std::uint64_t TimestampNanoseconds{0U};`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Projected synchronized coordinate in nanoseconds.

```cpp
std::uint64_t TimestampNanoseconds{0U};
```

### `max`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Conservative projected uncertainty in nanoseconds.

```cpp
std::uint32_t UncertaintyNanoseconds{std::numeric_limits<std::uint32_t>::max()};
```

### `RemainingPhaseCorrectionNanoseconds`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Remaining phase correction after the projected elapsed interval.

```cpp
std::int64_t RemainingPhaseCorrectionNanoseconds;
```

### `SaturatingAdd64`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Adds two unsigned values and saturates at the 64-bit boundary.

```cpp
constexpr std::uint64_t SaturatingAdd64(
            std::uint64_t left,
            std::uint64_t right
        ) noexcept
```

### `SaturatingAddUncertainty`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Adds two uncertainty magnitudes and saturates at the 32-bit uncertainty boundary.

```cpp
constexpr std::uint32_t SaturatingAddUncertainty(
            std::uint32_t left,
            std::uint64_t right
        ) noexcept
```

### `SignedMagnitude`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Returns the unsigned magnitude of a signed 64-bit value without overflowing at INT64_MIN.

```cpp
constexpr std::uint64_t SignedMagnitude(
            std::int64_t value
        ) noexcept
```

### `SignedFromMagnitude`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Reconstructs a signed 64-bit value from a sign and an already-bounded magnitude.

```cpp
constexpr std::int64_t SignedFromMagnitude(
            bool negative,
            std::uint64_t magnitude
        ) noexcept
```

### `SignedCoordinateDifference`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Calculates `to - from` as a signed nanosecond difference with deterministic saturation.

```cpp
constexpr std::int64_t SignedCoordinateDifference(
            std::uint64_t from,
            std::uint64_t to
        ) noexcept
```

### `ScaleProperFraction`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Scales a proper fraction by a 32-bit integer without requiring a 128-bit intermediate.

Preconditions: `numerator < denominator` and `denominator != 0`.

```cpp
inline ScaledFractionResult ScaleProperFraction(
            std::uint64_t numerator,
            std::uint32_t scale,
            std::uint64_t denominator
        ) noexcept
```

### `ScaleElapsedByPartsPerBillion`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Scales elapsed nanoseconds by a bounded parts-per-billion magnitude.

```cpp
inline std::uint64_t ScaleElapsedByPartsPerBillion(
            std::uint64_t elapsedNanoseconds,
            std::uint32_t partsPerBillion
        ) noexcept
```

### `ScaleElapsedUncertainty`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Scales elapsed nanoseconds by rate uncertainty and saturates at the uncertainty boundary.

```cpp
inline std::uint32_t ScaleElapsedUncertainty(
            std::uint64_t elapsedNanoseconds,
            std::uint32_t uncertaintyPartsPerBillion
        ) noexcept
```

### `RemainingPhaseAfterApplication`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Returns the remaining signed phase correction after applying a bounded unsigned amount.

```cpp
constexpr std::int64_t RemainingPhaseAfterApplication(
            std::int64_t phaseCorrection,
            std::uint64_t appliedMagnitude
        ) noexcept
```

### `EstimateFrequencyCorrection`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Estimates the signed source-frequency correction from two accepted observations.

- **Parameter `localDeltaNanoseconds`:** Monotonic elapsed time between observations.
- **Parameter `referenceDeltaNanoseconds`:** Synchronized-reference elapsed time between observations.
- **Parameter `maximumCorrectionPartsPerBillion`:** Maximum permitted absolute correction.
- **Parameter `correction`:** Receives the rounded signed correction when the estimate is acceptable.
- **Returns:** true when the estimate is within the configured correction bound.

```cpp
inline bool EstimateFrequencyCorrection(
            std::uint64_t localDeltaNanoseconds,
            std::uint64_t referenceDeltaNanoseconds,
            std::uint32_t maximumCorrectionPartsPerBillion,
            std::int32_t& correction
        ) noexcept
```

### `EstimateFrequencyUncertainty`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Estimates frequency uncertainty from the uncertainty of two observation coordinates.

```cpp
inline std::uint32_t EstimateFrequencyUncertainty(
            std::uint32_t previousUncertaintyNanoseconds,
            std::uint32_t currentUncertaintyNanoseconds,
            std::uint64_t localDeltaNanoseconds
        ) noexcept
```

### `TMonotonicClockProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Generic synchronized clock disciplined from source-agnostic synchronization observations.

The synchronized clock is derived entirely from the selected MonotonicClock. It owns no
second physical counter. Mutable discipline state is published through EDP-Platform's
fixed-storage ConcurrentSnapshot so one writer can update the mapping while arbitrary
application threads call Now() without a mutex, heap allocation, worker task, or vtable.

Exactly one execution context may call Observe() for a given provider. Source selection and
serialization of observations therefore remain upstream responsibilities.

- **Template parameter `TMonotonicClockProvider`:** Concrete application-selected MonotonicClock provider.
- **Template parameter `TAtomicWordProvider`:** Concrete application-selected Platform AtomicWord32 provider.
- **Template parameter `TMaximumFrequencyCorrectionPartsPerBillion`:** Maximum long-term correction magnitude.
- **Template parameter `TMaximumPhaseSlewPartsPerBillion`:** Maximum temporary phase-slew magnitude.
- **Template parameter `TSynchronizationUncertaintyLimitNanoseconds`:** Exclusive synchronization-quality limit.

```cpp
template<
        class TMonotonicClockProvider,
```

### `MonotonicTraits`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Validated monotonic-clock metadata used by this provider.

```cpp
using MonotonicTraits = Detail::SynchronizedMonotonicClockTraits<TMonotonicClockProvider>;
```

### `StateSnapshot`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Fixed-storage concurrent discipline snapshot supplied by EDP-Platform.

```cpp
using StateSnapshot = ESPressio::Platform::Concurrency::ConcurrentSnapshot<
            Detail::SynchronizedClockState,
```

### `_monotonicClock`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Borrowed lifetime-stable monotonic clock selected by application Bootstrap.

```cpp
const TMonotonicClockProvider* _monotonicClock;
```

### `_state`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Coherently published mutable synchronized-clock discipline state.

```cpp
StateSnapshot _state;
```

### `InitialState`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Creates the initial never-synchronized discipline state for this runtime incarnation.

```cpp
static Detail::SynchronizedClockState InitialState() noexcept
```

### `ObservationUncertainty`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Combines observation uncertainty with monotonic-clock resolution conservatively.

```cpp
static std::uint32_t ObservationUncertainty(
            const SynchronizationObservation& observation
        ) noexcept
```

### `EffectiveState`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Derives the externally visible lifecycle state for one projected reading.

```cpp
static SynchronizationState EffectiveState(
            SynchronizationState storedState,
            std::uint32_t uncertaintyNanoseconds
        ) noexcept
```

### `Project`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Projects one immutable discipline snapshot to the supplied monotonic coordinate.

```cpp
static Detail::SynchronizedProjection Project(
            const Detail::SynchronizedClockState& state,
            std::uint64_t monotonicNanoseconds
        ) noexcept
```

### `ReadingAt`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Projects one state snapshot to a complete public reading at a monotonic coordinate.

```cpp
static SynchronizedReading ReadingAt(
            const Detail::SynchronizedClockState& state,
            const MonotonicTimestamp& monotonicTimestamp
        ) noexcept
```

### `DegradeForRejectedFrequency`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Degrades a previously synchronized mapping after an implausible frequency observation.

```cpp
void DegradeForRejectedFrequency(
            Detail::SynchronizedClockState state
        ) noexcept
```

### `SynchronizedClockProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Creates a synchronized clock over one lifetime-stable MonotonicClock provider.

- **Parameter `monotonicClock`:** Monotonic clock selected and initialized by application Bootstrap.

```cpp
explicit SynchronizedClockProvider(
            const TMonotonicClockProvider& monotonicClock
        ) noexcept :
            _monotonicClock(&monotonicClock),
```

### `SynchronizedClockProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Prevents duplicating live synchronized-clock discipline state.

```cpp
SynchronizedClockProvider(const SynchronizedClockProvider&) = delete;
```

### `operator`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Prevents copy assignment of live synchronized-clock discipline state.

```cpp
SynchronizedClockProvider& operator =(const SynchronizedClockProvider&) = delete;
```

### `SynchronizedClockProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Prevents relocating a synchronized clock after readers may reference it.

```cpp
SynchronizedClockProvider(SynchronizedClockProvider&&) = delete;
```

### `operator`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Prevents move assignment of live synchronized-clock discipline state.

```cpp
SynchronizedClockProvider& operator =(SynchronizedClockProvider&&) = delete;
```

### `SynchronizationUncertaintyLimitNanoseconds`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Strict upper bound for uncertainty while a reading may report Synchronized.

```cpp
static constexpr std::uint32_t SynchronizationUncertaintyLimitNanoseconds =
            TSynchronizationUncertaintyLimitNanoseconds;
```

### `MaximumFrequencyCorrectionPartsPerBillion`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Maximum permitted magnitude of long-term source-frequency correction.

```cpp
static constexpr std::uint32_t MaximumFrequencyCorrectionPartsPerBillion =
            TMaximumFrequencyCorrectionPartsPerBillion;
```

### `MaximumPhaseSlewPartsPerBillion`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Maximum permitted magnitude of temporary phase slew.

```cpp
static constexpr std::uint32_t MaximumPhaseSlewPartsPerBillion =
            TMaximumPhaseSlewPartsPerBillion;
```

### `sizeof`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Consumer state bytes retained inside one ConcurrentSnapshot buffer.

```cpp
static constexpr std::size_t DisciplineStateBytes = sizeof(Detail::SynchronizedClockState);
```

### `ConcurrentStateStorageBytes`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Total fixed atomic storage retained by the discipline ConcurrentSnapshot.

```cpp
static constexpr std::size_t ConcurrentStateStorageBytes = StateSnapshot::AtomicStorageBytes;
```

### `Correlation`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Returns one immutable snapshot of the best accepted monotonic-to-reference mapping.

Correlation uses the raw accepted reference anchor rather than the phase-slewed published
SynchronizedClock anchor. The returned value is therefore suitable for retrospectively
projecting hardware/event monotonic timestamps even while the public clock is Reacquiring.

```cpp
ClockCorrelation Correlation() const noexcept
```

### `Now`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Returns the current synchronized timestamp and its quality information.

The coherent discipline snapshot is captured before the monotonic sample. Therefore a
concurrent writer may publish a newer mapping while this call executes, but the sampled
monotonic coordinate can never precede the anchor contained by the state being projected.

```cpp
SynchronizedReading Now() const noexcept
```

### `Observe`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Applies one source-agnostic synchronization observation to the clock discipline.

Exactly one execution context may call Observe() for this provider. Observations must be
serialized upstream before entering Clock. A first-ever mapping may step directly to the
reference coordinate; after successful synchronization, corrections are applied through
bounded slew so the published synchronized coordinate never moves backwards.

- **Parameter `observation`:** Accepted upstream estimate relating monotonic and synchronized time.
- **Returns:** Status describing whether the observation was accepted by Clock discipline.

```cpp
SynchronizationObservationStatus Observe(
            const SynchronizationObservation& observation
        ) noexcept
```

