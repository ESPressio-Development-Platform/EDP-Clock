# src/clock/SynchronizationUncertainty.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `6a9ab9ffab8c90901f48b6c2c70767942fe43380`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/6a9ab9ffab8c90901f48b6c2c70767942fe43380/src/clock/SynchronizationUncertainty.hpp)

## Direct includes

- `cstdint`
- `limits`
- `type_traits`

## Documented declarations

### `SynchronizationUncertainty`

**Classification:** PUBLIC API

Conservative non-negative uncertainty associated with synchronized time, in nanoseconds.

The 32-bit representation intentionally saturates at approximately 4.295 seconds. Saturation
means the synchronized mapping is no longer sufficiently bounded to remain meaningful for
the Clock domain's quality reporting.

```cpp
class SynchronizationUncertainty final
```

### `max`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Conservative uncertainty bound expressed in nanoseconds.

```cpp
std::uint32_t _nanoseconds{std::numeric_limits<std::uint32_t>::max()};
```

### `SynchronizationUncertainty`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Creates uncertainty from an already-saturated 32-bit nanosecond value.

```cpp
explicit constexpr SynchronizationUncertainty(
            std::uint32_t nanoseconds
        ) noexcept :
            _nanoseconds(nanoseconds) {}
```

### `SynchronizationUncertainty`

**Classification:** PUBLIC API · source access: `public`

Creates an unknown/saturated uncertainty value.

```cpp
constexpr SynchronizationUncertainty() noexcept = default;
```

### `FromNanoseconds`

**Classification:** PUBLIC API · source access: `public`

Creates uncertainty from a nanosecond value, saturating when it exceeds the 32-bit range.

- **Parameter `nanoseconds`:** Conservative uncertainty bound in nanoseconds.

```cpp
static constexpr SynchronizationUncertainty FromNanoseconds(
            std::uint64_t nanoseconds
        ) noexcept
```

### `Zero`

**Classification:** PUBLIC API · source access: `public`

Returns an exact zero-uncertainty value.

```cpp
static constexpr SynchronizationUncertainty Zero() noexcept
```

### `Maximum`

**Classification:** PUBLIC API · source access: `public`

Returns the saturated uncertainty value.

```cpp
static constexpr SynchronizationUncertainty Maximum() noexcept
```

### `Nanoseconds`

**Classification:** PUBLIC API · source access: `public`

Returns the conservative uncertainty bound in nanoseconds.

```cpp
constexpr std::uint32_t Nanoseconds() const noexcept
```

### `IsSaturated`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the uncertainty has saturated at its maximum representable value.

```cpp
constexpr bool IsSaturated() const noexcept
```

### `operator`

**Classification:** PUBLIC API · source access: `public`

Indicates whether two uncertainty values are identical.

```cpp
friend constexpr bool operator ==(
            const SynchronizationUncertainty& left,
            const SynchronizationUncertainty& right
        ) noexcept
```

### `left`

**Classification:** PUBLIC API · source access: `public`

Indicates whether two uncertainty values differ.

```cpp
friend constexpr bool operator !=(
            const SynchronizationUncertainty& left,
            const SynchronizationUncertainty& right
        ) noexcept
```

### `left`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the left uncertainty is smaller than the right uncertainty.

```cpp
friend constexpr bool operator <(
            const SynchronizationUncertainty& left,
            const SynchronizationUncertainty& right
        ) noexcept
```

### `left`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the left uncertainty is smaller than or equal to the right uncertainty.

```cpp
friend constexpr bool operator <=(
            const SynchronizationUncertainty& left,
            const SynchronizationUncertainty& right
        ) noexcept
```

### `left`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the left uncertainty is greater than the right uncertainty.

```cpp
friend constexpr bool operator >(
            const SynchronizationUncertainty& left,
            const SynchronizationUncertainty& right
        ) noexcept
```

### `left`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the left uncertainty is greater than or equal to the right uncertainty.

```cpp
friend constexpr bool operator >=(
            const SynchronizationUncertainty& left,
            const SynchronizationUncertainty& right
        ) noexcept
```

