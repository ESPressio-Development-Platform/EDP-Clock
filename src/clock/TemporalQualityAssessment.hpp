#pragma once

#include <cstdint>

namespace ESPressio::Clock {

    /// Application-policy assessment of authenticated temporal evidence.
    ///
    /// Clock defines the compact vocabulary only. It does not choose a policy or apply admission.
    /// Strict policy refusal occurs before a Primitive is admitted and is therefore not encoded as
    /// another assessment value.
    enum class TemporalQualityAssessment : std::uint8_t {
        Unreliable = 0U,
        NotRequired = 1U,
        Reliable = 2U
    };

} // ESPressio::Clock

