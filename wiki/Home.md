# EDP-Clock Developer Wiki

EDP-Clock owns ESPressio's canonical monotonic time coordinate, synchronized-time discipline, compact temporal provenance/capture values, immutable correlation and Stopwatch semantics.

It deliberately does not own calendars, civil time, source selection, Mesh lineage, provider-native timestamp extension or application admission policy. The Wiki is maintained beside the code on the branch it documents. Source plus repository `docs/` remain normative; this Wiki is the internal developer and exhaustive-reference layer.

## Public entry point

```cpp
#include <ESPressio_Clock.hpp>
```

## Dependencies

Mandatory package dependencies are EDP-System and EDP-Platform. Temporal provenance adds no Radio, Mesh or identity dependency.

## Developer map

Start with [Architecture](Architecture), [Public API](Public-API), [Composition](Composition), and [Resources / Lifecycle / Concurrency](Resources-Lifecycle-Concurrency). Maintainers should also read [Internal API](Internal-API), [Private Implementation](Private-Implementation), [Implementation](Implementation), [Dependency Contracts](Dependency-Contracts), [Build / Test / Source](Build-Test-Source), and the exhaustive [Reference Index](Reference-Index).
