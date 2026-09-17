# Clock Correlation

## Contents

1. [Purpose](#purpose)
2. [Reference Mapping Versus Published Time](#reference-mapping-versus-published-time)
3. [Availability](#availability)
4. [Correlation Snapshot](#correlation-snapshot)
5. [Timestamp Projection](#timestamp-projection)
6. [Uncertainty](#uncertainty)
7. [Range Failures](#range-failures)
8. [Concurrency and Lifetime](#concurrency-and-lifetime)
9. [Memory Model](#memory-model)
10. [Intended Use](#intended-use)
11. [Non-Goals](#non-goals)

---

## Purpose

Hardware and transport layers should capture `MonotonicTimestamp` as close as practical to the physical event boundary. Application processing may happen much later. `ClockCorrelation` lets a consumer retain one compact immutable mapping and later answer:

> Given this captured monotonic occurrence, what is the best synchronized-time estimate for when it happened?

The correlation layer therefore supports precise timestamp-at-occurrence semantics without requiring every ISR, radio, sensor, event, or protocol implementation to read synchronized time directly.

---

## Reference Mapping Versus Published Time

`SynchronizedClock` deliberately contains two related but different concepts:

```text
accepted reference mapping
        |
        +--> ClockCorrelation
        |      best estimate for physical/event occurrence time
        |
        +--> published SynchronizedClock
               bounded phase slew
               never steps backwards after synchronization
```

A new observation can reveal that the published clock is, for example, 2 ms ahead of the best current reference. The public clock cannot jump backwards, so it enters `Reacquiring` and slews toward the new reference. Event correlation should not inherit that temporary publication error.

`ClockCorrelation` therefore uses:

```text
MonotonicAnchorNanoseconds
ReferenceAnchorNanoseconds
FrequencyCorrectionPartsPerBillion
FrequencyUncertaintyPartsPerBillion
UncertaintyAtAnchorNanoseconds
```

It intentionally does **not** use:

```text
SynchronizedAnchorNanoseconds
RemainingPhaseCorrectionNanoseconds
```

The first set describes the best accepted physical/reference relationship. The second set exists to maintain the public non-regressing synchronized timeline.

---

## Availability

Before Clock has accepted any synchronization observation:

```cpp
synchronizedClock.Correlation().State()
    == ClockCorrelationState::Unavailable
```

After any accepted observation, the correlation becomes available even if the public clock remains `NeverSynchronized` because the observation uncertainty is too large to satisfy the strict synchronization contract.

This distinction is intentional:

```text
Correlation available
    means
Clock has a defensible monotonic/reference estimate.

Synchronized state
    means
The public clock currently satisfies its configured quality contract.
```

For example, an estimate with ±3 ms uncertainty can still be useful to an application even though it cannot satisfy the default strict `< 1 ms` `Synchronized` requirement.

Likewise, a correlation can remain useful while the public clock is `Reacquiring`, `Holdover`, or `LostSynchronization`; its own uncertainty tells the consumer how useful the retained estimate remains.

---

## Correlation Snapshot

A consumer captures one immutable mapping with:

```cpp
const auto correlation = synchronizedClock.Correlation();
```

`ClockCorrelation` contains:

```text
MonotonicTimestamp        anchor
SynchronizedTimestamp     reference anchor
int32                      frequency correction, ppb
uint32                     frequency uncertainty, ppb
SynchronizationUncertainty anchor uncertainty
ClockCorrelationState      availability
```

The complete value is exactly:

```text
32 bytes
```

It owns no heap allocation, pointer, mutex, callback, registry entry, worker task, or external lifetime relationship. Once captured, later Clock observations do not mutate that correlation.

This means a radio or sensor subsystem can retain one correlation for a frame, acquisition batch, DMA buffer, or group of events and project every associated monotonic timestamp against the same coherent mapping.

---

## Timestamp Projection

Projection is explicit:

```cpp
const auto result = correlation.Correlate(
    eventMonotonicTimestamp,
    captureUncertainty
);
```

A correlation can project both forward and backward from its anchor.

For a monotonic distance `D` from the anchor, Clock applies the retained signed frequency correction using integer-only parts-per-billion arithmetic:

```text
rate adjustment = D * abs(frequency correction) / 1,000,000,000
```

A positive correction increases synchronized elapsed time. A negative correction decreases it.

The resulting synchronized distance is added to or subtracted from the synchronized reference anchor depending on whether the event lies after or before the monotonic anchor.

No floating-point arithmetic is used.

### Why projection can be retrospective

A hardware event might occur at monotonic time `M1` while application processing occurs at `M2` several milliseconds later. The event retains `M1`; processing can use a retained correlation to recover the synchronized occurrence estimate later.

```text
physical event
    |
    +--> capture MonotonicTimestamp M1

... processing delay ...

application code
    |
    +--> correlation.Correlate(M1, capture uncertainty)
    |
    +--> synchronized occurrence estimate
```

The processing delay itself does not contaminate the event timestamp.

---

## Uncertainty

Projection quality remains first-class.

The resulting uncertainty is:

```text
anchor uncertainty
+ rate-uncertainty growth over abs(monotonic distance)
+ caller-supplied capture uncertainty
```

Rate uncertainty grows in both projection directions because distance from the known anchor increases uncertainty regardless of whether the event happened before or after that anchor.

The capture uncertainty is deliberately supplied by the caller. Clock cannot know whether a monotonic timestamp came from:

```text
hardware RX timestamping
an ISR
DMA metadata
a driver callback
an application task
```

Those surfaces can have materially different timestamp-capture quality. A caller that has a ±100 ns hardware timestamp should not be forced to pretend it has the same quality as a software callback with tens of microseconds of scheduling uncertainty.

`SynchronizationUncertainty` saturates at `UINT32_MAX` nanoseconds rather than wrapping. A saturated uncertainty can still accompany a mathematically valid coordinate; it means the retained mapping is no longer meaningfully precise.

---

## Range Failures

Coordinate underflow and overflow are not silently clamped.

`ClockCorrelationProjectionStatus` is:

```cpp
enum class ClockCorrelationProjectionStatus : std::uint8_t {
    Correlated = 0,
    CorrelationUnavailable = 1,
    SynchronizedCoordinateUnderflow = 2,
    SynchronizedCoordinateOverflow = 3
};
```

A failed projection carries saturated uncertainty and must not be treated as a valid synchronized timestamp.

Uncertainty saturation is different from coordinate overflow. The former means “the coordinate is mathematically representable but quality is effectively unbounded”; the latter means “the synchronized coordinate itself cannot be represented”.

---

## Concurrency and Lifetime

`SynchronizedClockProvider::Correlation()` performs one coherent read of the existing `ConcurrentSnapshot` discipline state and copies the accepted reference mapping into the immutable 32-byte value.

After that call, projection requires no further synchronized-clock state read:

```text
ConcurrentSnapshot read
        |
        v
ClockCorrelation
        |
        +--> Correlate(event 1)
        +--> Correlate(event 2)
        +--> Correlate(event 3)
        +--> ...
```

The correlation itself is trivially copyable and contains no concurrency primitive. This makes batch timestamp conversion inexpensive and deterministic.

Clock retains no correlation-history ring. Consumers that need a historical mapping retain the correlation value themselves.

---

## Memory Model

| Facility | Size |
| --- | ---: |
| `ClockCorrelation` | 32 bytes |
| `ClockCorrelationProjection` | 16 bytes |

The correlation layer adds no retained storage to `SynchronizedClockProvider`. A correlation object exists only where a consumer chooses to retain one.

The existing synchronized discipline state and its Platform `ConcurrentSnapshot` remain unchanged.

---

## Intended Use

Typical hardware/event timestamping is:

```cpp
const auto correlation = synchronizedClock.Correlation();

// Later, or for each item in a captured batch:
const auto projection = correlation.Correlate(
    capturedMonotonicTimestamp,
    captureUncertainty
);

if (projection.IsCorrelated()) {
    const auto occurredAt = projection.Timestamp();
    const auto quality = projection.Uncertainty();
}
```

This is particularly suitable for:

```text
radio RX/TX timestamps
Mesh message timing
sensor samples
Magic Cone timing gates
interrupt-originated events
DMA capture batches
distributed ESPressio Events
```

A consumer can choose when to capture the correlation. Capturing it earlier freezes the knowledge available at that point; capturing a newer correlation later uses newer accepted reference evidence for retrospective event projection.

---

## Non-Goals

Clock Correlation does not reconstruct the historical value that `SynchronizedNow()` happened to publish at an earlier instant.

That would require retaining every publication-discipline transition or a bounded history structure, increasing memory and complexity. If an application needs the exact historical published reading, it must retain the `SynchronizedReading` obtained at that time.

Correlation instead answers the distributed-system question that matters for physical events:

> Under this retained accepted reference mapping, what is the best synchronized estimate of the time represented by this monotonic occurrence, and how uncertain is that estimate?

It also does not own source selection, transport timestamp acquisition, calendar interpretation, time zones, RTC semantics, or scheduling.
