# EDP-Clock Developer Wiki

EDP-Clock owns ESPressio's canonical monotonic time coordinate, synchronized-time discipline and correlation, plus stopwatch semantics built on monotonic time.

This Wiki is maintained beside the code on `main`. Source code and repository `docs/` remain the normative implementation and durable contract sources; the Wiki is the internal developer navigation and explanation layer.

## Public entry point

```cpp
#include <ESPressio_Clock.hpp>
```

## Dependencies

Mandatory: EDP-System and EDP-Platform.

## Developer map

Use [Architecture](Architecture.md), [Public API](Public-API.md), [Internal API](Internal-API.md), [Implementation](Implementation.md), [Composition](Composition.md), [Resources / Lifecycle / Concurrency](Resources-Lifecycle-Concurrency.md), and [Build / Test / Source](Build-Test-Source.md).
