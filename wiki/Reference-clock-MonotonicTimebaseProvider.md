# src/clock/MonotonicTimebaseProvider.hpp

**Primary classification:** PUBLIC PROVIDER / EXTENSION API

**Source baseline:** `6a9ab9ffab8c90901f48b6c2c70767942fe43380`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/6a9ab9ffab8c90901f48b6c2c70767942fe43380/src/clock/MonotonicTimebaseProvider.hpp)

## Direct includes

- `cstdint`
- `ClockComposition.hpp`

## Documented declarations

### `TFrequencyNumerator`

**Classification:** PUBLIC PROVIDER / EXTENSION API

Base Composition declaration for a concrete MonotonicTimebase provider.

The exact nominal frequency is expressed as:

`counts per second = TFrequencyNumerator / TFrequencyDenominator`

A concrete provider inheriting this declaration must expose:

`std::uint64_t CurrentCount() const noexcept`

The returned count must be safe for concurrent reads, coherent, non-regressing, and
extended across any underlying hardware rollover for the supported application lifetime.

- **Template parameter `TFrequencyNumerator`:** Exact count-frequency numerator.
- **Template parameter `TFrequencyDenominator`:** Exact count-frequency denominator.

```cpp
template<
        std::uint32_t TFrequencyNumerator,
```

### `FrequencyNumerator`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Exact numerator of this timebase's nominal count frequency.

```cpp
static constexpr std::uint32_t FrequencyNumerator = TFrequencyNumerator;
```

### `FrequencyDenominator`

**Classification:** PUBLIC PROVIDER / EXTENSION API · source access: `public`

Exact denominator of this timebase's nominal count frequency.

```cpp
static constexpr std::uint32_t FrequencyDenominator = TFrequencyDenominator;
```

