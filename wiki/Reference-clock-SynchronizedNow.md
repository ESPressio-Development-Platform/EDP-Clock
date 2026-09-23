# src/clock/SynchronizedNow.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `6a9ab9ffab8c90901f48b6c2c70767942fe43380`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/6a9ab9ffab8c90901f48b6c2c70767942fe43380/src/clock/SynchronizedNow.hpp)

## Direct includes

- `type_traits`
- `utility`
- `SynchronizedReading.hpp`

## Documented declarations

### `SynchronizedClockBinding`

**Classification:** PUBLIC API

Process-lifetime binding used by the SynchronizedNow convenience function.

```cpp
struct SynchronizedClockBinding final
```

### `const void* Context{nullptr};`

**Classification:** PUBLIC API · source access: `public`

Borrowed address of the application-selected synchronized clock.

```cpp
const void* Context{nullptr};
```

### `SynchronizedReading`

**Classification:** PUBLIC API · source access: `public`

Type-erased read operation for the application-selected synchronized clock.

```cpp
SynchronizedReading (*Read)(const void*) noexcept{nullptr};
```

### `ApplicationSynchronizedClock`

**Classification:** PUBLIC API

Single application-wide synchronized clock binding established during Bootstrap.

```cpp
inline SynchronizedClockBinding ApplicationSynchronizedClock;
```

### `TClock`

**Classification:** PUBLIC API

Invokes the concrete bound synchronized clock without runtime inheritance or a vtable.

- **Template parameter `TClock`:** Concrete synchronized clock type bound by Bootstrap.
- **Parameter `context`:** Borrowed concrete clock address.
- **Returns:** Current synchronized reading and quality information.

```cpp
template<class TClock>
        SynchronizedReading ReadBoundSynchronizedClock(
            const void* context
        ) noexcept
```

### `TClock`

**Classification:** PUBLIC API

Permanently binds the application-wide SynchronizedNow convenience call to one clock instance.

Binding is a Bootstrap operation and must complete before concurrently executing code can
call SynchronizedNow(). The binding is intentionally irreversible for the running lifetime.

- **Template parameter `TClock`:** Concrete synchronized clock type.
- **Parameter `clock`:** Lifetime-stable synchronized clock selected by Bootstrap.
- **Returns:** true when the clock was bound; false when a clock was already bound.

```cpp
template<class TClock>
    bool BindSynchronizedClock(
        const TClock& clock
    ) noexcept
```

### `IsSynchronizedClockBound`

**Classification:** PUBLIC API

Indicates whether application Bootstrap has permanently bound SynchronizedNow().

```cpp
inline bool IsSynchronizedClockBound() noexcept
```

### `SynchronizedNow`

**Classification:** PUBLIC API

Returns the current application-wide synchronized reading and quality information.

Application Bootstrap must bind a lifetime-stable clock through BindSynchronizedClock()
before this function is called. Calling it before successful binding is a lifecycle error.

```cpp
inline SynchronizedReading SynchronizedNow() noexcept
```

