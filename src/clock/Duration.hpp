#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>

namespace ESPressio::Clock {

    /// Signed physical duration expressed in the Clock domain's canonical nanosecond unit.
    ///
    /// Duration is a pure value type. It owns no resources, performs no allocation, and has
    /// no relationship to calendars, dates, time zones, or other human interpretations of time.
    class Duration final {
    private:

        // Duration value.

        /// Signed number of nanoseconds represented by this duration.
        std::int64_t _nanoseconds{0};

        // Construction.

        /// Creates a duration from an already-normalized nanosecond value.
        explicit constexpr Duration(
            std::int64_t nanoseconds
        ) noexcept :
            _nanoseconds(nanoseconds) {}

    public:

        // Construction.

        /// Creates a zero-length duration.
        constexpr Duration() noexcept = default;

        /// Creates a duration from a signed nanosecond value.
        ///
        /// @param nanoseconds Signed duration expressed in nanoseconds.
        static constexpr Duration FromNanoseconds(
            std::int64_t nanoseconds
        ) noexcept {
            return Duration(nanoseconds);
        }

        /// Returns the smallest duration representable by this type.
        static constexpr Duration Minimum() noexcept {
            return Duration(std::numeric_limits<std::int64_t>::min());
        }

        /// Returns the largest duration representable by this type.
        static constexpr Duration Maximum() noexcept {
            return Duration(std::numeric_limits<std::int64_t>::max());
        }


        // Value access.

        /// Returns this duration expressed in signed nanoseconds.
        constexpr std::int64_t Nanoseconds() const noexcept {
            return _nanoseconds;
        }


        // Comparison.

        /// Indicates whether two durations represent the same physical interval.
        friend constexpr bool operator ==(
            const Duration& left,
            const Duration& right
        ) noexcept {
            return left._nanoseconds == right._nanoseconds;
        }

        /// Indicates whether two durations represent different physical intervals.
        friend constexpr bool operator !=(
            const Duration& left,
            const Duration& right
        ) noexcept {
            return !(left == right);
        }

        /// Indicates whether the left duration is shorter than the right duration.
        friend constexpr bool operator <(
            const Duration& left,
            const Duration& right
        ) noexcept {
            return left._nanoseconds < right._nanoseconds;
        }

        /// Indicates whether the left duration is shorter than or equal to the right duration.
        friend constexpr bool operator <=(
            const Duration& left,
            const Duration& right
        ) noexcept {
            return !(right < left);
        }

        /// Indicates whether the left duration is longer than the right duration.
        friend constexpr bool operator >(
            const Duration& left,
            const Duration& right
        ) noexcept {
            return right < left;
        }

        /// Indicates whether the left duration is longer than or equal to the right duration.
        friend constexpr bool operator >=(
            const Duration& left,
            const Duration& right
        ) noexcept {
            return !(left < right);
        }

    };


    static_assert(
        sizeof(Duration) == sizeof(std::int64_t),
        "Duration must remain an exact 8-byte nanosecond value"
    );

    static_assert(
        std::is_trivially_copyable_v<Duration>,
        "Duration must remain a trivially copyable value type"
    );

} // ESPressio::Clock
