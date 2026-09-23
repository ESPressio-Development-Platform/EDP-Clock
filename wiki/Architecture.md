# Architecture

The library separates four concerns. Monotonic timebase providers expose coherent non-regressing native counts plus an exact rational frequency. MonotonicClock converts that source into the canonical unsigned 64-bit nanosecond coordinate without cumulative rounding drift. SynchronizedClock disciplines that monotonic coordinate toward an external reference using timestamp observations, frequency correction, phase slew and explicit uncertainty. ClockCorrelation captures an immutable retrospective mapping between monotonic and reference time. Stopwatch consumes only the monotonic clock and deliberately has no synchronized-time dependency.

Synchronized state is explicitly modelled rather than inferred. The public states are NeverSynchronized, Synchronized, Holdover, Reacquiring and LostSynchronization. Published readings remain non-regressing even while the best accepted correlation may change.
