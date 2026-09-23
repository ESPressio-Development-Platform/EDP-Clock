# src/clock/SynchronizationObservation.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `6a9ab9ffab8c90901f48b6c2c70767942fe43380`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/6a9ab9ffab8c90901f48b6c2c70767942fe43380/src/clock/SynchronizationObservation.hpp)

## Direct includes

- `type_traits`
- `MonotonicTimestamp.hpp`
- `SynchronizationUncertainty.hpp`
- `SynchronizedTimestamp.hpp`

## Documented declarations

### `SynchronizationObservation`

**Classification:** PUBLIC API

Source-agnostic observation relating one local monotonic coordinate to synchronized time.

Transport-specific mechanisms reduce their evidence to this type before Clock sees it.
Clock therefore does not know whether an observation originated from Mesh, GNSS, an RTC,
a network exchange, another processor, or any other synchronization mechanism.

```cpp
class SynchronizationObservation final
```

### `_localTimestamp`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Local monotonic coordinate at which the reference estimate applies.

```cpp
MonotonicTimestamp _localTimestamp;
```

### `_referenceTimestamp`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Best available synchronized reference coordinate at the local observation coordinate.

```cpp
SynchronizedTimestamp _referenceTimestamp;
```

### `_uncertainty`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Conservative uncertainty associated with the reference estimate.

```cpp
SynchronizationUncertainty _uncertainty;
```

### `SynchronizationObservation`

**Classification:** PUBLIC API · source access: `public`

Creates one synchronization observation.

- **Parameter `localTimestamp`:** Local monotonic coordinate associated with the observation.
- **Parameter `referenceTimestamp`:** Best synchronized coordinate at the local coordinate.
- **Parameter `uncertainty`:** Conservative uncertainty of the synchronized estimate.

```cpp
constexpr SynchronizationObservation(
            const MonotonicTimestamp& localTimestamp,
            const SynchronizedTimestamp& referenceTimestamp,
            const SynchronizationUncertainty& uncertainty
        ) noexcept :
            _localTimestamp(localTimestamp),
```

### `LocalTimestamp`

**Classification:** PUBLIC API · source access: `public`

Returns the local monotonic coordinate at which this observation applies.

```cpp
constexpr const MonotonicTimestamp& LocalTimestamp() const noexcept
```

### `ReferenceTimestamp`

**Classification:** PUBLIC API · source access: `public`

Returns the synchronized reference coordinate estimated by this observation.

```cpp
constexpr const SynchronizedTimestamp& ReferenceTimestamp() const noexcept
```

### `Uncertainty`

**Classification:** PUBLIC API · source access: `public`

Returns the conservative uncertainty associated with this observation.

```cpp
constexpr const SynchronizationUncertainty& Uncertainty() const noexcept
```

