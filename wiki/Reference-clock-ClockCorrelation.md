# src/clock/ClockCorrelation.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `8298f13f210665ca90a2cbeb19de43e45342aa18`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/8298f13f210665ca90a2cbeb19de43e45342aa18/src/clock/ClockCorrelation.hpp)

## Direct includes

- `cstdint`
- `limits`
- `type_traits`
- `ClockCorrelationProjection.hpp`
- `ClockCorrelationState.hpp`
- `MonotonicTimestamp.hpp`
- `SynchronizationUncertainty.hpp`
- `SynchronizedTimestamp.hpp`

## Documented declarations

### `ClockCorrelationFactory`

**Classification:** PUBLIC API

Internal construction surface used by synchronized-clock providers to create correlations.

```cpp
struct ClockCorrelationFactory;
```

### `CorrelationRateMagnitude`

**Classification:** PUBLIC API

Returns the unsigned magnitude of a signed 32-bit rate correction.

```cpp
constexpr std::uint32_t CorrelationRateMagnitude(
            std::int32_t partsPerBillion
        ) noexcept
```

### `ScaleCorrelationElapsed`

**Classification:** PUBLIC API · source access: `public`

Scales elapsed nanoseconds by a sub-billion parts-per-billion magnitude.

```cpp
constexpr std::uint64_t ScaleCorrelationElapsed(
            std::uint64_t elapsedNanoseconds,
            std::uint32_t partsPerBillion
        ) noexcept
```

### `ScaleCorrelationUncertainty`

**Classification:** PUBLIC API · source access: `public`

Scales elapsed nanoseconds by rate uncertainty and saturates at the uncertainty boundary.

```cpp
constexpr std::uint32_t ScaleCorrelationUncertainty(
            std::uint64_t elapsedNanoseconds,
            std::uint32_t uncertaintyPartsPerBillion
        ) noexcept
```

### `AddCorrelationUncertainty`

**Classification:** PUBLIC API · source access: `public`

Adds one uncertainty contribution and saturates at the representable boundary.

```cpp
constexpr std::uint32_t AddCorrelationUncertainty(
            std::uint32_t current,
            std::uint32_t contribution
        ) noexcept
```

### `ClockCorrelation`

**Classification:** PUBLIC API · source access: `public`

Immutable snapshot relating local monotonic time to the best accepted synchronized reference.

A correlation represents the reference mapping known when the snapshot was captured. It is
intentionally independent from the non-regressing published SynchronizedClock timeline and
therefore does not include temporary phase slew. Consumers can retain this compact value and
project many delayed hardware/event timestamps without retaining Clock history or repeatedly
reading the concurrent synchronized-clock discipline state.

```cpp
class ClockCorrelation final
```

### `_monotonicAnchor`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Local monotonic coordinate at which the reference mapping is anchored.

```cpp
MonotonicTimestamp _monotonicAnchor;
```

### `_synchronizedAnchor`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Best accepted synchronized reference coordinate at the monotonic anchor.

```cpp
SynchronizedTimestamp _synchronizedAnchor;
```

### `std::int32_t _frequencyCorrectionPartsPerBillion{0};`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Long-term signed frequency correction applied when projecting away from the anchor.

```cpp
std::int32_t _frequencyCorrectionPartsPerBillion{0};
```

### `std::uint32_t _frequencyUncertaintyPartsPerBillion{0U};`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Conservative uncertainty of the frequency estimate in parts per billion.

```cpp
std::uint32_t _frequencyUncertaintyPartsPerBillion{0U};
```

### `_anchorUncertainty`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Conservative synchronization uncertainty at the correlation anchor.

```cpp
SynchronizationUncertainty _anchorUncertainty;
```

### `ClockCorrelationState _state{ClockCorrelationState::Unavailable};`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Indicates whether this snapshot contains an accepted reference mapping.

```cpp
ClockCorrelationState _state{ClockCorrelationState::Unavailable};
```

### `ClockCorrelation`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Creates one available immutable reference mapping.

```cpp
constexpr ClockCorrelation(
            const MonotonicTimestamp& monotonicAnchor,
            const SynchronizedTimestamp& synchronizedAnchor,
            std::int32_t frequencyCorrectionPartsPerBillion,
            std::uint32_t frequencyUncertaintyPartsPerBillion,
            const SynchronizationUncertainty& anchorUncertainty
        ) noexcept :
            _monotonicAnchor(monotonicAnchor),
```

### `ClockCorrelation`

**Classification:** PUBLIC API · source access: `public`

Creates an unavailable correlation containing no accepted synchronized reference mapping.

```cpp
constexpr ClockCorrelation() noexcept = default;
```

### `State`

**Classification:** PUBLIC API · source access: `public`

Returns whether this snapshot contains an accepted synchronized reference mapping.

```cpp
constexpr ClockCorrelationState State() const noexcept
```

### `IsAvailable`

**Classification:** PUBLIC API · source access: `public`

Indicates whether this snapshot can project monotonic occurrences into synchronized time.

```cpp
constexpr bool IsAvailable() const noexcept
```

### `MonotonicAnchor`

**Classification:** PUBLIC API · source access: `public`

Returns the monotonic coordinate at which this reference mapping is anchored.

```cpp
constexpr const MonotonicTimestamp& MonotonicAnchor() const noexcept
```

### `SynchronizedAnchor`

**Classification:** PUBLIC API · source access: `public`

Returns the synchronized reference coordinate at the monotonic anchor.

```cpp
constexpr const SynchronizedTimestamp& SynchronizedAnchor() const noexcept
```

### `FrequencyCorrectionPartsPerBillion`

**Classification:** PUBLIC API · source access: `public`

Returns the long-term signed frequency correction used by this mapping.

```cpp
constexpr std::int32_t FrequencyCorrectionPartsPerBillion() const noexcept
```

### `FrequencyUncertaintyPartsPerBillion`

**Classification:** PUBLIC API · source access: `public`

Returns the conservative uncertainty of the frequency estimate in parts per billion.

```cpp
constexpr std::uint32_t FrequencyUncertaintyPartsPerBillion() const noexcept
```

### `AnchorUncertainty`

**Classification:** PUBLIC API · source access: `public`

Returns the synchronization uncertainty associated with the mapping anchor.

```cpp
constexpr const SynchronizationUncertainty& AnchorUncertainty() const noexcept
```

### `Correlate`

**Classification:** PUBLIC API · source access: `public`

Projects one captured monotonic occurrence into synchronized time.

Projection uses the immutable reference mapping represented by this object, not the
phase-slewed published SynchronizedClock timeline. The supplied capture uncertainty is
added to anchor uncertainty and rate-uncertainty growth. Projection may move forward or
backward from the anchor; synchronized coordinate underflow/overflow is reported rather
than clamped.

- **Parameter `timestamp`:** Monotonic timestamp captured as close as possible to the occurrence.
- **Parameter `captureUncertainty`:** Conservative uncertainty introduced while capturing timestamp.
- **Returns:** Synchronized coordinate estimate, uncertainty and explicit projection status.

```cpp
constexpr ClockCorrelationProjection Correlate(
            const MonotonicTimestamp& timestamp,
            const SynchronizationUncertainty& captureUncertainty
        ) const noexcept
```

### `ClockCorrelationFactory`

**Classification:** PUBLIC API · source access: `public`

Creates available ClockCorrelation values from accepted synchronized-clock state.

```cpp
struct ClockCorrelationFactory final
```

### `Create`

**Classification:** PUBLIC API · source access: `public`

Creates one correlation from a coherent accepted reference mapping.

```cpp
static constexpr ClockCorrelation Create(
                const MonotonicTimestamp& monotonicAnchor,
                const SynchronizedTimestamp& synchronizedAnchor,
                std::int32_t frequencyCorrectionPartsPerBillion,
                std::uint32_t frequencyUncertaintyPartsPerBillion,
                const SynchronizationUncertainty& anchorUncertainty
            ) noexcept
```

