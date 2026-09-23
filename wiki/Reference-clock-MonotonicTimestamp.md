# src/clock/MonotonicTimestamp.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `6a9ab9ffab8c90901f48b6c2c70767942fe43380`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/6a9ab9ffab8c90901f48b6c2c70767942fe43380/src/clock/MonotonicTimestamp.hpp)

## Direct includes

- `cstdint`
- `type_traits`

## Documented declarations

### `MonotonicTimestamp`

**Classification:** PUBLIC API

Position on the local monotonic clock coordinate system, expressed in nanoseconds.

Zero is the opaque origin of the selected monotonic timebase. It is a valid timestamp
and carries no calendar, civil-time, or other human-interpreted meaning.

```cpp
class MonotonicTimestamp final
```

### `std::uint64_t _nanoseconds{0U};`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Nanoseconds elapsed from the opaque monotonic clock origin.

```cpp
std::uint64_t _nanoseconds{0U};
```

### `MonotonicTimestamp`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Creates a timestamp from an already-normalized nanosecond coordinate.

```cpp
explicit constexpr MonotonicTimestamp(
            std::uint64_t nanoseconds
        ) noexcept :
            _nanoseconds(nanoseconds) {}
```

### `MonotonicTimestamp`

**Classification:** PUBLIC API · source access: `public`

Creates the monotonic timestamp at the selected timebase's opaque origin.

```cpp
constexpr MonotonicTimestamp() noexcept = default;
```

### `FromNanoseconds`

**Classification:** PUBLIC API · source access: `public`

Creates a monotonic timestamp from a nanosecond coordinate.

- **Parameter `nanoseconds`:** Unsigned nanoseconds from the opaque monotonic origin.

```cpp
static constexpr MonotonicTimestamp FromNanoseconds(
            std::uint64_t nanoseconds
        ) noexcept
```

### `Nanoseconds`

**Classification:** PUBLIC API · source access: `public`

Returns this timestamp's coordinate in nanoseconds from the opaque monotonic origin.

```cpp
constexpr std::uint64_t Nanoseconds() const noexcept
```

### `operator`

**Classification:** PUBLIC API · source access: `public`

Indicates whether two timestamps represent the same monotonic coordinate.

```cpp
friend constexpr bool operator ==(
            const MonotonicTimestamp& left,
            const MonotonicTimestamp& right
        ) noexcept
```

### `left`

**Classification:** PUBLIC API · source access: `public`

Indicates whether two timestamps represent different monotonic coordinates.

```cpp
friend constexpr bool operator !=(
            const MonotonicTimestamp& left,
            const MonotonicTimestamp& right
        ) noexcept
```

### `left`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the left timestamp precedes the right timestamp.

```cpp
friend constexpr bool operator <(
            const MonotonicTimestamp& left,
            const MonotonicTimestamp& right
        ) noexcept
```

### `left`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the left timestamp precedes or equals the right timestamp.

```cpp
friend constexpr bool operator <=(
            const MonotonicTimestamp& left,
            const MonotonicTimestamp& right
        ) noexcept
```

### `left`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the left timestamp follows the right timestamp.

```cpp
friend constexpr bool operator >(
            const MonotonicTimestamp& left,
            const MonotonicTimestamp& right
        ) noexcept
```

### `left`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the left timestamp follows or equals the right timestamp.

```cpp
friend constexpr bool operator >=(
            const MonotonicTimestamp& left,
            const MonotonicTimestamp& right
        ) noexcept
```

