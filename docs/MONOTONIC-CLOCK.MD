# Monotonic Clock Foundation

## Contents

1. [Purpose](#purpose)
2. [Domain Boundary](#domain-boundary)
3. [Dependency Boundary](#dependency-boundary)
4. [Foundational Model](#foundational-model)
5. [MonotonicTimestamp](#monotonictimestamp)
6. [Duration](#duration)
7. [Delta](#delta)
8. [MonotonicTimebase](#monotonictimebase)
9. [Exact Timebase Frequency](#exact-timebase-frequency)
10. [MonotonicClock](#monotonicclock)
11. [Count-to-Nanosecond Conversion](#count-to-nanosecond-conversion)
12. [Clock Resolution](#clock-resolution)
13. [Thread Safety](#thread-safety)
14. [Bootstrap and Lifetime](#bootstrap-and-lifetime)
15. [MonotonicNow](#monotonicnow)
16. [Composition and Application Choice](#composition-and-application-choice)
17. [Hard-Coded Equivalent and Coupling](#hard-coded-equivalent-and-coupling)
18. [Complete Example](#complete-example)
19. [Memory Model](#memory-model)
20. [Platform and RTC Integration](#platform-and-rtc-integration)
21. [What This Tranche Does Not Contain](#what-this-tranche-does-not-contain)
22. [Relationship to Synchronized Clock](#relationship-to-synchronized-clock)
23. [Common Mistakes](#common-mistakes)

---

## Purpose

The Monotonic Clock foundation establishes the lowest Clock-domain concepts upon which later ESPressio timing behaviour can safely depend.

It answers four basic questions:

- What is the application's selected physical/electronic timebase?
- How is that timebase converted into a canonical monotonic clock coordinate?
- How are two monotonic coordinates compared as a physical duration?
- How can application code obtain the current monotonic time without carrying a Clock pointer in every consumer?

The design is intentionally small because Clock reads are expected to be frequent and ESPressio targets include devices with extremely constrained DRAM.

This first tranche contains only facilities whose semantics should remain stable regardless of how synchronized time, Stopwatch, timers, schedulers, or discipline algorithms are layered above it.

---

## Domain Boundary

`EDP-Clock` models **physical/electronic time**.

It does not model human interpretations of time.

The Clock domain therefore knows about concepts such as:

```text
physical progression
frequency
counter values
nanoseconds
duration
monotonicity
resolution
clock synchronization quality (provided by the synchronized-clock layer)
```

It deliberately does not know about:

```text
year
month
day
calendar
Gregorian rules
time zone
DST
civil time
formatted date/time
RTC register layouts
RTC battery state
```

If another library later maps Clock-domain values to human-interpreted time, that responsibility remains downstream from `EDP-Clock`.

This distinction is architectural, not merely naming preference.

---

## Dependency Boundary

The first Clock tranche depends on:

```text
EDP-System
    └── Composition Framework

C++ standard language/library facilities
    └── fixed-width integers
    └── numeric limits
    └── type traits
```

It does **not** depend on:

```text
Arduino
ESP-IDF
FreeRTOS
esp_timer
GPTimer
micros()
millis()
xTaskGetTickCount()
platform interrupt APIs
platform critical-section APIs
RTC libraries
std::chrono
```

Those facilities may be used by a concrete provider in another library, but they are not Clock-domain dependencies.

The monotonic facility itself requires no Platform concurrency primitive. The current `EDP-Clock` repository also contains `SynchronizedClock`, which consumes `EDP-Platform` for coherent lock-free discipline-state publication; no platform-specific SDK is exposed to the monotonic layer.

---

## Foundational Model

The first tranche is intentionally layered as follows:

```text
MonotonicTimebase
        |
        | application-selected physical/electronic progression
        v
MonotonicClock
        |
        | exact count -> canonical nanosecond conversion
        v
MonotonicTimestamp
        |
        | Delta(from, to)
        v
Duration
```

There are two Composition capabilities:

```cpp
ESPressio::Clock::MonotonicTimebase
ESPressio::Clock::MonotonicClock
```

And three foundational value/arithmetic facilities:

```cpp
ESPressio::Clock::MonotonicTimestamp
ESPressio::Clock::Duration
ESPressio::Clock::Delta(...)
```

Finally, the application-wide convenience surface provides:

```cpp
ESPressio::Clock::MonotonicNow()
```

---

## MonotonicTimestamp

`MonotonicTimestamp` is an 8-byte value representing a position on the application's local monotonic clock coordinate system.

Its canonical unit is **nanoseconds**.

Conceptually:

```text
opaque local origin                         current coordinate
      |--------------------------------------------|
      0 ns                                  8,471,223,991 ns
```

The origin is the clock's **epoch**, but that epoch is deliberately opaque.

A consumer does not need to know whether the origin corresponds to:

```text
power-on
hardware reset
counter initialization
platform initialization
another implementation-defined event
```

The only important contract is that coordinates do not regress.

### Zero is valid

A timestamp of zero means:

```text
this monotonic coordinate is exactly at the local clock origin
```

It does **not** mean:

```text
invalid
unset
unknown
```

This is different from identity values where zero may be reserved as invalid.

### Exact memory

The implementation enforces:

```cpp
sizeof(MonotonicTimestamp) == 8
```

It is trivially copyable and owns no resources.

---

## Duration

`Duration` represents a signed physical interval.

Its canonical unit is also nanoseconds.

The underlying representation is:

```cpp
std::int64_t
```

so:

```text
positive duration  -> movement forward from the first coordinate
zero duration      -> same coordinate
negative duration  -> movement backward from the first coordinate
```

This makes the direction of `Delta()` explicit without unsigned underflow.

`Duration` is exactly 8 bytes, trivially copyable, and has no dynamic state.

A signed 64-bit nanosecond duration spans approximately ±292 years, which is far beyond ordinary embedded-runtime interval requirements.

---

## Delta

`Delta()` calculates the physical difference between two monotonic timestamps:

```cpp
const auto elapsed = ESPressio::Clock::Delta(
    from,
    to
);
```

Its semantic definition is:

```text
Delta(from, to) = to - from
```

For example:

```text
from = 1,000 ns
to   = 4,500 ns

Delta(from, to) = +3,500 ns
```

Reversing the arguments gives:

```text
Delta(to, from) = -3,500 ns
```

The ordinary contract expects the requested difference to fit inside the signed 64-bit `Duration` range.

As a defensive boundary, a difference beyond that representable range is clamped to `Duration::Minimum()` or `Duration::Maximum()` instead of permitting integer overflow.

`Delta()` performs no allocation and has no clock dependency. It operates only on already-captured values.

---

## MonotonicTimebase

`MonotonicTimebase` is the application's selected source of physical/electronic progression.

It is an `ExclusiveCapability` in the Clock Composition domain:

```cpp
struct MonotonicTimebase final :
    ESPressio::System::CompositionFramework::ExclusiveCapability<Domain> {};
```

Only one provider may therefore supply the application Clock domain's selected monotonic timebase.

### Why Timebase is separate from Clock

The distinction is deliberate.

The application may decide that its best physical source is:

```text
MCU hardware timer
free-running peripheral counter
external oscillator
RTC fixed-frequency output
precision hardware counter
FPGA counter
application-specific source
```

That decision belongs to the application and its Composition.

`EDP-Clock` must not impose a platform-selected source.

### Provider operation

A concrete provider exposes:

```cpp
std::uint64_t CurrentCount() const noexcept;
```

The value is a **count**, not a timestamp.

Its physical meaning is supplied by the provider's exact nominal frequency.

### Required provider guarantees

A conforming provider guarantees that `CurrentCount()` is:

- coherent;
- non-regressing;
- safe for concurrent readers;
- `noexcept`;
- extended across underlying hardware rollover for the supported application lifetime.

If a raw device exposes a 16-bit, 32-bit, or other wrapping counter, hiding that rollover is the responsibility of the concrete timebase provider.

`MonotonicClock` does not know the raw hardware width.

### Convenience provider declaration

A concrete provider can declare its exact frequency using:

```cpp
class MyTimebase final :
    public ESPressio::Clock::MonotonicTimebaseProvider<1000000U> {

public:
    std::uint64_t CurrentCount() const noexcept {
        // Read the selected physical source.
    }
};
```

The provider above advertises exactly 1,000,000 counts per second.

---

## Exact Timebase Frequency

The timebase frequency is represented as an exact rational value:

```text
counts per second = numerator / denominator
```

The provider declaration therefore accepts:

```cpp
MonotonicTimebaseProvider<
    FrequencyNumerator,
    FrequencyDenominator
>
```

The denominator defaults to `1`.

Examples:

```cpp
// Exactly 1 MHz.
MonotonicTimebaseProvider<1000000U>

// Exactly 32.768 kHz.
MonotonicTimebaseProvider<32768U>

// Exactly 1.5 counts per second.
MonotonicTimebaseProvider<3U, 2U>
```

The numerator and denominator are 32-bit unsigned compile-time values.

This deliberately covers physical frequencies up to more than 4 GHz while allowing exact rational representation without requiring 128-bit runtime arithmetic.

### Why Clock does not store nanoseconds-per-count

A 32.768 kHz source has a period of:

```text
1,000,000,000 ns / 32,768
= 30,517.578125 ns
```

If Clock incorrectly reduced that to:

```text
30,517 ns per count
```

and repeatedly accumulated that rounded value, the Clock would manufacture drift.

Instead, EDP-Clock retains the exact frequency relationship and always converts the absolute source count.

---

## MonotonicClock

`MonotonicClock` is a separate `ExclusiveCapability`.

Downstream code should normally consume the Clock, not the physical timebase.

The standard implementation is:

```cpp
MonotonicClockProvider<TTimebaseProvider>
```

It declares:

```text
Provides:
    MonotonicClock

Requires:
    MonotonicTimebase
```

This establishes the architectural boundary:

```text
Application Bootstrap chooses physical source
                    |
                    v
            MonotonicTimebase
                    |
                    v
       generic EDP-Clock conversion
                    |
                    v
             MonotonicClock
                    |
                    v
             downstream code
```

### Runtime state

The generic provider retains exactly one borrowed pointer/reference-sized relationship to its selected timebase.

It does not own:

```text
hardware
heap memory
thread
mutex
semaphore
observer list
callback registry
timestamp accumulator
rounded tick remainder
```

Its `Now()` operation performs:

```text
read CurrentCount() exactly once
        |
        v
exact count-to-nanosecond conversion
        |
        v
return MonotonicTimestamp
```

The Clock itself therefore has no mutable timing state that needs synchronization.

---

## Count-to-Nanosecond Conversion

The mathematical conversion is:

```text
nanoseconds = floor(
    count * 1,000,000,000 * FrequencyDenominator
    ------------------------------------------------
                 FrequencyNumerator
)
```

The implementation does **not** repeatedly add a rounded per-count period.

Instead, it decomposes the calculation using integer quotient and remainder arithmetic.

This provides four important properties:

1. no floating-point dependency;
2. no cumulative rounding drift;
3. no 128-bit integer requirement;
4. conversion is derived from the absolute physical count on every read.

### 32.768 kHz example

For one source count:

```text
floor(1,000,000,000 / 32,768)
= 30,517 ns
```

For exactly 32,768 counts:

```text
floor(32,768 * 1,000,000,000 / 32,768)
= 1,000,000,000 ns
```

The fractional part lost from the first individual coordinate does not accumulate as clock drift because each coordinate is derived independently from the absolute count.

### Canonical-range exhaustion

A 64-bit unsigned nanosecond timestamp spans approximately 584 years.

If a timebase count mathematically exceeds the canonical timestamp range, the conversion clamps to the maximum `MonotonicTimestamp` coordinate instead of wrapping backwards.

This is a defensive non-regression boundary, not an expected embedded-runtime condition.

---

## Clock Resolution

`MonotonicClock` advertises:

```cpp
ClockResolutionNanoseconds
```

as a Composition property.

The value is the conservative whole-nanosecond interval required to distinguish successive physical source counts:

```text
resolution = ceil(
    1,000,000,000 * denominator
    --------------------------------
              numerator
)
```

For a 32.768 kHz source:

```text
resolution = 30,518 ns
```

The exact timestamp conversion still uses the rational source frequency. The resolution property does **not** pretend each source count is exactly 30,518 ns.

This property is important for later Clock capabilities.

The synchronized-clock provider uses this property to enforce its strict sub-millisecond source requirement with a Composition constraint equivalent to:

```cpp
LessThan<
    ClockResolutionNanoseconds,
    1000000U
>
```

The boundary is deliberately **strictly less than** one millisecond.

---

## Thread Safety

Monotonic Clock reads are expected to cross thread/task boundaries and may be high frequency.

Thread safety is divided deliberately between layers.

### Timebase provider responsibility

The concrete `MonotonicTimebase` provider must make:

```cpp
CurrentCount()
```

safe for concurrent readers.

On a 32-bit MCU, a provider must not simply assume an arbitrary 64-bit software counter can be read atomically while another execution context updates it.

A concrete implementation may use whatever mechanism is appropriate to its own platform:

```text
hardware atomic snapshot
critical section
versioned/seqlock-like snapshot
platform atomic primitive
another suitable implementation
```

That implementation detail belongs outside `EDP-Clock`.

### MonotonicClock responsibility

The generic Clock:

- performs one `CurrentCount()` call;
- owns no mutable clock state;
- performs deterministic local arithmetic;
- returns an immutable value.

It therefore adds no mutex, atomic counter, worker, or other synchronization mechanism to the high-frequency read path.

### Value types

`MonotonicTimestamp` and `Duration` are immutable-by-value observations once captured.

They can be copied between threads like other ordinary 8-byte values. Synchronization of the surrounding application's data ownership remains the application's responsibility.

### ISR safety is separate

The Clock-domain concurrency contract requires safe concurrent thread/task reads.

It does not automatically assert that every concrete timebase provider is callable from an ISR.

ISR suitability is a separate provider/platform concern and is not claimed by this tranche.

---

## Bootstrap and Lifetime

The expected application order is:

```text
construct/initialize selected physical timebase
                 |
                 v
construct generic MonotonicClockProvider
                 |
                 v
bind application-wide MonotonicNow()
                 |
                 v
start code that consumes monotonic time concurrently
```

The selected timebase and the generic Clock must remain alive for every consumer that may access them.

The selected timebase does not change during the running lifetime.

The `MonotonicClockProvider` is intentionally non-copyable and non-movable so later consumers may safely borrow it after Bootstrap.

`BindMonotonicClock()` is also one-way: after a Clock has been bound, another binding is refused.

This is initialization ordering rather than runtime synchronization.

---

## MonotonicNow

Most application code should not need to retain its own Clock pointer merely to interrogate monotonic time.

The application-wide convenience call is:

```cpp
const auto now = ESPressio::Clock::MonotonicNow();
```

Bootstrap establishes it once:

```cpp
MyTimebase timebase;
ESPressio::Clock::MonotonicClockProvider<MyTimebase> clock(timebase);

const bool bound = ESPressio::Clock::BindMonotonicClock(clock);
```

The binding retains:

```text
one borrowed context pointer
one function pointer
```

On an ordinary 32-bit ESP32 ABI this is typically 8 bytes of application-wide retained state.

This fixed application-wide cost avoids requiring every persistent consumer to retain its own Clock pointer.

### No vtable

The convenience binding performs small type erasure using a context pointer and generated function thunk.

It does not require:

```text
virtual inheritance
vtable
heap allocation
service registry
runtime capability lookup
```

### Precondition

`MonotonicNow()` is deliberately optimized for the high-frequency valid path.

It assumes Bootstrap has already completed successful binding.

Calling it before `BindMonotonicClock()` succeeds is an application lifecycle error.

Consumers that need to verify Bootstrap state may query:

```cpp
ESPressio::Clock::IsMonotonicClockBound()
```

Normal application code should not need to poll this state after Bootstrap.

---

## Composition and Application Choice

A complete Clock composition might be:

```cpp
using ApplicationClock =
    ESPressio::Clock::MonotonicClockProvider<ApplicationTimebase>;

using ApplicationComposition =
    ESPressio::System::CompositionFramework::Composition<
        ESPressio::Clock::Domain,
        ApplicationTimebase,
        ApplicationClock
    >;
```

This allows Composition to validate that:

```text
one MonotonicTimebase exists
one MonotonicClock exists
the MonotonicClock requirement for MonotonicTimebase is satisfied
clock properties are available to later compile-time consumers
```

Composition selects architecture.

Bootstrap still owns runtime construction and lifetime.

---

## Hard-Coded Equivalent and Coupling

Without Composition, an application could hard-code a platform timer directly:

```cpp
class StopwatchConsumer final {
public:
    Duration Elapsed() const noexcept {
        const auto raw = SomePlatformTimerRead();
        // Convert platform-specific units here.
    }
};
```

This appears simple, but it creates several forms of coupling.

### Hardware/platform coupling

The consumer now knows which platform API supplies physical time.

Changing from:

```text
MCU timer
```

to:

```text
external oscillator
RTC frequency output
precision counter
```

requires changing consuming code rather than Bootstrap composition.

### Unit coupling

Every consumer may need to know whether the source reports:

```text
microseconds
counter ticks
CPU cycles
RTOS ticks
another source-specific unit
```

That duplicates conversion logic and creates inconsistent rounding behaviour.

### Thread-safety coupling

Every consumer must know whether the underlying hardware/API can be read coherently across execution contexts.

### Dependency coupling

Reusable code begins including platform headers simply because it needs elapsed time.

### Bootstrap disappears into implementation detail

There is no single application-level point at which the physical timebase decision is visible and reviewable.

The Composition-based design instead produces:

```text
consumer
    |
    v
MonotonicClock capability
    |
    v
generic Clock semantics
    |
    v
MonotonicTimebase capability
    |
    v
application-selected concrete provider
```

A reusable consumer depends on the Clock concept rather than a hardware decision.

---

## Complete Example

The repository contains the complete example at:

```text
examples/monotonic-basic/main.cpp
```

The important flow is reproduced below.

### 1. Define a timebase provider

```cpp
class ApplicationTimebase final :
    public ESPressio::Clock::MonotonicTimebaseProvider<1000000U> {

private:
    std::uint64_t _count{0U};

public:
    std::uint64_t CurrentCount() const noexcept {
        return _count;
    }

    void Advance(
        std::uint64_t count
    ) noexcept {
        _count += count;
    }
};
```

This example provider represents an exact 1 MHz source.

A real provider would normally reside in the appropriate platform, hardware, RTC, or application library and would implement the required concurrent-read guarantee.

### 2. Select the generic Clock provider

```cpp
using ApplicationClock =
    ESPressio::Clock::MonotonicClockProvider<ApplicationTimebase>;
```

### 3. Validate the architecture

```cpp
using ApplicationComposition =
    ESPressio::System::CompositionFramework::Composition<
        ESPressio::Clock::Domain,
        ApplicationTimebase,
        ApplicationClock
    >;
```

### 4. Bootstrap the instances

```cpp
ApplicationTimebase timebase;
ApplicationClock monotonicClock(timebase);

if (!ESPressio::Clock::BindMonotonicClock(monotonicClock)) {
    // Bootstrap failed because another clock was already bound.
}
```

### 5. Consume monotonic time without carrying a pointer

```cpp
const auto started = ESPressio::Clock::MonotonicNow();

// Work occurs.

const auto finished = ESPressio::Clock::MonotonicNow();

const auto elapsed = ESPressio::Clock::Delta(
    started,
    finished
);
```

---

## Memory Model

The first tranche is intentionally memory-bounded.

| Facility | Retained Clock-domain state |
| --- | ---: |
| `MonotonicTimestamp` | 8 bytes per captured value |
| `Duration` | 8 bytes per captured value |
| `MonotonicClockProvider<T>` | one borrowed pointer/reference-sized field |
| application-wide `MonotonicNow()` binding | one context pointer + one function pointer |
| `Delta()` | no retained state |
| count conversion | no retained state |

There is no Clock-owned dynamic allocation.

There are no Clock-owned workers, mutexes, queues, observers, callback collections, or accumulated fractional-tick buffers.

Concrete timebase providers may require their own minimal hardware/concurrency state, but that memory belongs to the selected implementation rather than the Clock abstraction.

---

## Platform and RTC Integration

`EDP-Clock` deliberately knows nothing about RTC semantics or platform timer APIs.

The current dependency direction is deliberately layered:

```text
EDP-System
    ^
    |
EDP-Platform
    ^
    |
EDP-Clock
    ^
    |
    +-----------------------+
    |                       |
concrete timebase       future EDP-RTC / other source library
provider                may satisfy Clock-facing source contracts
```

The important point is not the eventual repository names.

The important rule is:

> A downstream library may satisfy a Clock capability, but EDP-Clock does not acquire that downstream library's domain semantics.

For example, an RTC implementation may expose a fixed-frequency oscillator/counter as a `MonotonicTimebase`.

Clock sees only:

```text
count
exact frequency
monotonic/thread-safe guarantees
```

It does not know that the source is an RTC.

---

## What This Tranche Does Not Contain

The monotonic tranche intentionally does not itself implement:

```text
synchronized clock discipline
Stopwatch
Timer
TimerScheduler
MonotonicAlarm
calendar/date/time semantics
RTC semantics
platform timer implementations
```

`SynchronizedClock` is now implemented as a separate layer on top of this stable monotonic foundation. It consumes `MonotonicClock` rather than changing the semantics of `MonotonicTimebase`, `MonotonicTimestamp`, `Duration`, or `Delta()`.

---

## Relationship to Synchronized Clock

The implemented synchronized layer builds directly upon the contracts defined here:

```cpp
SynchronizedClock
SynchronizedTimestamp
SynchronizedReading
SynchronizedNow()
```

The distinction remains deliberately explicit:

```text
MonotonicNow()
    -> immediate local monotonic coordinate

SynchronizedNow()
    -> shared synchronized coordinate together with uncertainty and lifecycle state
```

`SynchronizedClock` maintains no second physical counter. It projects the same `MonotonicClock` through an observation-driven frequency/phase mapping and publishes that mapping through `EDP-Platform::Concurrency::ConcurrentSnapshot`.

A synchronized reading is still not human wall-clock/date-time information. It remains a physical Clock-domain coordinate with a shared but opaque epoch.

The hard quality invariant is:

```text
State == Synchronized
    implies
Uncertainty < 1,000,000 nanoseconds
```

See [SYNCHRONIZED-CLOCK.MD](./SYNCHRONIZED-CLOCK.MD) for the complete discipline, lifecycle, concurrency, and memory model.

---

## Common Mistakes

### Treating zero as invalid

Wrong:

```cpp
if (timestamp.Nanoseconds() == 0U) {
    // Assume there is no timestamp.
}
```

Zero is the valid opaque monotonic origin.

### Treating the timebase count as nanoseconds

Wrong:

```cpp
const auto nanoseconds = timebase.CurrentCount();
```

The count only gains physical meaning through its exact frequency.

### Rounding the source period once and accumulating it

Wrong for 32.768 kHz:

```text
period = 30,517 ns
current += period every count
```

This manufactures drift.

Use the generic `MonotonicClockProvider`, which converts the absolute count from the exact rational frequency.

### Reading a wrapping hardware counter directly

A timebase provider must hide raw rollover before exposing `CurrentCount()`.

### Assuming a 64-bit software read is automatically safe on a 32-bit MCU

The concrete provider is responsible for coherent concurrent acquisition.

### Putting platform-specific synchronization primitives in EDP-Clock

`SynchronizedClock` consumes the generic `EDP-Platform::Concurrency::ConcurrentSnapshot` abstraction. Platform-specific critical sections, SDK atomics, timer APIs, and ISR details still do not belong in `EDP-Clock`; concrete `AtomicWord32` mechanics stay in the appropriate `EDP-Platform-*` provider.

### Rebinding MonotonicNow at runtime

The application-wide binding is permanent for the running lifetime.

### Giving the monotonic epoch human meaning

The origin is intentionally opaque.

### Introducing calendar concepts

Calendar/date/time interpretation is outside the Clock domain.

---

## Summary

The first Clock tranche establishes a deliberately small physical-time foundation:

```text
application-selected MonotonicTimebase
            |
            v
generic MonotonicClock
            |
            v
8-byte MonotonicTimestamp
            |
            +---- Delta() ----> 8-byte signed Duration
            |
            v
application-wide MonotonicNow()
```

The design preserves exact source frequency, avoids cumulative rounding drift, keeps platform-specific mechanisms outside the monotonic abstraction, makes the physical timebase an application-level decision, and adds no unnecessary dynamic memory or synchronization machinery to the monotonic read path.
