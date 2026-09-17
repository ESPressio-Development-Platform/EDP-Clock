#pragma once

#include <cstdint>

namespace ESPressio::Clock {

    /// Result status produced when projecting a monotonic occurrence through a ClockCorrelation.
    enum class ClockCorrelationProjectionStatus : std::uint8_t {
        Correlated = 0U,
        CorrelationUnavailable = 1U,
        SynchronizedCoordinateUnderflow = 2U,
        SynchronizedCoordinateOverflow = 3U
    };

} // ESPressio::Clock
