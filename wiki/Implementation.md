# Implementation Invariants

The implementation must preserve:

- exact rational source conversion without cumulative tick-rounding drift;
- non-regression of an authoritative published timeline except after explicit runtime reconstruction;
- separation of accepted reference correlation from phase-slewed publication;
- invalidation of old correlation across a cross-Era boundary;
- source-agnostic Clock values containing no Device identity, lineage, rank or provider-native tick;
- conservative unavailable/reversed capture handling and ceil-half-width uncertainty;
- one-writer/many-reader fixed snapshot publication;
- fixed public layouts guarded by `static_assert`; and
- zero heap allocation, waits, callbacks or hidden execution contexts in Clock operations.

Upstream code must serialize observations and both transition methods. Clock deliberately does not protect multiple competing writers.
