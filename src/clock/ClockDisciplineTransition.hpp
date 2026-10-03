#pragma once

#include <cstdint>

namespace ESPressio::Clock {

    /// Cross-era behavior selected upstream by application policy and invoked on Clock discipline.
    enum class CrossEraTransitionMode : std::uint8_t {
        PreservePublishedTimeline = 0U,
        ReconstructTimeline = 1U
    };


    /// Result of applying one explicit source/era discipline transition.
    enum class ClockDisciplineTransitionStatus : std::uint8_t {
        Applied = 0U,
        NoPublishedTimeline = 1U
    };

} // ESPressio::Clock

