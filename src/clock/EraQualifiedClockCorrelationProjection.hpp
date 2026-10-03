#pragma once

#include <type_traits>

#include "ClockCorrelationProjection.hpp"
#include "ClockEra.hpp"

namespace ESPressio::Clock {

    /// One correlation projection paired with its opaque temporal era.
    class EraQualifiedClockCorrelationProjection final {
    private:

        // Temporal provenance and projection.

        /// Era in which the projected synchronized coordinate is meaningful.
        ClockEra _era;

        /// Projected timestamp, uncertainty and correlation status.
        ClockCorrelationProjection _projection;

    public:

        // Construction.

        /// Creates unavailable era-qualified projection evidence.
        constexpr EraQualifiedClockCorrelationProjection() noexcept = default;

        /// Creates one projection paired with its temporal era.
        constexpr EraQualifiedClockCorrelationProjection(
            const ClockEra& era,
            const ClockCorrelationProjection& projection
        ) noexcept :
            _era(era),
            _projection(projection) {}


        // Value access.

        /// Returns the opaque temporal era of this projection.
        constexpr const ClockEra& Era() const noexcept {
            return _era;
        }

        /// Returns the underlying synchronized-coordinate projection.
        constexpr const ClockCorrelationProjection& Projection() const noexcept {
            return _projection;
        }

        /// Indicates whether both era provenance and correlation are available.
        constexpr bool IsCorrelated() const noexcept {
            return _era.IsValid() && _projection.IsCorrelated();
        }

    };


    static_assert(
        sizeof(EraQualifiedClockCorrelationProjection) == 24U,
        "EraQualifiedClockCorrelationProjection must remain a compact 24-byte value"
    );

    static_assert(
        std::is_trivially_copyable_v<EraQualifiedClockCorrelationProjection>,
        "EraQualifiedClockCorrelationProjection must remain trivially copyable"
    );

} // ESPressio::Clock

