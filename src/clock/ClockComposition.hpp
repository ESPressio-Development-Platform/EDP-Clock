#pragma once

#include <cstdint>

#include <ESPressio_System.hpp>

namespace ESPressio::Clock {

    namespace Framework = ESPressio::System::CompositionFramework;

    /// Composition domain containing physical and electronic Clock capabilities.
    struct Domain final : Framework::Domain {};


    /// Exclusive capability supplying the application-selected monotonic physical timebase.
    ///
    /// A conforming provider exposes a `std::uint64_t CurrentCount() const noexcept` operation.
    /// Concurrent calls to CurrentCount() must be safe, coherent, and non-regressing. Any raw
    /// hardware rollover must be hidden by the concrete provider before the count is exposed.
    struct MonotonicTimebase final : Framework::ExclusiveCapability<Domain> {};


    /// Exact numerator of the MonotonicTimebase nominal count frequency in counts per second.
    struct TimebaseFrequencyNumerator final : Framework::Property<MonotonicTimebase, std::uint32_t> {};


    /// Exact denominator of the MonotonicTimebase nominal count frequency in counts per second.
    struct TimebaseFrequencyDenominator final : Framework::Property<MonotonicTimebase, std::uint32_t> {};


    /// Exclusive capability supplying canonical nanosecond monotonic timestamps.
    struct MonotonicClock final : Framework::ExclusiveCapability<Domain> {};


    /// Conservative whole-nanosecond resolution of the MonotonicClock capability.
    struct ClockResolutionNanoseconds final : Framework::Property<MonotonicClock, std::uint64_t> {};

} // ESPressio::Clock
