# Stopwatch

## Purpose

`Stopwatch` is the Clock-domain utility for measuring elapsed physical time across one or more running intervals.

It consumes an application-selected `MonotonicClock` provider directly. It does not provide a new Composition capability and it has no relationship to synchronized time, Clock Correlation, RTC state, calendar time, alarms, scheduling, or task delays.

The core rule is simple:

```text
Stopwatch measures elapsed MonotonicClock time only.
```

Synchronization acquisition, phase slew, rate discipline, loss of synchronization, and source changes therefore cannot alter an in-progress Stopwatch measurement.

---

## Construction

A Stopwatch is explicitly bound to one lifetime-stable MonotonicClock provider:

```cpp
using ApplicationStopwatch = ESPressio::Clock::Stopwatch<ApplicationClock>;

ApplicationStopwatch stopwatch(monotonicClock);
```

The provider is borrowed, never owned. Application Bootstrap remains responsible for ensuring the provider outlives every Stopwatch that references it.

At construction:

```text
state   = stopped
elapsed = 0
```

Construction performs no Clock read.

The supplied provider is compile-time validated to ensure that it advertises the existing `MonotonicClock` capability and exposes:

```cpp
MonotonicTimestamp Now() const noexcept;
```

`Stopwatch` itself is intentionally not a provider. It is a consumer-side measurement utility rather than part of application architecture selection.

---

## Lifecycle

### Start

```cpp
stopwatch.Start();
```

When stopped, `Start()` captures one monotonic timestamp and begins accumulating elapsed time.

If elapsed time was already accumulated before the previous `Stop()`, `Start()` resumes from that value rather than clearing it.

Calling `Start()` while already running is a no-op and performs no Clock read.

### Stop

```cpp
stopwatch.Stop();
```

When running, `Stop()` samples the MonotonicClock exactly once, accumulates the interval since the latest `Start()` or `Restart()`, and freezes the result.

Calling `Stop()` while already stopped is a no-op and performs no Clock read.

### Reset

```cpp
stopwatch.Reset();
```

`Reset()`:

```text
sets elapsed to zero
sets state to stopped
performs no Clock read
```

### Restart

```cpp
stopwatch.Restart();
```

`Restart()` clears all accumulated elapsed time, samples the MonotonicClock exactly once, and immediately begins a new interval.

It is therefore equivalent in meaning to a reset followed by a start, but requires only the one necessary Clock sample.

---

## Elapsed

```cpp
const ESPressio::Clock::Duration elapsed = stopwatch.Elapsed();
```

When stopped, `Elapsed()` returns the frozen accumulated duration and performs no Clock read.

When running, `Elapsed()` samples the MonotonicClock exactly once and returns:

```text
accumulated completed intervals
+
current monotonic coordinate - latest running anchor
```

Calling `Elapsed()` while running does not stop, restart, or otherwise mutate the measurement session.

The returned value is the existing signed Clock-domain `Duration`, but Stopwatch elapsed time is always non-negative.

---

## Saturation

`Duration` stores signed 64-bit nanoseconds, while `MonotonicTimestamp` stores an unsigned 64-bit coordinate.

Stopwatch therefore saturates at:

```text
Duration::Maximum()
INT64_MAX nanoseconds
approximately 292 years
```

rather than allowing an elapsed interval to wrap into a negative duration.

Once saturated, later running intervals remain saturated until `Reset()` or `Restart()` clears the accumulated value.

A conforming MonotonicClock is required to be non-regressing. Stopwatch nevertheless defensively treats a regressing sample as a zero-length current interval so that a broken provider cannot cause unsigned subtraction wraparound.

---

## Memory and Resource Model

Each Stopwatch retains only:

```text
one MonotonicTimestamp running anchor       8 bytes
one uint64 accumulated elapsed magnitude    8 bytes
one borrowed provider pointer               native pointer size
one uint8 running flag                      1 byte
ABI alignment padding                       platform dependent
```

There is:

```text
no heap allocation
no task
no worker
no mutex
no atomic state
no callback
no registry
no lap history
no retained synchronized-time state
```

The type is therefore appropriate for many independent measurements on constrained ESP32-class systems, subject only to ordinary object-storage cost.

---

## Copy and Move Semantics

A Stopwatch is non-copyable because copying a mutable measurement session would silently create two independent objects with identical historical state.

It is movable so ownership of a measurement session can be transferred deliberately.

After a move, the destination receives the full measurement state. The moved-from Stopwatch remains valid, stays bound to the same MonotonicClock provider, and is reset to:

```text
stopped
elapsed = 0
```

This is deliberate transfer semantics rather than the primitive-field duplication that a compiler-default move would otherwise produce.

---

## Concurrency

`Stopwatch` deliberately contains no synchronization primitive.

One Stopwatch instance is intended to have one logical owner. Concurrent mutation or simultaneous mutation/read access to the same Stopwatch requires synchronization by the consuming application.

This is separate from the MonotonicClock provider contract: the provider itself may support concurrent `Now()` calls from many consumers, while each Stopwatch remains an ordinary small mutable value owned by one consumer.

---

## Deliberate Exclusions

Stopwatch does not provide:

```text
Lap storage
Split history
Callbacks
Timeouts
Alarms
Wait or Sleep
Periodic execution
Scheduling
Synchronized timestamps
RTC or civil-time interpretation
```

A caller can obtain a non-destructive split at any point simply by calling `Elapsed()` and retaining the returned `Duration`.

Timers, deadlines, alarms, and scheduling are separate responsibilities that consume Clock rather than belonging to Stopwatch itself.
