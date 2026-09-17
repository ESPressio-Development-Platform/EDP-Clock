#pragma once

#include <cstdint>

namespace ESPressio::Clock {

    /// Lifecycle and quality state reported with each synchronized clock reading.
    enum class SynchronizationState : std::uint8_t {
        NeverSynchronized = 0,
        Synchronized = 1,
        Holdover = 2,
        Reacquiring = 3,
        LostSynchronization = 4
    };

} // ESPressio::Clock
