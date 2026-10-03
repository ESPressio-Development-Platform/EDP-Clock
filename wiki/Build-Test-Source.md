# Build, Test and Source Map

C++20 is required.

- `src/` owns public Clock contracts and header-only implementation.
- `docs/` contains normative monotonic, synchronized, correlation, temporal-provenance and Stopwatch contracts.
- `tests/synchronized/ClockEraTests.cpp` validates exact new layouts, Era validity/comparison, unavailable/reversed/odd/full-range capture bounds and Era-qualified projection/assessment.
- `tests/synchronized/SynchronizedClockTests.cpp` validates same-Era reacquisition, cross-Era correlation invalidation, non-regression, fail-closed saturated quality and runtime reconstruction.
- `demos/temporal-provenance-basic` supplies Arduino IDE, PlatformIO Arduino and PlatformIO ESP-IDF targets.

The complete host suite is run by `tests/run_host_tests.sh` with explicit EDP-System and EDP-Platform source paths. The current contract checkpoint passed local GCC C++20 plus ASan/UBSan; authoritative appliance, target compiler/layout and demo target-build evidence remains pending and must not be inferred from host success.
