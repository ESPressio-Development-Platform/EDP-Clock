#pragma once

#include <cstdint>

namespace ESPressio::Clock {

    /// Result of applying one source-agnostic synchronization observation.
    enum class SynchronizationObservationStatus : std::uint8_t {
        Accepted = 0,
        RejectedFutureObservation = 1,
        RejectedOutOfOrderObservation = 2,
        RejectedFrequencyCorrection = 3
    };

} // ESPressio::Clock
