# src/clock/MonotonicClockProvider.hpp

**Primary classification:** PUBLIC PROVIDER / EXTENSION API

**Source baseline:** `8298f13f210665ca90a2cbeb19de43e45342aa18`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/8298f13f210665ca90a2cbeb19de43e45342aa18/src/clock/MonotonicClockProvider.hpp)

## Direct includes

- `cstdint`
- `limits`
- `type_traits`
- `utility`
- `ClockComposition.hpp`
- `MonotonicTimestamp.hpp`

## Documented declarations

### `TTimebaseProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Extracts and validates the exact frequency advertised by a MonotonicTimebase provider.

```cpp
template<class TTimebaseProvider>
        struct MonotonicTimebaseTraits
```

### `Properties`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Compile-time property set advertised by the selected timebase provider.

```cpp
using Properties =
                typename TTimebaseProvider::CompositionOffers::template PropertiesFor<MonotonicTimebase>;
```

### `FrequencyNumerator`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Exact frequency numerator advertised by the selected timebase.

```cpp
static constexpr std::uint32_t FrequencyNumerator =
                Properties::template Value<TimebaseFrequencyNumerator>;
```

### `FrequencyDenominator`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Exact frequency denominator advertised by the selected timebase.

```cpp
static constexpr std::uint32_t FrequencyDenominator =
                Properties::template Value<TimebaseFrequencyDenominator>;
```

### `CountResult`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Return type produced by the selected provider's CurrentCount operation.

```cpp
using CountResult = decltype(
                std::declval<const TTimebaseProvider&>().CurrentCount()
            );
```

### `NanosecondScale`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Number of canonical nanoseconds represented by the timebase frequency denominator.

```cpp
static constexpr std::uint64_t NanosecondScale =
                1000000000ULL * static_cast<std::uint64_t>(FrequencyDenominator);
```

### `ResolutionNanoseconds`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Whole-nanosecond resolution conservatively advertised to clock consumers.

```cpp
static constexpr std::uint64_t ResolutionNanoseconds =
                (NanosecondScale / static_cast<std::uint64_t>(FrequencyNumerator)) +
                (
                    (NanosecondScale % static_cast<std::uint64_t>(FrequencyNumerator)) == 0U
                        ? 0U
                        : 1U
                );
```

### `TTimebaseProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Converts an absolute timebase count to canonical nanoseconds without cumulative rounding.

```cpp
template<class TTimebaseProvider>
        constexpr std::uint64_t ConvertCountToNanoseconds(
            std::uint64_t count
        ) noexcept
```

### `TTimebaseProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Generic MonotonicClock provider backed by one application-selected MonotonicTimebase provider.

The provider retains only a borrowed pointer to the lifetime-stable timebase. It owns no
timer hardware, thread primitive, heap allocation, worker, callback registry, or mutable
timing state. Thread safety of physical count acquisition is the timebase provider's contract.

- **Template parameter `TTimebaseProvider`:** Concrete provider selected by application Bootstrap.

```cpp
template<class TTimebaseProvider>
    class MonotonicClockProvider final : public Framework::Provider<
        Domain,
```

### `_timebase`

**Classification:** PRIVATE IMPLEMENTATION · source access: `private`

Borrowed lifetime-stable timebase selected by application Bootstrap.

```cpp
const TTimebaseProvider* _timebase;
```

### `MonotonicClockProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Creates the generic monotonic clock over the selected timebase provider.

- **Parameter `timebase`:** Lifetime-stable concrete timebase provider.

```cpp
explicit constexpr MonotonicClockProvider(
            const TTimebaseProvider& timebase
        ) noexcept :
            _timebase(&timebase) {}
```

### `MonotonicClockProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Prevents duplicating a lifetime clock provider accidentally.

```cpp
MonotonicClockProvider(const MonotonicClockProvider&) = delete;
```

### `operator`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Prevents replacing a lifetime clock provider by copy assignment.

```cpp
MonotonicClockProvider& operator =(const MonotonicClockProvider&) = delete;
```

### `MonotonicClockProvider`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Prevents relocation after other facilities have borrowed this clock.

```cpp
MonotonicClockProvider(MonotonicClockProvider&&) = delete;
```

### `operator`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Prevents relocation by move assignment.

```cpp
MonotonicClockProvider& operator =(MonotonicClockProvider&&) = delete;
```

### `FrequencyNumerator`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Exact numerator of the selected timebase count frequency.

```cpp
static constexpr std::uint32_t FrequencyNumerator =
            Detail::MonotonicTimebaseTraits<TTimebaseProvider>::FrequencyNumerator;
```

### `FrequencyDenominator`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Exact denominator of the selected timebase count frequency.

```cpp
static constexpr std::uint32_t FrequencyDenominator =
            Detail::MonotonicTimebaseTraits<TTimebaseProvider>::FrequencyDenominator;
```

### `ResolutionNanoseconds`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Conservative whole-nanosecond resolution advertised by this monotonic clock.

```cpp
static constexpr std::uint64_t ResolutionNanoseconds =
            Detail::MonotonicTimebaseTraits<TTimebaseProvider>::ResolutionNanoseconds;
```

### `Now`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Returns the current monotonic timestamp without mutating Clock-owned state.

Each call reads the selected timebase exactly once and converts that absolute count to
nanoseconds. Conversion is derived from the exact rational source frequency, so rounded
per-count error is never accumulated across successive reads.

```cpp
MonotonicTimestamp Now() const noexcept
```

