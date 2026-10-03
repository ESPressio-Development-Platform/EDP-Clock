#pragma once

#include <cstdint>
#include <type_traits>

#include "ClockCorrelationProjectionStatus.hpp"
#include "ClockEra.hpp"
#include "EraQualifiedClockCorrelationProjection.hpp"
#include "SynchronizationUncertainty.hpp"
#include "SynchronizedTimestamp.hpp"
#include "TemporalQualityAssessment.hpp"

namespace ESPressio::Clock {

    /// Compact immutable temporal projection plus application-policy assessment.
    ///
    /// The projection fields are stored directly so adding one-byte assessment does not expand a
    /// frequently retained State/context value from 24 to 32 bytes through aggregate tail padding.
    class EraQualifiedClockAssessment final {
    private:

        // Era-qualified projection.

        /// Era in which the synchronized coordinate is meaningful.
        ClockEra _era;

        /// Best synchronized coordinate estimate for the occurrence.
        SynchronizedTimestamp _timestamp;

        /// Conservative uncertainty associated with the estimate.
        SynchronizationUncertainty _uncertainty;

        /// Correlation outcome associated with the estimate.
        ClockCorrelationProjectionStatus _projectionStatus{
            ClockCorrelationProjectionStatus::CorrelationUnavailable
        };

        // Policy assessment.

        /// Application-policy assessment of the temporal evidence.
        TemporalQualityAssessment _assessment{TemporalQualityAssessment::Unreliable};

        /// Explicit zeroed padding keeps the 24-byte representation deterministic.
        std::uint16_t _reserved{0U};

    public:

        // Construction.

        /// Creates unavailable, unreliable temporal evidence.
        constexpr EraQualifiedClockAssessment() noexcept = default;

        /// Creates one policy assessment over immutable era-qualified projection evidence.
        constexpr EraQualifiedClockAssessment(
            const EraQualifiedClockCorrelationProjection& projection,
            TemporalQualityAssessment assessment
        ) noexcept :
            _era(projection.Era()),
            _timestamp(projection.Projection().Timestamp()),
            _uncertainty(projection.Projection().Uncertainty()),
            _projectionStatus(projection.Projection().Status()),
            _assessment(assessment) {}


        // Projection access.

        /// Returns the opaque temporal era of the assessed occurrence.
        constexpr const ClockEra& Era() const noexcept {
            return _era;
        }

        /// Returns the synchronized coordinate estimate carried by this assessment.
        constexpr const SynchronizedTimestamp& Timestamp() const noexcept {
            return _timestamp;
        }

        /// Returns the conservative uncertainty bound carried by this assessment.
        constexpr const SynchronizationUncertainty& Uncertainty() const noexcept {
            return _uncertainty;
        }

        /// Returns the correlation status carried by this assessment.
        constexpr ClockCorrelationProjectionStatus ProjectionStatus() const noexcept {
            return _projectionStatus;
        }

        /// Indicates whether the carried projection has valid era and coordinate evidence.
        constexpr bool IsCorrelated() const noexcept {
            return
                _era.IsValid() &&
                (_projectionStatus == ClockCorrelationProjectionStatus::Correlated);
        }


        // Assessment access.

        /// Returns the application-policy assessment of the temporal evidence.
        constexpr TemporalQualityAssessment Assessment() const noexcept {
            return _assessment;
        }

    };


    static_assert(
        sizeof(EraQualifiedClockAssessment) == 24U,
        "EraQualifiedClockAssessment must remain a compact 24-byte context value"
    );

    static_assert(
        std::is_trivially_copyable_v<EraQualifiedClockAssessment>,
        "EraQualifiedClockAssessment must remain trivially copyable"
    );

} // ESPressio::Clock

