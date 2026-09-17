#pragma once

#include <type_traits>

#include "SynchronizationState.hpp"
#include "SynchronizationUncertainty.hpp"
#include "SynchronizedTimestamp.hpp"

namespace ESPressio::Clock {

    /// One synchronized clock observation with the quality information valid for that observation.
    class SynchronizedReading final {
    private:

        // Reading values.

        /// Synchronized coordinate associated with this reading.
        SynchronizedTimestamp _timestamp;

        /// Conservative uncertainty bound associated with this reading.
        SynchronizationUncertainty _uncertainty;

        /// Synchronization lifecycle/quality state associated with this reading.
        SynchronizationState _state;

    public:

        // Construction.

        /// Creates a synchronized reading from one coherent timestamp, uncertainty and state.
        ///
        /// @param timestamp Synchronized coordinate represented by the reading.
        /// @param uncertainty Conservative uncertainty bound associated with the coordinate.
        /// @param state Synchronization lifecycle/quality state for the coordinate.
        constexpr SynchronizedReading(
            const SynchronizedTimestamp& timestamp,
            const SynchronizationUncertainty& uncertainty,
            SynchronizationState state
        ) noexcept :
            _timestamp(timestamp),
            _uncertainty(uncertainty),
            _state(state) {}


        // Value access.

        /// Returns the synchronized coordinate represented by this reading.
        constexpr const SynchronizedTimestamp& Timestamp() const noexcept {
            return _timestamp;
        }

        /// Returns the conservative uncertainty bound associated with this reading.
        constexpr const SynchronizationUncertainty& Uncertainty() const noexcept {
            return _uncertainty;
        }

        /// Returns the synchronization lifecycle/quality state associated with this reading.
        constexpr SynchronizationState State() const noexcept {
            return _state;
        }

    };


    static_assert(
        sizeof(SynchronizedReading) == 16U,
        "SynchronizedReading must remain a compact 16-byte timestamp/quality value"
    );

    static_assert(
        std::is_trivially_copyable_v<SynchronizedReading>,
        "SynchronizedReading must remain a trivially copyable value type"
    );

} // ESPressio::Clock
