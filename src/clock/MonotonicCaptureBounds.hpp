#pragma once

#include <cstdint>
#include <type_traits>

#include "MonotonicTimestamp.hpp"
#include "SynchronizationUncertainty.hpp"

namespace ESPressio::Clock {

    /// Canonical local-monotonic interval known to contain one physical occurrence.
    ///
    /// Connectivity providers convert native counters, wrap state and event-specific capture
    /// evidence into this source-agnostic value before crossing the Clock boundary. An interval
    /// whose earliest coordinate follows its latest coordinate is explicitly unavailable.
    class MonotonicCaptureBounds final {
    private:

        // Capture interval.

        /// Earliest canonical local-monotonic coordinate at which the occurrence may have happened.
        MonotonicTimestamp _earliest{MonotonicTimestamp::FromNanoseconds(1U)};

        /// Latest canonical local-monotonic coordinate at which the occurrence may have happened.
        MonotonicTimestamp _latest{MonotonicTimestamp::FromNanoseconds(0U)};

        // Construction.

        /// Creates one already-validated interval.
        constexpr MonotonicCaptureBounds(
            const MonotonicTimestamp& earliest,
            const MonotonicTimestamp& latest
        ) noexcept :
            _earliest(earliest),
            _latest(latest) {}

    public:

        // Construction.

        /// Creates unavailable capture evidence.
        constexpr MonotonicCaptureBounds() noexcept = default;

        /// Creates bounds when the supplied coordinates form a valid closed interval.
        ///
        /// Reversed coordinates produce unavailable evidence rather than being silently reordered.
        static constexpr MonotonicCaptureBounds Between(
            const MonotonicTimestamp& earliest,
            const MonotonicTimestamp& latest
        ) noexcept {
            if (latest < earliest) return MonotonicCaptureBounds();

            return MonotonicCaptureBounds(
                earliest,
                latest
            );
        }

        /// Creates exact capture evidence at one canonical local-monotonic coordinate.
        static constexpr MonotonicCaptureBounds Exact(
            const MonotonicTimestamp& timestamp
        ) noexcept {
            return MonotonicCaptureBounds(
                timestamp,
                timestamp
            );
        }


        // Interval access.

        /// Indicates whether this value contains usable capture evidence.
        constexpr bool IsAvailable() const noexcept {
            return _earliest <= _latest;
        }

        /// Returns the earliest coordinate in the closed capture interval.
        constexpr const MonotonicTimestamp& Earliest() const noexcept {
            return _earliest;
        }

        /// Returns the latest coordinate in the closed capture interval.
        constexpr const MonotonicTimestamp& Latest() const noexcept {
            return _latest;
        }

        /// Returns the overflow-safe midpoint estimate of the capture interval.
        ///
        /// Unavailable evidence returns the monotonic origin and must remain unavailable to its
        /// caller through IsAvailable().
        constexpr MonotonicTimestamp Midpoint() const noexcept {
            if (!IsAvailable()) return MonotonicTimestamp();

            const auto earliest = _earliest.Nanoseconds();
            const auto width = _latest.Nanoseconds() - earliest;

            return MonotonicTimestamp::FromNanoseconds(
                earliest + (width / 2U)
            );
        }

        /// Returns the conservative midpoint uncertainty implied by the closed interval.
        ///
        /// Odd interval widths round upward. Unavailable evidence produces saturated uncertainty.
        constexpr SynchronizationUncertainty Uncertainty() const noexcept {
            if (!IsAvailable()) return SynchronizationUncertainty::Maximum();

            const auto width = _latest.Nanoseconds() - _earliest.Nanoseconds();
            const auto halfWidth = (width / 2U) + ((width & 1U) == 0U ? 0U : 1U);

            return SynchronizationUncertainty::FromNanoseconds(
                halfWidth
            );
        }

    };


    static_assert(
        sizeof(MonotonicCaptureBounds) == 16U,
        "MonotonicCaptureBounds must remain an exact 16-byte canonical interval"
    );

    static_assert(
        std::is_trivially_copyable_v<MonotonicCaptureBounds>,
        "MonotonicCaptureBounds must remain a trivially copyable value type"
    );

} // ESPressio::Clock

