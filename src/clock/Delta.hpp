#pragma once

#include <cstdint>
#include <limits>

#include "Duration.hpp"
#include "MonotonicTimestamp.hpp"

namespace ESPressio::Clock {

    /// Calculates the signed physical duration from one monotonic timestamp to another.
    ///
    /// The normal Clock contract expects the requested difference to fit within Duration's
    /// signed 64-bit nanosecond range. If a caller nevertheless asks for a difference beyond
    /// that representable range, the result is clamped to the nearest Duration boundary rather
    /// than allowing integer overflow.
    ///
    /// @param from Starting monotonic coordinate.
    /// @param to Ending monotonic coordinate.
    /// @return Signed duration equal to `to - from`.
    constexpr Duration Delta(
        const MonotonicTimestamp& from,
        const MonotonicTimestamp& to
    ) noexcept {
        const auto fromNanoseconds = from.Nanoseconds();
        const auto toNanoseconds = to.Nanoseconds();
        constexpr auto maximumPositiveMagnitude =
            static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());
        constexpr auto maximumNegativeMagnitude = maximumPositiveMagnitude + 1U;

        if (toNanoseconds >= fromNanoseconds) {
            const auto magnitude = toNanoseconds - fromNanoseconds;

            if (magnitude > maximumPositiveMagnitude) return Duration::Maximum();

            return Duration::FromNanoseconds(
                static_cast<std::int64_t>(magnitude)
            );
        }

        const auto magnitude = fromNanoseconds - toNanoseconds;

        if (magnitude >= maximumNegativeMagnitude) return Duration::Minimum();

        return Duration::FromNanoseconds(
            -static_cast<std::int64_t>(magnitude)
        );
    }

} // ESPressio::Clock
