# src/clock/SynchronizedReading.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `6a9ab9ffab8c90901f48b6c2c70767942fe43380`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/6a9ab9ffab8c90901f48b6c2c70767942fe43380/src/clock/SynchronizedReading.hpp)

## Direct includes

- `type_traits`
- `SynchronizationState.hpp`
- `SynchronizationUncertainty.hpp`
- `SynchronizedTimestamp.hpp`

## Documented declarations

### `SynchronizedReading`

**Classification:** PUBLIC API

One synchronized clock observation with the quality information valid for that observation.

```cpp
class SynchronizedReading final
```

### `_timestamp`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Synchronized coordinate associated with this reading.

```cpp
SynchronizedTimestamp _timestamp;
```

### `_uncertainty`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Conservative uncertainty bound associated with this reading.

```cpp
SynchronizationUncertainty _uncertainty;
```

### `_state`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Synchronization lifecycle/quality state associated with this reading.

```cpp
SynchronizationState _state;
```

### `SynchronizedReading`

**Classification:** PUBLIC API · source access: `public`

Creates a synchronized reading from one coherent timestamp, uncertainty and state.

- **Parameter `timestamp`:** Synchronized coordinate represented by the reading.
- **Parameter `uncertainty`:** Conservative uncertainty bound associated with the coordinate.
- **Parameter `state`:** Synchronization lifecycle/quality state for the coordinate.

```cpp
constexpr SynchronizedReading(
            const SynchronizedTimestamp& timestamp,
            const SynchronizationUncertainty& uncertainty,
            SynchronizationState state
        ) noexcept :
            _timestamp(timestamp),
```

### `Timestamp`

**Classification:** PUBLIC API · source access: `public`

Returns the synchronized coordinate represented by this reading.

```cpp
constexpr const SynchronizedTimestamp& Timestamp() const noexcept
```

### `Uncertainty`

**Classification:** PUBLIC API · source access: `public`

Returns the conservative uncertainty bound associated with this reading.

```cpp
constexpr const SynchronizationUncertainty& Uncertainty() const noexcept
```

### `State`

**Classification:** PUBLIC API · source access: `public`

Returns the synchronization lifecycle/quality state associated with this reading.

```cpp
constexpr SynchronizationState State() const noexcept
```

