# src/clock/ClockComposition.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `8298f13f210665ca90a2cbeb19de43e45342aa18`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/8298f13f210665ca90a2cbeb19de43e45342aa18/src/clock/ClockComposition.hpp)

## Direct includes

- `cstdint`
- `ESPressio_System.hpp`

## Documented declarations

### `Domain`

**Classification:** PUBLIC API

Composition domain containing physical and electronic Clock capabilities.

```cpp
struct Domain final : Framework::Domain {};
```

### `MonotonicTimebase`

**Classification:** PUBLIC API · source access: `public`

Exclusive capability supplying the application-selected monotonic physical timebase.

A conforming provider exposes a `std::uint64_t CurrentCount() const noexcept` operation.
Concurrent calls to CurrentCount() must be safe, coherent, and non-regressing. Any raw
hardware rollover must be hidden by the concrete provider before the count is exposed.

```cpp
struct MonotonicTimebase final : Framework::ExclusiveCapability<Domain> {};
```

### `TimebaseFrequencyNumerator`

**Classification:** PUBLIC API · source access: `public`

Exact numerator of the MonotonicTimebase nominal count frequency in counts per second.

```cpp
struct TimebaseFrequencyNumerator final : Framework::Property<MonotonicTimebase, std::uint32_t> {};
```

### `TimebaseFrequencyDenominator`

**Classification:** PUBLIC API · source access: `public`

Exact denominator of the MonotonicTimebase nominal count frequency in counts per second.

```cpp
struct TimebaseFrequencyDenominator final : Framework::Property<MonotonicTimebase, std::uint32_t> {};
```

### `MonotonicClock`

**Classification:** PUBLIC API · source access: `public`

Exclusive capability supplying canonical nanosecond monotonic timestamps.

```cpp
struct MonotonicClock final : Framework::ExclusiveCapability<Domain> {};
```

### `ClockResolutionNanoseconds`

**Classification:** PUBLIC API · source access: `public`

Conservative whole-nanosecond resolution of the MonotonicClock capability.

```cpp
struct ClockResolutionNanoseconds final : Framework::Property<MonotonicClock, std::uint64_t> {};
```

### `SynchronizedClock`

**Classification:** PUBLIC API · source access: `public`

Exclusive capability supplying source-disciplined synchronized clock readings.

```cpp
struct SynchronizedClock final : Framework::ExclusiveCapability<Domain> {};
```

### `SynchronizedClockUncertaintyLimitNanoseconds`

**Classification:** PUBLIC API · source access: `public`

Exclusive upper uncertainty bound below which SynchronizedClock may report Synchronized.

```cpp
struct SynchronizedClockUncertaintyLimitNanoseconds final : Framework::Property<SynchronizedClock, std::uint32_t> {};
```

### `SynchronizedClockMaximumFrequencyCorrectionPartsPerBillion`

**Classification:** PUBLIC API · source access: `public`

Maximum absolute long-term frequency correction applied by SynchronizedClock, in ppb.

```cpp
struct SynchronizedClockMaximumFrequencyCorrectionPartsPerBillion final : Framework::Property<SynchronizedClock, std::uint32_t> {};
```

### `SynchronizedClockMaximumPhaseSlewPartsPerBillion`

**Classification:** PUBLIC API · source access: `public`

Maximum absolute temporary phase slew applied by SynchronizedClock, in ppb.

```cpp
struct SynchronizedClockMaximumPhaseSlewPartsPerBillion final : Framework::Property<SynchronizedClock, std::uint32_t> {};
```

