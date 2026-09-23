# src/clock/ClockCorrelationProjection.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `8298f13f210665ca90a2cbeb19de43e45342aa18`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/8298f13f210665ca90a2cbeb19de43e45342aa18/src/clock/ClockCorrelationProjection.hpp)

## Direct includes

- `cstdint`
- `type_traits`
- `ClockCorrelationProjectionStatus.hpp`
- `SynchronizationUncertainty.hpp`
- `SynchronizedTimestamp.hpp`

## Documented declarations

### `ClockCorrelationProjection`

**Classification:** PUBLIC API

Result of projecting one monotonic occurrence into synchronized time through a correlation.

```cpp
class ClockCorrelationProjection final
```

### `_timestamp`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Best synchronized coordinate estimate for the projected monotonic occurrence.

```cpp
SynchronizedTimestamp _timestamp;
```

### `_uncertainty`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Conservative uncertainty bound associated with the synchronized coordinate estimate.

```cpp
SynchronizationUncertainty _uncertainty;
```

### `ClockCorrelationProjectionStatus _status{ClockCorrelationProjectionSta`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Status describing whether the projection produced a usable synchronized coordinate.

```cpp
ClockCorrelationProjectionStatus _status{ClockCorrelationProjectionStatus::CorrelationUnavailable};
```

### `ClockCorrelationProjection`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Creates one projection result from already-validated values.

```cpp
constexpr ClockCorrelationProjection(
            const SynchronizedTimestamp& timestamp,
            const SynchronizationUncertainty& uncertainty,
            ClockCorrelationProjectionStatus status
        ) noexcept :
            _timestamp(timestamp),
```

### `ClockCorrelationProjection`

**Classification:** PUBLIC API · source access: `public`

Creates an unavailable projection result.

```cpp
constexpr ClockCorrelationProjection() noexcept = default;
```

### `Timestamp`

**Classification:** PUBLIC API · source access: `public`

Returns the synchronized coordinate estimate carried by this projection.

```cpp
constexpr const SynchronizedTimestamp& Timestamp() const noexcept
```

### `Uncertainty`

**Classification:** PUBLIC API · source access: `public`

Returns the conservative uncertainty bound associated with this projection.

```cpp
constexpr const SynchronizationUncertainty& Uncertainty() const noexcept
```

### `Status`

**Classification:** PUBLIC API · source access: `public`

Returns the status describing whether this projection succeeded.

```cpp
constexpr ClockCorrelationProjectionStatus Status() const noexcept
```

### `IsCorrelated`

**Classification:** PUBLIC API · source access: `public`

Indicates whether this result contains a successfully correlated synchronized coordinate.

```cpp
constexpr bool IsCorrelated() const noexcept
```

