#pragma once

#include <type_traits>

#include "ClockCorrelation.hpp"
#include "ClockEra.hpp"
#include "EraQualifiedClockCorrelationProjection.hpp"

namespace ESPressio::Clock {

    /// Immutable monotonic-to-synchronized correlation paired with its opaque temporal era.
    class EraQualifiedClockCorrelation final {
    private:

        // Temporal provenance and correlation.

        /// Era in which the correlation's synchronized coordinates are meaningful.
        ClockEra _era;

        /// Source-agnostic immutable reference mapping.
        ClockCorrelation _correlation;

    public:

        // Construction.

        /// Creates unavailable era-qualified correlation evidence.
        constexpr EraQualifiedClockCorrelation() noexcept = default;

        /// Creates one immutable correlation paired with its temporal era.
        constexpr EraQualifiedClockCorrelation(
            const ClockEra& era,
            const ClockCorrelation& correlation
        ) noexcept :
            _era(era),
            _correlation(correlation) {}


        // Value access.

        /// Returns the opaque temporal era of this correlation.
        constexpr const ClockEra& Era() const noexcept {
            return _era;
        }

        /// Returns the underlying immutable reference correlation.
        constexpr const ClockCorrelation& Correlation() const noexcept {
            return _correlation;
        }

        /// Indicates whether both era provenance and the correlation are available.
        constexpr bool IsAvailable() const noexcept {
            return _era.IsValid() && _correlation.IsAvailable();
        }


        // Timestamp correlation.

        /// Projects one local monotonic occurrence while retaining the correlation's temporal era.
        constexpr EraQualifiedClockCorrelationProjection Correlate(
            const MonotonicTimestamp& timestamp,
            const SynchronizationUncertainty& captureUncertainty
        ) const noexcept {
            if (!IsAvailable()) return EraQualifiedClockCorrelationProjection();

            return EraQualifiedClockCorrelationProjection(
                _era,
                _correlation.Correlate(
                    timestamp,
                    captureUncertainty
                )
            );
        }

    };


    static_assert(
        sizeof(EraQualifiedClockCorrelation) == 40U,
        "EraQualifiedClockCorrelation must remain a compact 40-byte immutable mapping"
    );

    static_assert(
        std::is_trivially_copyable_v<EraQualifiedClockCorrelation>,
        "EraQualifiedClockCorrelation must remain trivially copyable"
    );

} // ESPressio::Clock

