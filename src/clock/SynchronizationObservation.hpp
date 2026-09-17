#pragma once

#include <type_traits>

#include "MonotonicTimestamp.hpp"
#include "SynchronizationUncertainty.hpp"
#include "SynchronizedTimestamp.hpp"

namespace ESPressio::Clock {

    /// Source-agnostic observation relating one local monotonic coordinate to synchronized time.
    ///
    /// Transport-specific mechanisms reduce their evidence to this type before Clock sees it.
    /// Clock therefore does not know whether an observation originated from Mesh, GNSS, an RTC,
    /// a network exchange, another processor, or any other synchronization mechanism.
    class SynchronizationObservation final {
    private:

        // Observation coordinates.

        /// Local monotonic coordinate at which the reference estimate applies.
        MonotonicTimestamp _localTimestamp;

        /// Best available synchronized reference coordinate at the local observation coordinate.
        SynchronizedTimestamp _referenceTimestamp;

        /// Conservative uncertainty associated with the reference estimate.
        SynchronizationUncertainty _uncertainty;

    public:

        // Construction.

        /// Creates one synchronization observation.
        ///
        /// @param localTimestamp Local monotonic coordinate associated with the observation.
        /// @param referenceTimestamp Best synchronized coordinate at the local coordinate.
        /// @param uncertainty Conservative uncertainty of the synchronized estimate.
        constexpr SynchronizationObservation(
            const MonotonicTimestamp& localTimestamp,
            const SynchronizedTimestamp& referenceTimestamp,
            const SynchronizationUncertainty& uncertainty
        ) noexcept :
            _localTimestamp(localTimestamp),
            _referenceTimestamp(referenceTimestamp),
            _uncertainty(uncertainty) {}


        // Value access.

        /// Returns the local monotonic coordinate at which this observation applies.
        constexpr const MonotonicTimestamp& LocalTimestamp() const noexcept {
            return _localTimestamp;
        }

        /// Returns the synchronized reference coordinate estimated by this observation.
        constexpr const SynchronizedTimestamp& ReferenceTimestamp() const noexcept {
            return _referenceTimestamp;
        }

        /// Returns the conservative uncertainty associated with this observation.
        constexpr const SynchronizationUncertainty& Uncertainty() const noexcept {
            return _uncertainty;
        }

    };


    static_assert(
        std::is_trivially_copyable_v<SynchronizationObservation>,
        "SynchronizationObservation must remain a trivially copyable value type"
    );

} // ESPressio::Clock
