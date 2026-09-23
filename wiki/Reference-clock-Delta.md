# src/clock/Delta.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `8298f13f210665ca90a2cbeb19de43e45342aa18`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/8298f13f210665ca90a2cbeb19de43e45342aa18/src/clock/Delta.hpp)

## Direct includes

- `cstdint`
- `limits`
- `Duration.hpp`
- `MonotonicTimestamp.hpp`
- `SynchronizedTimestamp.hpp`

## Documented declarations

### `Delta`

**Classification:** PUBLIC API

Calculates the signed physical duration from one monotonic timestamp to another.

The normal Clock contract expects the requested difference to fit within Duration's
signed 64-bit nanosecond range. If a caller nevertheless asks for a difference beyond
that representable range, the result is clamped to the nearest Duration boundary rather
than allowing integer overflow.

- **Parameter `from`:** Starting monotonic coordinate.
- **Parameter `to`:** Ending monotonic coordinate.
- **Returns:** Signed duration equal to `to - from`.

```cpp
constexpr Duration Delta(
        const MonotonicTimestamp& from,
        const MonotonicTimestamp& to
    ) noexcept
```

### `Delta`

**Classification:** PUBLIC API

Calculates the signed physical duration from one synchronized timestamp to another.

Cross-domain arithmetic is intentionally unavailable: monotonic and synchronized
timestamps cannot be passed to the same Delta overload.

- **Parameter `from`:** Starting synchronized coordinate.
- **Parameter `to`:** Ending synchronized coordinate.
- **Returns:** Signed duration equal to `to - from`.

```cpp
constexpr Duration Delta(
        const SynchronizedTimestamp& from,
        const SynchronizedTimestamp& to
    ) noexcept
```

