# EDP-Clock Dependency Graph

## Library Dependency Tree

```text
EDP-Clock
├── EDP-System [mandatory]
└── EDP-Platform [mandatory]
    └── EDP-System [mandatory, transitive]
```

## Mandatory Dependencies

- `EDP-System` — provides the Composition Framework used by Clock capabilities and providers.
- `EDP-Platform` — provides the `AtomicWord32` contract and `ConcurrentSnapshot` used by synchronized-clock discipline publication.

## Optional Dependencies

`EDP-Clock` currently has no optional ESPressio library dependencies.

A consuming application that instantiates `SynchronizedClockProvider` must select a concrete `AtomicWord32` provider satisfying `EDP-Platform`. Typical target selections include:

```text
Arduino application
└── EDP-Platform-Arduino [application Composition choice]

ESP-IDF application
└── EDP-Platform-ESP-IDF [application Composition choice]

FreeRTOS-oriented application
└── EDP-Platform-FreeRTOS [application Composition choice]
```

Those concrete providers are application-level capability satisfiers rather than direct or optional dependencies of the `EDP-Clock` library itself.

The demo projects select target-appropriate concrete providers as demo/build dependencies.
