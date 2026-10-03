# Resources, Lifecycle and Concurrency

Clock core state is fixed-size and allocation-free. The synchronized logical snapshot remains 48 bytes and the current Platform double-buffered `ConcurrentSnapshot` storage remains 104 bytes. New public values are: ClockEra 8 bytes, MonotonicCaptureBounds 16 bytes, Era-qualified correlation 40 bytes, Era-qualified projection 24 bytes and flattened assessment 24 bytes.

Clock providers borrow lifetime-stable lower-level providers selected at Bootstrap. Era-qualified values own their bytes and no external lifetime. Correlations are immutable; consumers retain any historical mapping they need.

Exactly one upstream execution context may mutate a synchronized provider through `Observe()`, `BeginSameEraTransition()` or `BeginCrossEraTransition()`. Arbitrary readers may call `Now()` and `Correlation()` concurrently. Transition calls do not allocate, wait, schedule, invoke application code or perform I/O.

No blanket ISR-safety claim is made. Trivial value operations are bounded, but provider and Platform contracts must explicitly establish any ISR use.

Stopwatch lifecycle remains Start/Stop/Reset/Restart over a borrowed monotonic clock and bounded inline state.
