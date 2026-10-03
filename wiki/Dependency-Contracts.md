# Dependency Contracts

EDP-Clock has mandatory package dependencies on **EDP-System** and **EDP-Platform**. Temporal provenance introduces no EDP-Radio, EDP-Mesh, EDP-RemoteSystem or identity dependency.

## EDP-System

Clock Domain capabilities, provider offers, properties and requirements use the EDP-System Composition Framework.

## Same-domain Clock contracts

### MonotonicClockProvider

Requires exactly one same-domain `MonotonicTimebase` provider. The selected timebase exposes coherent, concurrent-safe, non-regressing `CurrentCount() const noexcept` and exact frequency numerator/denominator properties.

### SynchronizedClockProvider

Requires exactly one same-domain `MonotonicClock` whose `ClockResolutionNanoseconds` is strictly less than the configured synchronization uncertainty limit.

## EDP-Platform contract

`SynchronizedClockProvider` additionally requires exactly one external `AtomicWord32` provider with `LockFree == true` and `AtomicWordStorageBytes == sizeof(uint32_t)`. That provider backs the fixed-storage one-writer/many-reader `ConcurrentSnapshot`.

## Downstream temporal integration

Connectivity implementations may produce canonical `MonotonicCaptureBounds`; coordinators may retain `ClockEra` and Era-qualified values and invoke transitions. These are downstream consumers, not reverse package dependencies. Native capture conversion and full Era provenance remain outside EDP-Clock.

## Application-wide bindings and ownership

`BindMonotonicClock()` and `BindSynchronizedClock()` establish process-lifetime borrowed bindings during Bootstrap. Providers borrow their lower-level provider instances and own no timer hardware or Platform provider lifetime.

> Dependency contract re-audited for temporal provenance at EDP-Clock `c3cb4436f0bd831c7800ca3669ce8342db9b1b72`.
