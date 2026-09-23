# src/clock/FourTimestampExchange.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `8298f13f210665ca90a2cbeb19de43e45342aa18`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/8298f13f210665ca90a2cbeb19de43e45342aa18/src/clock/FourTimestampExchange.hpp)

## Direct includes

- `type_traits`
- `MonotonicTimestamp.hpp`
- `SynchronizationUncertainty.hpp`
- `SynchronizedTimestamp.hpp`

## Documented declarations

### `FourTimestampExchange`

**Classification:** PUBLIC API

Source-agnostic four-timestamp exchange used to estimate synchronized time at a local monotonic coordinate.

T1 and T4 belong to the requesting system's local MonotonicClock. T2 and T3 belong to the
responding reference system's SynchronizedClock. Transport code owns timestamp capture and
carriage; EDP-Clock owns the generic reduction mathematics represented by this value.

```cpp
class FourTimestampExchange final
```

### `_localTransmitTimestamp`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

T1: local monotonic coordinate captured when the request leaves the local system.

```cpp
MonotonicTimestamp _localTransmitTimestamp;
```

### `_remoteReceiveTimestamp`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

T2: reference synchronized coordinate captured when the request reaches the remote system.

```cpp
SynchronizedTimestamp _remoteReceiveTimestamp;
```

### `_remoteTransmitTimestamp`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

T3: reference synchronized coordinate captured when the response leaves the remote system.

```cpp
SynchronizedTimestamp _remoteTransmitTimestamp;
```

### `_localReceiveTimestamp`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

T4: local monotonic coordinate captured when the response reaches the local system.

```cpp
MonotonicTimestamp _localReceiveTimestamp;
```

### `_localTransmitUncertainty`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Conservative uncertainty associated with the T1 capture.

```cpp
SynchronizationUncertainty _localTransmitUncertainty;
```

### `_remoteReceiveUncertainty`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Conservative uncertainty associated with the T2 synchronized timestamp and capture.

```cpp
SynchronizationUncertainty _remoteReceiveUncertainty;
```

### `_remoteTransmitUncertainty`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Conservative uncertainty associated with the T3 synchronized timestamp and capture.

```cpp
SynchronizationUncertainty _remoteTransmitUncertainty;
```

### `_localReceiveUncertainty`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Conservative uncertainty associated with the T4 capture.

```cpp
SynchronizationUncertainty _localReceiveUncertainty;
```

### `FourTimestampExchange`

**Classification:** PUBLIC API · source access: `public`

Creates one complete four-timestamp exchange.

- **Parameter `localTransmitTimestamp`:** T1 local monotonic request-transmit coordinate.
- **Parameter `localTransmitUncertainty`:** Conservative T1 capture uncertainty.
- **Parameter `remoteReceiveTimestamp`:** T2 remote/reference synchronized receive coordinate.
- **Parameter `remoteReceiveUncertainty`:** Conservative T2 reference/capture uncertainty.
- **Parameter `remoteTransmitTimestamp`:** T3 remote/reference synchronized transmit coordinate.
- **Parameter `remoteTransmitUncertainty`:** Conservative T3 reference/capture uncertainty.
- **Parameter `localReceiveTimestamp`:** T4 local monotonic response-receive coordinate.
- **Parameter `localReceiveUncertainty`:** Conservative T4 capture uncertainty.

```cpp
constexpr FourTimestampExchange(
            const MonotonicTimestamp& localTransmitTimestamp,
            const SynchronizationUncertainty& localTransmitUncertainty,
            const SynchronizedTimestamp& remoteReceiveTimestamp,
            const SynchronizationUncertainty& remoteReceiveUncertainty,
            const SynchronizedTimestamp& remoteTransmitTimestamp,
            const SynchronizationUncertainty& remoteTransmitUncertainty,
            const MonotonicTimestamp& localReceiveTimestamp,
            const SynchronizationUncertainty& localReceiveUncertainty
        ) noexcept :
            _localTransmitTimestamp(localTransmitTimestamp),
```

### `LocalTransmitTimestamp`

**Classification:** PUBLIC API · source access: `public`

Returns T1, the local monotonic request-transmit coordinate.

```cpp
constexpr const MonotonicTimestamp& LocalTransmitTimestamp() const noexcept
```

### `RemoteReceiveTimestamp`

**Classification:** PUBLIC API · source access: `public`

Returns T2, the remote/reference synchronized receive coordinate.

```cpp
constexpr const SynchronizedTimestamp& RemoteReceiveTimestamp() const noexcept
```

### `RemoteTransmitTimestamp`

**Classification:** PUBLIC API · source access: `public`

Returns T3, the remote/reference synchronized transmit coordinate.

```cpp
constexpr const SynchronizedTimestamp& RemoteTransmitTimestamp() const noexcept
```

### `LocalReceiveTimestamp`

**Classification:** PUBLIC API · source access: `public`

Returns T4, the local monotonic response-receive coordinate.

```cpp
constexpr const MonotonicTimestamp& LocalReceiveTimestamp() const noexcept
```

### `LocalTransmitUncertainty`

**Classification:** PUBLIC API · source access: `public`

Returns the conservative T1 capture uncertainty.

```cpp
constexpr const SynchronizationUncertainty& LocalTransmitUncertainty() const noexcept
```

### `RemoteReceiveUncertainty`

**Classification:** PUBLIC API · source access: `public`

Returns the conservative T2 reference/capture uncertainty.

```cpp
constexpr const SynchronizationUncertainty& RemoteReceiveUncertainty() const noexcept
```

### `RemoteTransmitUncertainty`

**Classification:** PUBLIC API · source access: `public`

Returns the conservative T3 reference/capture uncertainty.

```cpp
constexpr const SynchronizationUncertainty& RemoteTransmitUncertainty() const noexcept
```

### `LocalReceiveUncertainty`

**Classification:** PUBLIC API · source access: `public`

Returns the conservative T4 capture uncertainty.

```cpp
constexpr const SynchronizationUncertainty& LocalReceiveUncertainty() const noexcept
```

