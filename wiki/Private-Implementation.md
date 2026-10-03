# Private Implementation

`SynchronizedClockProvider` retains one borrowed monotonic provider pointer and one fixed `ConcurrentSnapshot<SynchronizedClockState>`. The logical state remains exactly 48 bytes; the new published-timeline bit reuses former reserved storage. There is no heap, worker, callback registry or source table.

Projection separates the accepted reference anchor from the published non-regressing anchor. Same-Era transition changes quality while preserving both mappings. Cross-Era preservation first projects the current public coordinate, then publishes a state with no accepted reference, saturated uncertainty, zero old-source phase/rate correction and retained timeline authority. Reconstruction publishes the initial state.

`EraQualifiedClockAssessment` stores projection fields directly instead of nesting the 24-byte projection object. This uses 24 bytes rather than 32 while retaining explicit reserved bytes and trivial copyability.

Private arithmetic must continue to saturate rather than wrap and must never subtract or compare unrelated timestamp domains.
