#pragma once

#include <cstdint>

namespace ESPressio::Clock {

    /// Availability state of one immutable clock correlation snapshot.
    enum class ClockCorrelationState : std::uint8_t {
        Unavailable = 0U,
        Available = 1U
    };

} // ESPressio::Clock
