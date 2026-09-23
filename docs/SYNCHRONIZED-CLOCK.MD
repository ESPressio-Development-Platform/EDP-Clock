# Synchronized Clock

## Contents

1. [Purpose](#purpose)
2. [Domain Boundary](#domain-boundary)
3. [Dependencies](#dependencies)
4. [Public Types](#public-types)
5. [Synchronization State](#synchronization-state)
6. [Synchronization Observations](#synchronization-observations)
7. [Four-Timestamp Time Transfer](#four-timestamp-time-transfer)
8. [Discipline Behaviour](#discipline-behaviour)
9. [Uncertainty](#uncertainty)
10. [Concurrency](#concurrency)
11. [SynchronizedNow](#synchronizednow)
12. [Composition and Bootstrap](#composition-and-bootstrap)
13. [Memory Model](#memory-model)
14. [Source and Transport Separation](#source-and-transport-separation)
15. [Example](#example)
16. [Summary](#summary)

---

## Purpose

`SynchronizedClock` derives a shared physical/electronic time coordinate from the already-selected `MonotonicClock`.

It does **not** own another hardware timer. The model is:

```text
MonotonicClock
      +
shared synchronization observations
      +
frequency discipline
      +
bounded phase slew
      =
SynchronizedClock
```

The synchronized coordinate is intentionally not UTC, local time, calendar time, or any other human interpretation. It is an opaque shared nanosecond coordinate suitable for comparing events across independently running systems that share the same synchronization reference.

---

## Domain Boundary

`EDP-Clock` owns the synchronized-time semantics:

```text
SynchronizedTimestamp
SynchronizationUncertainty
SynchronizationState
SynchronizedReading
SynchronizationObservation
FourTimestampExchange
EstimateFourTimestampExchange()
time-transfer admission
observation admission
frequency correction
phase slew
uncertainty projection
SynchronizedNow()
```

It does not own Mesh, Radio, UDP/TCP, GNSS, RTC hardware, source arbitration, transport timestamp capture, calendars, time zones, or formatted human time.

Those concerns remain in their own domains and reduce their evidence to a `SynchronizationObservation` before Clock consumes it.

---

## Dependencies

The synchronized layer depends on:

```text
EDP-System
    Composition Framework

EDP-Platform
    AtomicWord32
    ConcurrentSnapshot

EDP-Clock monotonic foundation
    MonotonicClock
    MonotonicTimestamp
    Duration
    Delta
```

Production Clock code does not depend directly on Arduino, ESP-IDF, FreeRTOS, `std::atomic`, mutexes, semaphores, tasks, queues, or operating-system synchronization primitives.

A concrete Platform `AtomicWord32` provider is selected independently by Bootstrap and validated through `ConcurrentSnapshot`.

---

## Public Types

### SynchronizedTimestamp

`SynchronizedTimestamp` is an unsigned 64-bit nanosecond coordinate:

```cpp
sizeof(SynchronizedTimestamp) == 8
```

Zero is a valid synchronized coordinate. It is a distinct strong type from `MonotonicTimestamp`, so cross-domain arithmetic is not accidentally available.

Two synchronized timestamps may be compared with:

```cpp
const Duration elapsed = Delta(
    from,
    to
);
```

### SynchronizationUncertainty

`SynchronizationUncertainty` is an unsigned 32-bit nanosecond bound:

```cpp
sizeof(SynchronizationUncertainty) == 4
```

Its range is `0 .. 4,294,967,295 ns` (about 4.295 seconds). Values beyond that range saturate. Saturation means the mapping is no longer sufficiently bounded for useful synchronized-time quality reporting.

The default-constructed value is saturated rather than zero, preventing default construction from accidentally implying perfect confidence.

### SynchronizedReading

`SynchronizedNow()` returns a `SynchronizedReading` containing:

```text
SynchronizedTimestamp
SynchronizationUncertainty
SynchronizationState
```

The complete reading is 16 bytes. Timestamp, uncertainty and lifecycle state are projected from one coherent discipline snapshot and one monotonic sample.

---

## Synchronization State

The public lifecycle state is:

```cpp
enum class SynchronizationState : std::uint8_t {
    NeverSynchronized = 0,
    Synchronized = 1,
    Holdover = 2,
    Reacquiring = 3,
    LostSynchronization = 4
};
```

### NeverSynchronized

Initial state for every runtime incarnation. It means that the synchronized clock has never yet satisfied its synchronization-quality contract in this runtime incarnation.

Once the provider leaves `NeverSynchronized`, it never returns to that state until a new runtime incarnation begins.

### Synchronized

The state may be `Synchronized` only while total uncertainty is strictly:

```text
< 1,000,000 ns
```

Exactly 1 ms is **not** synchronized.

### Holdover

The clock synchronized previously and still has a meaningful extrapolated mapping, but fresh evidence has aged enough that uncertainty is no longer below the synchronization threshold.

### Reacquiring

The clock synchronized previously, but current accepted evidence or an active phase correction means the mapping does not presently satisfy the strict synchronization-quality contract.

### LostSynchronization

The clock synchronized previously, but uncertainty has saturated and the mapping should no longer be treated as meaningfully bounded synchronized time.

---

## Synchronization Observations

Clock accepts one source-agnostic input shape:

```cpp
SynchronizationObservation
```

containing:

```text
LocalTimestamp
ReferenceTimestamp
ReferenceUncertainty
```

Semantically:

> At `LocalTimestamp`, the best accepted estimate of synchronized time was `ReferenceTimestamp`, within `ReferenceUncertainty`.

Clock therefore does not need to know whether the evidence came from a Mesh peer, GNSS, an RTC-derived reference, a network exchange, a wired reference, a host processor, or another future source.

For transport mechanisms capable of request/response timestamp exchange, `EDP-Clock` also owns the generic four-timestamp reduction that produces this observation. Transport code captures and carries T1/T2/T3/T4; it does not need to reimplement the Clock mathematics.

`Observe()` returns an explicit `SynchronizationObservationStatus`. Observations can be rejected when their local monotonic coordinate regresses, their inferred rate exceeds policy, arithmetic cannot be represented safely, or the timing relationship is inconsistent with the retained mapping.

Clock does not silently clamp invalid evidence and present it as healthy.

---

## Four-Timestamp Time Transfer

`FourTimestampExchange` represents one source-agnostic request/response exchange:

```text
T1  local MonotonicTimestamp      request transmitted
T2  remote SynchronizedTimestamp request received
T3  remote SynchronizedTimestamp response transmitted
T4  local MonotonicTimestamp      response received
```

Each timestamp carries a separate conservative `SynchronizationUncertainty` bound. Transport implementations should capture the timestamps as close as practical to the physical transmit/receive boundary and supply any additional capture/reference uncertainty they know about.

The important type distinction is deliberate: **T1 and T4 are local monotonic coordinates, not local synchronized timestamps**. The local node can therefore perform a four-timestamp exchange while its own `SynchronizedClock` is still `NeverSynchronized`. T2 and T3 belong to the remote/reference synchronized coordinate.

### Reduction mathematics

Clock never directly subtracts timestamps belonging to different coordinate domains. Instead it calculates same-domain intervals:

```text
localElapsed       = T4 - T1
remoteTurnaround   = T3 - T2
pathRoundTrip      = localElapsed - remoteTurnaround
estimatedReturn    = floor(pathRoundTrip / 2)
referenceAtT4      = T3 + estimatedReturn
```

This is the useful four-timestamp relationship expressed without requiring a pre-existing local synchronized epoch. `referenceAtT4` becomes the `ReferenceTimestamp` in a `SynchronizationObservation` whose `LocalTimestamp` is T4.

An exchange is rejected when:

```text
T4 < T1
T3 < T2
remoteTurnaround > localElapsed
T3 + estimatedReturn cannot be represented
```

The corresponding `FourTimestampExchangeStatus` makes the rejection reason explicit.

### Path asymmetry and uncertainty

The midpoint estimate is **not** presented as perfect simply because the arithmetic assumes a midpoint. Without stronger transport-specific knowledge, the forward and reverse path delays may be asymmetric. Clock therefore adds:

```text
ceil(pathRoundTrip / 2)
```

as a conservative path-asymmetry uncertainty bound. It then adds the four supplied timestamp/capture uncertainty bounds. The public 32-bit uncertainty saturates rather than wraps.

Consequently a transport with several milliseconds of unresolved path delay may produce a mathematically valid observation that still cannot satisfy the strict `< 1 ms` `Synchronized` contract. That is intentional: Clock reports the quality the evidence can justify.

The estimator is used as:

```cpp
const auto estimate = ESPressio::Clock::EstimateFourTimestampExchange(
    exchange
);

if (estimate.IsAccepted()) {
    synchronizedClock.Observe(
        estimate.Observation()
    );
}
```

`FourTimestampExchangeResult::RoundTripPathDelayNanoseconds()` also exposes the derived aggregate transport path delay for diagnostics or higher-level source-selection policy.

---

## Discipline Behaviour

### Initial establishment

Before synchronized time has ever been publicly established, the first acceptable observation may establish the mapping immediately. This avoids forcing a long slew from an arbitrary startup offset.

If that mapping satisfies the quality requirement, the state becomes `Synchronized`. Otherwise the state remains `NeverSynchronized`; merely receiving observations does not mean synchronization has ever met its contract.

### Frequency correction

Long-term rate correction is represented as signed integer parts-per-billion. The default accepted range is:

```text
-1,000,000 ppb .. +1,000,000 ppb
```

which is ±1,000 ppm (±0.1%).

An observation sequence implying a larger steady correction is rejected rather than silently clamped. Such a correction is more likely to indicate a bad source, incorrect source metadata, or unsuitable timebase than something Clock should hide.

### Phase slew

After synchronized time has been established, published synchronized time must not step backwards.

Phase corrections are therefore applied by temporarily changing the projected rate within the configured slew bound. The default is:

```text
±5,000,000 ppb
= ±5,000 ppm
= ±0.5%
```

At maximum slew, phase is corrected at approximately 5 ms per second. Therefore a 2 ms correction converges in about 400 ms.

Initial acquisition may step because no trustworthy synchronized history exists yet. Subsequent negative corrections slow the synchronized clock rather than reversing it.

---

## Uncertainty

Uncertainty is first-class state, not decorative metadata.

Projected uncertainty includes, as applicable:

```text
reference observation uncertainty
remaining phase error
frequency/rate uncertainty
elapsed time since the retained anchor/reference
```

Uncertainty grows as reference evidence ages. It is never frozen at the last observation value.

The hard invariant is:

```text
State == Synchronized
    implies
Uncertainty < 1,000,000 ns
```

If uncertainty reaches or exceeds 1 ms, the state cannot remain `Synchronized`.

Typical reference-loss progression is:

```text
Synchronized
    |
    | uncertainty grows
    v
Holdover
    |
    | uncertainty saturates
    v
LostSynchronization
```

When trustworthy observations return, a previously synchronized clock may enter `Reacquiring` and then return to `Synchronized` once residual error and total uncertainty are again strictly below 1 ms.

---

## Concurrency

Synchronized discipline state is mutable and expected to be read frequently across thread/task boundaries.

The implementation uses:

```text
EDP-Platform::Concurrency::ConcurrentSnapshot
```

with:

```text
one writer
many concurrent readers
fixed storage
no heap
no reader mutex
no worker task
```

The single writer publishes complete replacement discipline states. Readers obtain one coherent snapshot and therefore cannot combine a new anchor with an old rate or uncertainty value.

The public read path deliberately:

1. captures one coherent discipline snapshot;
2. samples `MonotonicClock` exactly once;
3. projects timestamp, uncertainty and state from those inputs;
4. returns one `SynchronizedReading`.

The discipline snapshot is captured before the monotonic sample, preventing a concurrently published mapping from containing an anchor later than the monotonic coordinate used to project it.

Thread-safe does not automatically mean ISR-safe. ISR timestamp capture remains a source/provider contract.

---

## SynchronizedNow

Application Bootstrap binds one lifetime-stable synchronized provider:

```cpp
BindSynchronizedClock(
    synchronizedClock
);
```

Linked code units may then call:

```cpp
const auto reading = SynchronizedNow();
```

without each long-lived consumer storing another clock pointer.

The binding is permanent for the runtime incarnation and is not a service locator or mutable registry.

`SynchronizedNow()` intentionally returns `SynchronizedReading`, not a naked timestamp. Discarding synchronization quality must be an explicit consumer decision.

---

## Composition and Bootstrap

Clock and Platform remain separate Composition domains.

The Clock domain supplies:

```text
MonotonicTimebase
MonotonicClock
SynchronizedClock
```

The Platform domain supplies:

```text
AtomicWord32
```

`SynchronizedClockProvider` declares a consolidated provider `Contract`: exactly one qualified `MonotonicClock` must be present in the same Clock domain, and exactly one qualified Platform `AtomicWord32` provider must be present in an external domain. The external Requirement carries the lock-free/four-byte qualification already required by `ConcurrentSnapshot`; it adds compile-time Architecture visibility without adding runtime state.

The intended Bootstrap order is:

```text
1. System Identity
2. Platform Requirements
3. Clock
4. higher-level domains
```

Typical construction is therefore:

```text
construct/select Platform AtomicWord32 provider
construct MonotonicTimebase
construct MonotonicClock
construct SynchronizedClock
bind MonotonicNow()
bind SynchronizedNow()
initialize synchronization sources/transports
initialize higher-level application domains
```

---

## Memory Model

The synchronized implementation is fixed-size and heap-free.

| Facility | Size |
| --- | ---: |
| `SynchronizedTimestamp` | 8 bytes |
| `SynchronizationUncertainty` | 4 bytes |
| `SynchronizedReading` | 16 bytes |
| `FourTimestampExchange` | 48 bytes |
| `FourTimestampExchangeResult` | 40 bytes |

The logical mutable discipline state is exactly:

```text
48 bytes
```

Its object representation uses explicit reserved bytes rather than relying on indeterminate ABI padding.

With the Platform double-buffered snapshot and four-byte atomic words:

```text
2 x 48-byte state buffers
+ 8-byte publication generation
= 104 bytes
```

The provider additionally retains one borrowed pointer/reference-sized relationship to the selected monotonic clock.

No dynamic allocation is performed by Synchronized Clock.

---

## Source and Transport Separation

Clock does not choose which synchronization source is authoritative.

```text
Mesh / Radio / Network transport
        |
        | captures and carries T1/T2/T3/T4
        v
FourTimestampExchange
        |
        | EDP-Clock time-transfer mathematics
        v
SynchronizationObservation
        |
        +------------------------------+
                                       |
GNSS / RTC / direct reference --------+--> source selection / accepted observations
                                       |
                                       v
                               SynchronizedClock
```

The transport owns timestamp acquisition and carriage. For four-timestamp transports, Clock owns the generic per-exchange delay/asymmetry mathematics and converts the exchange into a `SynchronizationObservation`. Direct references such as GNSS or RTC-derived sources may produce observations without a four-timestamp exchange.

The upstream source-selection layer decides which source or accepted estimate to trust, when to switch source, and whether/how to combine multiple observations. Clock decides how an individual four-timestamp exchange is reduced, whether an observation is temporally/rate admissible, how phase/rate are disciplined, how uncertainty evolves, and what lifecycle state consumers observe.

This separation allows future Mesh, Radio, RTC and networking implementations to evolve independently from Clock discipline internals.

---

## Example

A typical provider shape is:

```cpp
using Monotonic = ESPressio::Clock::MonotonicClockProvider<
    ApplicationTimebase
>;

using Synchronized = ESPressio::Clock::SynchronizedClockProvider<
    Monotonic,
    ApplicationAtomicWord32Provider
>;

ApplicationTimebase timebase;
Monotonic monotonic(timebase);
Synchronized synchronized(monotonic);

ESPressio::Clock::BindMonotonicClock(
    monotonic
);

ESPressio::Clock::BindSynchronizedClock(
    synchronized
);
```

A request/response transport can submit a complete four-timestamp exchange:

```cpp
const auto estimate = ESPressio::Clock::EstimateFourTimestampExchange(
    ESPressio::Clock::FourTimestampExchange(
        t1,
        t1Uncertainty,
        t2,
        t2Uncertainty,
        t3,
        t3Uncertainty,
        t4,
        t4Uncertainty
    )
);

if (estimate.IsAccepted()) {
    synchronized.Observe(
        estimate.Observation()
    );
}
```

A direct source that already has a defensible local/reference relationship may still construct `SynchronizationObservation` directly; four-timestamp exchange is not mandatory for sources that do not use request/response transfer.

Consumers can then use:

```cpp
const auto reading = ESPressio::Clock::SynchronizedNow();
```

and inspect its timestamp, uncertainty and lifecycle state together.

See `examples/synchronized-basic/main.cpp` for the deterministic buildable example.

---

## Summary

The synchronized Clock foundation provides:

```text
one physical MonotonicClock
        |
        v
transport T1/T2/T3/T4 exchanges
        |
        v
Clock-owned time-transfer estimator
        |
        +--> source-agnostic observations <--- direct references
                                      |
                                      v
integer-only frequency discipline
        |
        v
bounded non-regressing phase slew
        |
        v
explicit uncertainty growth
        |
        v
NeverSynchronized / Synchronized / Holdover /
Reacquiring / LostSynchronization lifecycle
        |
        v
fixed-storage ConcurrentSnapshot publication
        |
        v
SynchronizedReading
        |
        v
SynchronizedNow()
```

Default policy is strict sub-millisecond synchronization (`uncertainty < 1 ms`), ±1,000 ppm long-term correction, and ±5,000 ppm temporary phase slew.

The implementation remains source-agnostic, transport-agnostic in its mathematics, calendar-agnostic, heap-free, and safe for concurrent readers under the single-writer discipline contract. Transport libraries acquire/carry timestamps; they do not own duplicate Clock discipline or four-timestamp reduction algorithms.
