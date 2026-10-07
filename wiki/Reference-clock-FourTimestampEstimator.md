# src/clock/FourTimestampEstimator.hpp

**Primary classification:** PUBLIC API

**Source baseline:** `6dbd31804521a8691b618454a496e1c0d4bf6ee0`

[Open exact source](https://github.com/ESPressio-Development-Platform/EDP-Clock/blob/6dbd31804521a8691b618454a496e1c0d4bf6ee0/src/clock/FourTimestampEstimator.hpp)

## Direct includes

- `cstdint`
- `limits`
- `FourTimestampExchange.hpp`
- `FourTimestampExchangeResult.hpp`

## Documented declarations

### `AddExchangeUncertainty`

**Classification:** PRIVATE IMPLEMENTATION

Adds one nanosecond uncertainty contribution while saturating at the public uncertainty boundary. The estimator uses this helper to accumulate path and timestamp uncertainty without allowing integer wrap to make an observation appear more precise than its evidence supports.

```cpp
constexpr std::uint64_t AddExchangeUncertainty(
            std::uint64_t current,
            std::uint64_t contribution
        ) noexcept
```

### `EstimateFourTimestampExchange`

**Classification:** PUBLIC API

Reduces a four-timestamp exchange into one source-agnostic synchronization observation.

T1/T4 are local monotonic coordinates and T2/T3 are remote/reference synchronized coordinates. The estimator therefore uses only same-domain subtraction:

`localElapsed = T4 - T1`

`remoteTurnaround = T3 - T2`

For an ordinary non-negative path:

`pathRoundTrip = localElapsed - remoteTurnaround`

`referenceAtT4 = T3 + floor(pathRoundTrip / 2)`

Path feasibility is uncertainty-aware. The estimator forms a conservative timestamp envelope:

`timestampEnvelope = U(T1) + U(T2) + U(T3) + U(T4)`

If `remoteTurnaround > localElapsed`, the nominal path is negative. That exchange is rejected only when the negative-path deficit is larger than the complete declared timestamp envelope. When the deficit still overlaps the physically valid zero-delay boundary, the estimator accepts the exchange with `pathRoundTrip = 0`. It never manufactures a negative transport delay and it carries all four timestamp uncertainty contributions into the resulting observation.

The midpoint path estimate does not assume that the actual forward and reverse paths are perfectly symmetric. For positive measured path delay, half of the measured round-trip path is retained as a conservative path-asymmetry uncertainty bound, rounded upward. All four supplied timestamp uncertainty bounds are then added conservatively. The public uncertainty saturates rather than wraps.

The resulting observation is anchored at local T4 and can therefore be used for first-ever synchronization while the local `SynchronizedClock` is still `NeverSynchronized`.

Hard rejection remains explicit for invalid local timestamp order, invalid remote timestamp order, a nominal negative path wholly outside the declared uncertainty envelope, and reference-coordinate overflow.

- **Parameter `exchange`:** Complete T1/T2/T3/T4 exchange and timestamp uncertainty bounds.
- **Returns:** Accepted observation estimate or an explicit rejection status.

```cpp
constexpr FourTimestampExchangeResult EstimateFourTimestampExchange(
        const FourTimestampExchange& exchange
    ) noexcept
```
