# Synchronized Clock

## Contents

1. [Purpose](#purpose)
2. [Domain Boundary](#domain-boundary)
3. [Dependencies](#dependencies)
4. [Public Types](#public-types)
5. [Synchronization State](#synchronization-state)
6. [Synchronization Observations](#synchronization-observations)
7. [Discipline Behaviour](#discipline-behaviour)
8. [Uncertainty](#uncertainty)
9. [Concurrency](#concurrency)
10. [Composition and Bootstrap](#composition-and-bootstrap)
11. [Memory Model](#memory-model)
12. [Source and Transport Separation](#source-and-transport-separation)
13. [Example](#example)
14. [Summary](#summary)

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

A transport-specific mechanism may perform richer mathematics first—for example a four-timestamp exchange—and submit the resulting estimate and uncertainty to Clock.

`Observe()` returns an explicit `SynchronizationObservationStatus`. Observations can be rejected when their local monotonic coordinate regresses, their inferred rate exceeds policy, arithmetic cannot be represented safely, or the timing relationship is inconsistent with the retained mapping.

Clock does not silently clamp invalid evidence and present it as healthy.

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

`SynchronizedClockProvider` formally requires `MonotonicClock` in the Clock domain. Its selected `TAtomicWordProvider` is independently validated through the Platform `ConcurrentSnapshot` contract. This avoids inventing a duplicate Clock capability solely to bridge Composition domains.

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
Mesh -----------+
GNSS -----------+
RTC ------------+--> source selection / observation math
Network --------+                 |
Other ----------+                 v
                           SynchronizationObservation
                                      |
                                      v
                              SynchronizedClock
```

The upstream layer decides which source to trust, when to switch source, whether to combine sources, and how transport delay/jitter contributes to uncertainty.

Clock decides whether an observation is temporally/rate admissible, how phase/rate are disciplined, how uncertainty evolves, and what lifecycle state consumers observe.

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

When an upstream synchronization mechanism has an accepted estimate:

```cpp
synchronized.Observe(
    ESPressio::Clock::SynchronizationObservation(
        localCapture,
        referenceTimestamp,
        referenceUncertainty
    )
);
```

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
source-agnostic observations
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

The implementation remains source-agnostic, transport-agnostic, calendar-agnostic, heap-free, and safe for concurrent readers under the single-writer discipline contract.
