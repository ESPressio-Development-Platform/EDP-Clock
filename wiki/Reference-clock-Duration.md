# src/clock/Duration.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `8298f13f210665ca90a2cbeb19de43e45342aa18`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/8298f13f210665ca90a2cbeb19de43e45342aa18/src/clock/Duration.hpp)

## Direct includes

- `cstdint`
- `limits`
- `type_traits`

## Documented declarations

### `Duration`

**Classification:** PUBLIC API

Signed physical duration expressed in the Clock domain's canonical nanosecond unit.

Duration is a pure value type. It owns no resources, performs no allocation, and has
no relationship to calendars, dates, time zones, or other human interpretations of time.

```cpp
class Duration final
```

### `std::int64_t _nanoseconds{0};`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Signed number of nanoseconds represented by this duration.

```cpp
std::int64_t _nanoseconds{0};
```

### `Duration`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Creates a duration from an already-normalized nanosecond value.

```cpp
explicit constexpr Duration(
            std::int64_t nanoseconds
        ) noexcept :
            _nanoseconds(nanoseconds) {}
```

### `Duration`

**Classification:** PUBLIC API · source access: `public`

Creates a zero-length duration.

```cpp
constexpr Duration() noexcept = default;
```

### `FromNanoseconds`

**Classification:** PUBLIC API · source access: `public`

Creates a duration from a signed nanosecond value.

- **Parameter `nanoseconds`:** Signed duration expressed in nanoseconds.

```cpp
static constexpr Duration FromNanoseconds(
            std::int64_t nanoseconds
        ) noexcept
```

### `Minimum`

**Classification:** PUBLIC API · source access: `public`

Returns the smallest duration representable by this type.

```cpp
static constexpr Duration Minimum() noexcept
```

### `Maximum`

**Classification:** PUBLIC API · source access: `public`

Returns the largest duration representable by this type.

```cpp
static constexpr Duration Maximum() noexcept
```

### `Nanoseconds`

**Classification:** PUBLIC API · source access: `public`

Returns this duration expressed in signed nanoseconds.

```cpp
constexpr std::int64_t Nanoseconds() const noexcept
```

### `operator`

**Classification:** PUBLIC API · source access: `public`

Indicates whether two durations represent the same physical interval.

```cpp
friend constexpr bool operator ==(
            const Duration& left,
            const Duration& right
        ) noexcept
```

### `left`

**Classification:** PUBLIC API · source access: `public`

Indicates whether two durations represent different physical intervals.

```cpp
friend constexpr bool operator !=(
            const Duration& left,
            const Duration& right
        ) noexcept
```

### `left`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the left duration is shorter than the right duration.

```cpp
friend constexpr bool operator <(
            const Duration& left,
            const Duration& right
        ) noexcept
```

### `left`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the left duration is shorter than or equal to the right duration.

```cpp
friend constexpr bool operator <=(
            const Duration& left,
            const Duration& right
        ) noexcept
```

### `left`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the left duration is longer than the right duration.

```cpp
friend constexpr bool operator >(
            const Duration& left,
            const Duration& right
        ) noexcept
```

### `left`

**Classification:** PUBLIC API · source access: `public`

Indicates whether the left duration is longer than or equal to the right duration.

```cpp
friend constexpr bool operator >=(
            const Duration& left,
            const Duration& right
        ) noexcept
```

