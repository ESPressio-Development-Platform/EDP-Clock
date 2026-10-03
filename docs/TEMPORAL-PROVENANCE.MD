# Temporal Provenance and Source Transitions

## Purpose

Distributed occurrences can be compared only when their synchronized coordinates belong to the same temporal provenance. EDP-Clock supplies the smallest source-agnostic value and discipline vocabulary needed to preserve that fact. It does not learn Mesh identity, route lineage, source rank, Radio-native ticks or application admission policy.

The contract consists of:

- `ClockEra`, a compact opaque comparable provenance key;
- `MonotonicCaptureBounds`, a canonical local-monotonic interval containing one physical occurrence;
- Era-qualified correlation, projection and policy-assessment values; and
- explicit same-Era and cross-Era discipline transitions invoked by the existing single writer.

## Ownership Boundary

| Concern | Owner |
| --- | --- |
| Opaque Era value semantics and comparison | EDP-Clock |
| Full Era provenance mapping and collision/conflict handling | upstream coordinator |
| Root/parent/provider selection and lineage | upstream coordinator |
| Provider-native counter capture, wrap and conversion | connectivity provider |
| Canonical local-monotonic interval and uncertainty | EDP-Clock value contract |
| Correlation/projection mathematics | EDP-Clock |
| Application time significance/admission | application policy |

No Device identifier, runtime identifier, route, stratum, provider rank or native counter enters an EDP-Clock Era-qualified value.

## ClockEra

`ClockEra` is exactly 8 bytes, trivially copyable and totally ordered as an opaque unsigned representation. Zero is invalid/unspecified. Clock neither manufactures nor interprets non-zero values.

The upstream coordinator must retain and authenticate the canonical mapping from a non-zero Era value to complete provenance. If two different provenances claim the same compact value while either mapping remains relevant, the coordinator must refuse the conflict rather than blend timelines. This makes compact repeated State/context values possible without pretending the full identity tuple fits inside Clock.

Ordering exists for deterministic containers and canonical comparisons; it does not imply chronological succession between unrelated Eras.

## Canonical Monotonic Capture Bounds

A provider converts its native evidence into a closed canonical interval:

```cpp
const auto capture = MonotonicCaptureBounds::Between(
    earliestCanonicalMonotonic,
    latestCanonicalMonotonic
);
```

Reversed bounds produce unavailable evidence and are never silently reordered. Exact evidence uses `Exact(timestamp)`.

The midpoint is calculated without overflow:

```text
midpoint = earliest + floor((latest - earliest) / 2)
```

The conservative uncertainty is:

```text
ceil((latest - earliest) / 2)
```

and saturates through `SynchronizationUncertainty`. An unavailable interval returns the monotonic origin only as a non-usable sentinel plus saturated uncertainty; callers must check `IsAvailable()`.

The interval carries no native counter, wrap state, provider generation, event class or quality claim. Those remain provider-contract facts surrounding the canonical Clock value.

## Era-Qualified Values

`EraQualifiedClockCorrelation` pairs one valid Era with an immutable 32-byte `ClockCorrelation`. It projects a captured local occurrence to an `EraQualifiedClockCorrelationProjection` and refuses to manufacture correlated evidence when either the Era or correlation is unavailable.

`EraQualifiedClockAssessment` carries:

- Era;
- projected synchronized timestamp;
- synchronization uncertainty;
- projection status; and
- `TemporalQualityAssessment`.

The assessment vocabulary is:

- `NotRequired`: application policy does not require temporal quality for this admitted information;
- `Reliable`: evidence satisfies application policy;
- `Unreliable`: admitted advisory evidence does not satisfy the selected quality requirement.

Strict-policy refusal happens before Primitive admission and is not encoded as an admitted assessment.

The assessment representation is flattened to 24 bytes rather than nesting a 24-byte projection and expanding to 32 bytes through aggregate alignment. It remains trivially copyable and owns no external lifetime.

## Discipline Transitions

All transition calls share the existing `Observe()` single-writer rule. They are non-blocking, allocation-free publications through the existing fixed `ConcurrentSnapshot`.

### Same Era

`BeginSameEraTransition()` is used when the upstream source, parent or provider changes but temporal provenance remains the same. It:

- retains the accepted reference correlation;
- retains the public non-regressing timeline and rate/phase discipline;
- raises anchor uncertainty to at least the configured synchronization limit;
- reports `Reacquiring` unless already lost; and
- returns `NoPublishedTimeline` and clears provisional state if no authoritative timeline has ever existed.

A subsequent accepted same-Era observation uses ordinary discipline and can restore `Synchronized`.

### Cross Era: preserve published timeline

`BeginCrossEraTransition(PreservePublishedTimeline)` is the non-regressing application choice. It:

- samples and anchors the current published coordinate;
- invalidates the old accepted reference correlation;
- clears old phase/rate evidence rather than blending Eras;
- retains authority to publish a non-regressing coordinate;
- saturates uncertainty and reports `LostSynchronization`; and
- causes the first new-Era observation to slew from the preserved public coordinate.

A new Era far behind the preserved coordinate can remain `LostSynchronization` until bounded slew reduces phase error into representable quality. Clock never disguises that interval as reliable.

### Cross Era: reconstruct runtime timeline

`BeginCrossEraTransition(ReconstructTimeline)` resets discipline to the initial `NeverSynchronized` state. The next accepted observation may establish the new Era directly, including a coordinate below the previously published one. This is an explicit application/runtime reconstruction decision, never an implicit source-switch side effect.

## State and Memory

| Value/state | Exact size |
| --- | ---: |
| `ClockEra` | 8 bytes |
| `MonotonicCaptureBounds` | 16 bytes |
| `EraQualifiedClockCorrelation` | 40 bytes |
| `EraQualifiedClockCorrelationProjection` | 24 bytes |
| `EraQualifiedClockAssessment` | 24 bytes |
| synchronized discipline snapshot | 48 bytes |

The discipline snapshot now distinguishes current accepted-reference availability from a timeline that has once become authoritative. The new bit replaces reserved storage; `ConcurrentStateStorageBytes` remains 104 bytes with the current Platform snapshot layout.

All values are fixed-size and trivially copyable. No transition allocates, waits, schedules work or invokes application code.

## Concurrency and Integration

Readers may continue calling `Now()` and `Correlation()` concurrently. Exactly one upstream writer serializes `Observe()`, `BeginSameEraTransition()` and `BeginCrossEraTransition()`. Clock does not serialize competing upstream coordinators.

A connectivity provider should:

1. validate native capture availability and generation;
2. extend/convert native counters and wrap state into canonical local monotonic bounds;
3. pass only canonical bounds/uncertainty across the Clock boundary; and
4. preserve provider-specific event class, resolution and live validity in its own contract.

An upstream coordinator should atomically associate current Era provenance with the reading/correlation it exposes to integrations. EDP-Clock intentionally does not own that coordination object.

## Non-Goals

These contracts do not:

- define a Mesh or deployment identifier;
- select, rank, authenticate or advertise a time source;
- prove an 802.11 timestamp's physical accuracy;
- define a wire encoding;
- apply strict/advisory/time-irrelevant admission policy;
- mutate Primitive payloads;
- make unrelated Eras comparable; or
- relax the configured synchronized-clock uncertainty limit.
