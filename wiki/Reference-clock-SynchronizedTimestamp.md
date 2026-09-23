# src/clock/SynchronizedTimestamp.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `6a9ab9ffab8c90901f48b6c2c70767942fe43380`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/6a9ab9ffab8c90901f48b6c2c70767942fe43380/src/clock/SynchronizedTimestamp.hpp)

## Direct includes

- `cstdint`
- `type_traits`

## Documented declarations

### `SynchronizedTimestamp`

**Classification:** PUBLIC API

Position on the shared synchronized clock coordinate system, expressed in nanoseconds.

The synchronized epoch is deliberately opaque to EDP-Clock. It is established by the
accepted synchronization reference and carries no calendar, civil-time, or time-zone
meaning. Zero is a valid synchronized coordinate.

```cpp
class SynchronizedTimestamp final
```

### `std::uint64_t _nanoseconds{0U};`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Nanoseconds elapsed from the shared synchronized clock origin.

```cpp
std::uint64_t _nanoseconds{0U};
```

### `SynchronizedTimestamp`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Creates a timestamp from an already-normalized nanosecond coordinate.

```cpp
explicit constexpr SynchronizedTimestamp(
            std::uint64_t nanoseconds
        ) noexcept :
            _nanoseconds(nanoseconds) {}
```

### `SynchronizedTimestamp`

**Classification:** PUBLIC API · source access: `public`

Creates the synchronized timestamp at the shared clock origin.

```cpp
constexpr SynchronizedTimestamp() noexcept = default;
```

### `FromNanoseconds`

**Classification:** PUBLIC API · source access: `public`

Creates a synchronized timestamp from a nanosecond coordinate.

- **Parameter `nanoseconds`:** Unsigned nanoseconds from the shared synchronized origin.

```cpp
static constexpr SynchronizedTimestamp FromNanoseconds(
            std::uint64_t nanoseconds
        ) noexcept
```

### `Nanoseconds`

**Classification:** PUBLIC API · source access: `public`

Returns this timestamp's coordinate in nanoseconds from the synchronized origin.

```cpp
constexpr std::uint64_t Nanoseconds() const noexcept
```

### `operator`

**Classification:** PUBLIC API · source access: `public`

Indicates whether two timestamps represent the same synchronized coordinate.

```cpp
friend constexpr bool operator ==(
            const SynchronizedTimestamp& left,
            const SynchronizedTimestamp& right
        ) noexcept
```

### `left`

**Classification:** PUBLIC API · source access: `public`

Indicates whether two timestamps represent different synchronized coordinates.

```cpp
friend constexpr bool operator !=(
            const SynchronizedTimestamp& left,
            const SynchronizedTimestamp& right
        ) noexcept
```

### `left`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the left timestamp precedes the right timestamp.

```cpp
friend constexpr bool operator <(
            const SynchronizedTimestamp& left,
            const SynchronizedTimestamp& right
        ) noexcept
```

### `left`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the left timestamp precedes or equals the right timestamp.

```cpp
friend constexpr bool operator <=(
            const SynchronizedTimestamp& left,
            const SynchronizedTimestamp& right
        ) noexcept
```

### `left`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the left timestamp follows the right timestamp.

```cpp
friend constexpr bool operator >(
            const SynchronizedTimestamp& left,
            const SynchronizedTimestamp& right
        ) noexcept
```

### `left`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the left timestamp follows or equals the right timestamp.

```cpp
friend constexpr bool operator >=(
            const SynchronizedTimestamp& left,
            const SynchronizedTimestamp& right
        ) noexcept
```

