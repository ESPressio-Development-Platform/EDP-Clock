# Dependency Contracts

EDP-Clock depends on **EDP-System** and **EDP-Platform**.

## EDP-System

The Clock domain, capabilities, provider offers and requirements use the EDP-System Composition Framework.

## Same-domain Clock contracts

### MonotonicClockProvider

Requires exactly one same-domain `MonotonicTimebase` provider. The selected timebase must expose a coherent, concurrent-safe, non-regressing `CurrentCount() const noexcept` and exact frequency numerator/denominator properties.

### SynchronizedClockProvider

Requires exactly one same-domain `MonotonicClock` whose `ClockResolutionNanoseconds` is **strictly less than** the configured synchronization uncertainty limit.

## EDP-Platform contract

`SynchronizedClockProvider` additionally requires exactly one external `AtomicWord32` provider with:

- `LockFree == true`;
- `AtomicWordStorageBytes == sizeof(uint32_t)`.

That provider backs the fixed-storage `ConcurrentSnapshot` used for one-writer/many-reader synchronized-clock publication.

## Application-wide monotonic binding

`BindMonotonicClock()` establishes a process-lifetime borrowed binding used by `MonotonicNow()`. Binding must happen during Bootstrap before concurrent users run and is intentionally irreversible for the runtime lifetime. EDP-Threading depends on this convenience binding for canonical deadlines.

## Ownership

Clock providers borrow their lower-level provider instances; they do not own timer hardware or Platform provider lifetime.

> Dependency contract audit baseline: `774c3bd626d3b79b988ccc93c990d156ed433bcc` (`main`).
