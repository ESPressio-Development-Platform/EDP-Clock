# Resources, Lifecycle and Concurrency

Clock state is fixed-size and allocation-free in the core library. Synchronized publication uses Platform AtomicWord32 and ConcurrentSnapshot rather than heap-backed synchronization. No blanket ISR-safety claim is made. Stopwatch lifecycle is Start/Stop/Reset/Restart over a borrowed monotonic clock and contains only bounded state.
