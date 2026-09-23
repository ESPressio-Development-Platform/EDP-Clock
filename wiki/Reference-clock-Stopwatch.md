# src/clock/Stopwatch.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `6a9ab9ffab8c90901f48b6c2c70767942fe43380`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/6a9ab9ffab8c90901f48b6c2c70767942fe43380/src/clock/Stopwatch.hpp)

## Direct includes

- `cstdint`
- `limits`
- `type_traits`
- `utility`
- `ClockComposition.hpp`
- `Duration.hpp`
- `MonotonicTimestamp.hpp`

## Documented declarations

### `TMonotonicClockProvider`

**Classification:** PUBLIC API

Validates the MonotonicClock provider consumed by Stopwatch.

```cpp
template<class TMonotonicClockProvider>
        struct StopwatchMonotonicClockTraits
```

### `NowResult`

**Classification:** PUBLIC API · source access: `public`

Return type produced by the selected provider's Now operation.

```cpp
using NowResult = decltype(
                std::declval<const TMonotonicClockProvider&>().Now()
            );
```

### `IsValid`

**Classification:** PUBLIC API · source access: `public`

Indicates that all compile-time Stopwatch provider requirements are satisfied.

```cpp
static constexpr bool IsValid = true;
```

### `max`

**Classification:** PUBLIC API

Largest non-negative duration representable through Duration.

```cpp
inline constexpr std::uint64_t MaximumStopwatchNanoseconds =
            static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());
```

### `SaturatingStopwatchAdd`

**Classification:** PUBLIC API

Adds elapsed nanoseconds and saturates at Duration::Maximum().

```cpp
constexpr std::uint64_t SaturatingStopwatchAdd(
            std::uint64_t accumulatedNanoseconds,
            std::uint64_t additionalNanoseconds
        ) noexcept
```

### `StopwatchIntervalNanoseconds`

**Classification:** PUBLIC API

Returns a non-negative monotonic interval without permitting unsigned underflow.

```cpp
constexpr std::uint64_t StopwatchIntervalNanoseconds(
            const MonotonicTimestamp& started,
            const MonotonicTimestamp& finished
        ) noexcept
```

### `TMonotonicClockProvider`

**Classification:** PUBLIC API

Small stateful elapsed-time utility driven exclusively by one MonotonicClock provider.

Stopwatch owns no clock, task, timer, synchronization primitive, heap allocation, callback,
or history. It retains only a borrowed provider pointer and fixed local measurement state.
Synchronization discipline, Clock Correlation, RTCs, and civil time cannot affect it.

- **Template parameter `TMonotonicClockProvider`:** Concrete MonotonicClock provider selected by application Bootstrap.

```cpp
template<class TMonotonicClockProvider>
    class Stopwatch final
```

### `ProviderTraits`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Compile-time validation of the supplied MonotonicClock provider.

```cpp
using ProviderTraits = Detail::StopwatchMonotonicClockTraits<TMonotonicClockProvider>;
```

### `MonotonicTimestamp _runningSince{};`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Monotonic coordinate captured by the most recent successful Start() or Restart().

```cpp
MonotonicTimestamp _runningSince{};
```

### `std::uint64_t _accumulatedNanoseconds{0U};`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Elapsed nanoseconds accumulated across completed running intervals.

```cpp
std::uint64_t _accumulatedNanoseconds{0U};
```

### `_clock`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Borrowed lifetime-stable MonotonicClock provider.

```cpp
const TMonotonicClockProvider* _clock;
```

### `std::uint8_t _running{0U};`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Indicates whether the stopwatch currently accumulates elapsed time.

```cpp
std::uint8_t _running{0U};
```

### `ElapsedNanosecondsAt`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Calculates elapsed nanoseconds against one already-captured current timestamp.

```cpp
std::uint64_t ElapsedNanosecondsAt(
            const MonotonicTimestamp& now
        ) const noexcept
```

### `ResetMovedFromState`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Resets one moved-from Stopwatch to a valid stopped state while retaining its provider binding.

```cpp
void ResetMovedFromState() noexcept
```

### `Stopwatch`

**Classification:** PUBLIC API · source access: `public`

Creates a stopped zero-duration Stopwatch over the supplied MonotonicClock provider.

- **Parameter `clock`:** Lifetime-stable MonotonicClock provider used for every measurement.

```cpp
explicit Stopwatch(
            const TMonotonicClockProvider& clock
        ) noexcept :
            _clock(&clock) {}
```

### `Stopwatch`

**Classification:** PUBLIC API · source access: `public`

Prevents accidental duplication of one mutable measurement session.

```cpp
Stopwatch(const Stopwatch&) = delete;
```

### `operator`

**Classification:** PUBLIC API · source access: `public`

Prevents accidental copy replacement of one mutable measurement session.

```cpp
Stopwatch& operator =(const Stopwatch&) = delete;
```

### `Stopwatch`

**Classification:** PUBLIC API · source access: `public`

Transfers one measurement session and resets the moved-from Stopwatch to stopped zero state.

```cpp
Stopwatch(
            Stopwatch&& other
        ) noexcept :
            _runningSince(other._runningSince),
```

### `operator`

**Classification:** PUBLIC API · source access: `public`

Transfers one measurement session and resets the moved-from Stopwatch to stopped zero state.

```cpp
Stopwatch& operator =(
            Stopwatch&& other
        ) noexcept
```

### `Start`

**Classification:** PUBLIC API · source access: `public`

Starts or resumes elapsed-time accumulation without clearing already accumulated time.

Calling Start() while already running is a no-op and does not read the Clock.

```cpp
void Start() noexcept
```

### `Stop`

**Classification:** PUBLIC API · source access: `public`

Stops elapsed-time accumulation and freezes the measured duration.

Calling Stop() while already stopped is a no-op and does not read the Clock.

```cpp
void Stop() noexcept
```

### `Reset`

**Classification:** PUBLIC API · source access: `public`

Clears all elapsed time and leaves the Stopwatch stopped without reading the Clock.

```cpp
void Reset() noexcept
```

### `Restart`

**Classification:** PUBLIC API · source access: `public`

Clears all elapsed time and immediately starts a fresh measurement interval.

```cpp
void Restart() noexcept
```

### `Elapsed`

**Classification:** PUBLIC API · source access: `public`

Returns the current measured elapsed duration.

A running Stopwatch reads the MonotonicClock exactly once. A stopped Stopwatch performs
no Clock read. Elapsed time saturates at Duration::Maximum() rather than wrapping.

```cpp
Duration Elapsed() const noexcept
```

### `IsRunning`

**Classification:** PUBLIC API · source access: `public`

Indicates whether elapsed-time accumulation is currently active.

```cpp
bool IsRunning() const noexcept
```

