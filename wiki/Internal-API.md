# Internal API

Internal machinery owns exact count-to-nanosecond conversion, four-timestamp estimation, integer-only frequency/phase discipline, uncertainty growth, coherent Platform snapshot publication and transition state preparation.

The synchronized snapshot distinguishes:

- `HasObservation`: a current accepted reference mapping exists and may be exposed by `Correlation()`;
- `HasPublishedTimeline`: synchronized time has once become authoritative and ordinary correction must not regress it.

This distinction allows cross-Era preservation to invalidate reference correlation without stopping the public coordinate, and prevents a merely provisional never-qualified observation from gaining non-regression authority.

These helpers and snapshot fields are not consumer extension points. Other repositories consume public Clock values/provider methods and must not depend on `Detail` arithmetic or layout beyond the explicit public size constants/assertions.
