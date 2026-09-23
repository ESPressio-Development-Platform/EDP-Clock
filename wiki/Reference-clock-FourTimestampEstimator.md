# src/clock/FourTimestampEstimator.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `8298f13f210665ca90a2cbeb19de43e45342aa18`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/8298f13f210665ca90a2cbeb19de43e45342aa18/src/clock/FourTimestampEstimator.hpp)

## Direct includes

- `cstdint`
- `limits`
- `FourTimestampExchange.hpp`
- `FourTimestampExchangeResult.hpp`

## Documented declarations

### `AddExchangeUncertainty`

**Classification:** PUBLIC API

Adds one nanosecond uncertainty contribution while saturating at the public uncertainty boundary.

```cpp
constexpr std::uint64_t AddExchangeUncertainty(
            std::uint64_t current,
            std::uint64_t contribution
        ) noexcept
```

### `EstimateFourTimestampExchange`

**Classification:** PUBLIC API

Reduces a four-timestamp exchange into one source-agnostic synchronization observation.

T1/T4 are local monotonic coordinates and T2/T3 are remote/reference synchronized
coordinates. The estimator therefore uses only same-domain subtraction:

`localElapsed = T4 - T1`
`remoteTurnaround = T3 - T2`
`pathRoundTrip = localElapsed - remoteTurnaround`
`referenceAtT4 = T3 + floor(pathRoundTrip / 2)`

The midpoint path estimate does not assume that the actual forward and reverse paths are
perfectly symmetric. Half of the measured path round-trip is retained as a conservative
path-asymmetry uncertainty bound. All four supplied timestamp uncertainty bounds are added
conservatively. The resulting observation is anchored at local T4 and can therefore be used
for first-ever synchronization while the local SynchronizedClock is still NeverSynchronized.

- **Parameter `exchange`:** Complete T1/T2/T3/T4 exchange and timestamp uncertainty bounds.
- **Returns:** Accepted observation estimate or an explicit rejection status.

```cpp
constexpr FourTimestampExchangeResult EstimateFourTimestampExchange(
        const FourTimestampExchange& exchange
    ) noexcept
```

