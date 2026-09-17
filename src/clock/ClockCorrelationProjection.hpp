#pragma once

#include <cstdint>
#include <type_traits>

#include "ClockCorrelationProjectionStatus.hpp"
#include "SynchronizationUncertainty.hpp"
#include "SynchronizedTimestamp.hpp"

namespace ESPressio::Clock {

    /// Result of projecting one monotonic occurrence into synchronized time through a correlation.
    class ClockCorrelationProjection final {
    private:

        // Projection values.

        /// Best synchronized coordinate estimate for the projected monotonic occurrence.
        SynchronizedTimestamp _timestamp;

        /// Conservative uncertainty bound associated with the synchronized coordinate estimate.
        SynchronizationUncertainty _uncertainty;

        /// Status describing whether the projection produced a usable synchronized coordinate.
        ClockCorrelationProjectionStatus _status{ClockCorrelationProjectionStatus::CorrelationUnavailable};

        // Construction.

        /// Creates one projection result from already-validated values.
        constexpr ClockCorrelationProjection(
            const SynchronizedTimestamp& timestamp,
            const SynchronizationUncertainty& uncertainty,
            ClockCorrelationProjectionStatus status
        ) noexcept :
            _timestamp(timestamp),
            _uncertainty(uncertainty),
            _status(status) {}

        friend class ClockCorrelation;

    public:

        // Construction.

        /// Creates an unavailable projection result.
        constexpr ClockCorrelationProjection() noexcept = default;


        // Value access.

        /// Returns the synchronized coordinate estimate carried by this projection.
        constexpr const SynchronizedTimestamp& Timestamp() const noexcept {
            return _timestamp;
        }

        /// Returns the conservative uncertainty bound associated with this projection.
        constexpr const SynchronizationUncertainty& Uncertainty() const noexcept {
            return _uncertainty;
        }

        /// Returns the status describing whether this projection succeeded.
        constexpr ClockCorrelationProjectionStatus Status() const noexcept {
            return _status;
        }

        /// Indicates whether this result contains a successfully correlated synchronized coordinate.
        constexpr bool IsCorrelated() const noexcept {
            return _status == ClockCorrelationProjectionStatus::Correlated;
        }

    };


    static_assert(
        sizeof(ClockCorrelationProjection) == 16U,
        "ClockCorrelationProjection must remain a compact 16-byte timestamp/quality value"
    );

    static_assert(
        std::is_trivially_copyable_v<ClockCorrelationProjection>,
        "ClockCorrelationProjection must remain a trivially copyable value type"
    );

} // ESPressio::Clock
