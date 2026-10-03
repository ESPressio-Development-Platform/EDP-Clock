# Architecture

EDP-Clock separates five concerns:

1. A Bootstrap-selected `MonotonicTimebase` exposes coherent, non-regressing native counts plus an exact rational frequency.
2. `MonotonicClockProvider` converts that source to the canonical unsigned 64-bit nanosecond coordinate without cumulative rounding drift.
3. `SynchronizedClockProvider` disciplines the canonical monotonic coordinate toward source-agnostic observations using integer rate correction, bounded phase slew and explicit uncertainty.
4. `ClockCorrelation` captures an immutable accepted-reference mapping for retrospective occurrence projection. Compact `ClockEra` and Era-qualified wrappers preserve temporal provenance without importing source identity.
5. `Stopwatch` consumes only monotonic time and has no synchronized/provenance dependency.

Connectivity providers convert native capture evidence into `MonotonicCaptureBounds` before the Clock boundary. The interval contains only canonical local monotonic coordinates; native ticks, wrap state, event class, provider generation and live validity remain provider-owned.

The upstream coordinator owns source/root/parent selection, the full authenticated mapping behind an opaque Era, and application transition policy. It is the sole writer of observations and explicit same-/cross-Era transitions. Clock never ranks sources or blends different Eras automatically.

Synchronized state is explicit: `NeverSynchronized`, `Synchronized`, `Holdover`, `Reacquiring`, and `LostSynchronization`. Published time remains non-regressing after it first becomes authoritative unless an explicit cross-Era `ReconstructTimeline` operation resets the runtime mapping. Cross-Era preservation invalidates old correlation while retaining the public coordinate with saturated uncertainty.
