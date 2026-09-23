# Public API

The principal public surface includes monotonic timestamp and duration value types, the monotonic timebase/clock provider vocabulary, synchronized-clock observations/readings/state, ClockCorrelation and Stopwatch.

Monotonic timestamps use the canonical nanosecond coordinate. Duration is signed and Stopwatch elapsed time is non-negative and saturating. Synchronized readings carry timestamp, uncertainty and synchronization state together so consumers do not accidentally discard confidence information.

Exact declarations, template parameters and signatures remain authoritative in the headers exported by `ESPressio_Clock.hpp`.
