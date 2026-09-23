# src/clock/FourTimestampExchangeResult.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `8298f13f210665ca90a2cbeb19de43e45342aa18`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/8298f13f210665ca90a2cbeb19de43e45342aa18/src/clock/FourTimestampExchangeResult.hpp)

## Direct includes

- `cstdint`
- `type_traits`
- `FourTimestampExchange.hpp`
- `FourTimestampExchangeStatus.hpp`
- `SynchronizationObservation.hpp`

## Documented declarations

### `EstimateFourTimestampExchange`

**Classification:** PUBLIC API

Reduces a four-timestamp exchange into one source-agnostic synchronization observation.

```cpp
constexpr FourTimestampExchangeResult EstimateFourTimestampExchange(
        const FourTimestampExchange& exchange
    ) noexcept;
```

### `FourTimestampExchangeResult`

**Classification:** PUBLIC API

Result of reducing one four-timestamp time-transfer exchange into a synchronization observation.

```cpp
class FourTimestampExchangeResult final
```

### `_observation`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Observation produced for Clock discipline when the reduction succeeds.

```cpp
SynchronizationObservation _observation;
```

### `_roundTripPathDelayNanoseconds`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Estimated aggregate transport path delay excluding remote turnaround time.

```cpp
std::uint64_t _roundTripPathDelayNanoseconds;
```

### `_status`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Admission status produced by the four-timestamp reduction.

```cpp
FourTimestampExchangeStatus _status;
```

### `FourTimestampExchangeResult`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Creates one reduction result from fully prepared values.

```cpp
constexpr FourTimestampExchangeResult(
            const SynchronizationObservation& observation,
            std::uint64_t roundTripPathDelayNanoseconds,
            FourTimestampExchangeStatus status
        ) noexcept :
            _observation(observation),
```

### `EstimateFourTimestampExchange`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Allows only the Clock-owned estimator to construct reduction results.

```cpp
friend constexpr FourTimestampExchangeResult EstimateFourTimestampExchange(
            const FourTimestampExchange& exchange
        ) noexcept;
```

### `IsAccepted`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the exchange was reduced successfully.

```cpp
constexpr bool IsAccepted() const noexcept
```

### `Status`

**Classification:** PUBLIC API · source access: `public`

Returns the reduction status.

```cpp
constexpr FourTimestampExchangeStatus Status() const noexcept
```

### `Observation`

**Classification:** PUBLIC API · source access: `public`

Returns the derived synchronization observation.

Consumers must inspect IsAccepted() or Status() before submitting this value to Clock
discipline. Rejected results expose a saturated-uncertainty sentinel observation only so
the result remains a compact allocation-free value type without optional storage.

```cpp
constexpr const SynchronizationObservation& Observation() const noexcept
```

### `RoundTripPathDelayNanoseconds`

**Classification:** PUBLIC API · source access: `public`

Returns the estimated aggregate path round-trip delay excluding remote turnaround time.

```cpp
constexpr std::uint64_t RoundTripPathDelayNanoseconds() const noexcept
```

