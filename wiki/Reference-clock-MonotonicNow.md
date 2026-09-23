# src/clock/MonotonicNow.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `8298f13f210665ca90a2cbeb19de43e45342aa18`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/8298f13f210665ca90a2cbeb19de43e45342aa18/src/clock/MonotonicNow.hpp)

## Direct includes

- `type_traits`
- `utility`
- `MonotonicTimestamp.hpp`

## Documented declarations

### `MonotonicClockBinding`

**Classification:** PUBLIC API

Process-lifetime binding used by the MonotonicNow convenience function.

```cpp
struct MonotonicClockBinding final
```

### `const void* Context{nullptr};`

**Classification:** PUBLIC API · source access: `public`

Borrowed address of the application-selected monotonic clock.

```cpp
const void* Context{nullptr};
```

### `MonotonicTimestamp`

**Classification:** PUBLIC API · source access: `public`

Type-erased read operation for the application-selected monotonic clock.

```cpp
MonotonicTimestamp (*Read)(const void*) noexcept{nullptr};
```

### `ApplicationMonotonicClock`

**Classification:** PUBLIC API

Single application-wide monotonic clock binding established during Bootstrap.

```cpp
inline MonotonicClockBinding ApplicationMonotonicClock;
```

### `TClock`

**Classification:** PUBLIC API

Invokes the concrete bound clock without requiring runtime inheritance or a vtable.

- **Template parameter `TClock`:** Concrete clock type bound by Bootstrap.
- **Parameter `context`:** Borrowed concrete clock address.
- **Returns:** Current monotonic timestamp.

```cpp
template<class TClock>
        MonotonicTimestamp ReadBoundMonotonicClock(
            const void* context
        ) noexcept
```

### `TClock`

**Classification:** PUBLIC API

Permanently binds the application-wide MonotonicNow convenience call to one clock instance.

Binding is a Bootstrap operation and must complete before any concurrently executing code
can call MonotonicNow(). The binding is intentionally irreversible for the running lifetime.

- **Template parameter `TClock`:** Concrete monotonic clock type.
- **Parameter `clock`:** Lifetime-stable monotonic clock selected by Bootstrap.
- **Returns:** true when the clock was bound; false when a clock was already bound.

```cpp
template<class TClock>
    bool BindMonotonicClock(
        const TClock& clock
    ) noexcept
```

### `IsMonotonicClockBound`

**Classification:** PUBLIC API

Indicates whether application Bootstrap has permanently bound MonotonicNow().

```cpp
inline bool IsMonotonicClockBound() noexcept
```

### `MonotonicNow`

**Classification:** PUBLIC API

Returns the current application-wide monotonic timestamp.

Application Bootstrap must bind a lifetime-stable clock through BindMonotonicClock() before
this function is called. No availability branch is placed on this high-frequency read path;
calling MonotonicNow() before successful Bootstrap binding is therefore a lifecycle error.

```cpp
inline MonotonicTimestamp MonotonicNow() noexcept
```

