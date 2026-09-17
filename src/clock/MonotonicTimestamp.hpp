#pragma once

#include <cstdint>
#include <type_traits>

namespace ESPressio::Clock {

    /// Position on the local monotonic clock coordinate system, expressed in nanoseconds.
    ///
    /// Zero is the opaque origin of the selected monotonic timebase. It is a valid timestamp
    /// and carries no calendar, civil-time, or other human-interpreted meaning.
    class MonotonicTimestamp final {
    private:

        // Timestamp value.

        /// Nanoseconds elapsed from the opaque monotonic clock origin.
        std::uint64_t _nanoseconds{0U};

        // Construction.

        /// Creates a timestamp from an already-normalized nanosecond coordinate.
        explicit constexpr MonotonicTimestamp(
            std::uint64_t nanoseconds
        ) noexcept :
            _nanoseconds(nanoseconds) {}

    public:

        // Construction.

        /// Creates the monotonic timestamp at the selected timebase's opaque origin.
        constexpr MonotonicTimestamp() noexcept = default;

        /// Creates a monotonic timestamp from a nanosecond coordinate.
        ///
        /// @param nanoseconds Unsigned nanoseconds from the opaque monotonic origin.
        static constexpr MonotonicTimestamp FromNanoseconds(
            std::uint64_t nanoseconds
        ) noexcept {
            return MonotonicTimestamp(nanoseconds);
        }


        // Value access.

        /// Returns this timestamp's coordinate in nanoseconds from the opaque monotonic origin.
        constexpr std::uint64_t Nanoseconds() const noexcept {
            return _nanoseconds;
        }


        // Comparison.

        /// Indicates whether two timestamps represent the same monotonic coordinate.
        friend constexpr bool operator ==(
            const MonotonicTimestamp& left,
            const MonotonicTimestamp& right
        ) noexcept {
            return left._nanoseconds == right._nanoseconds;
        }

        /// Indicates whether two timestamps represent different monotonic coordinates.
        friend constexpr bool operator !=(
            const MonotonicTimestamp& left,
            const MonotonicTimestamp& right
        ) noexcept {
            return !(left == right);
        }

        /// Indicates whether the left timestamp precedes the right timestamp.
        friend constexpr bool operator <(
            const MonotonicTimestamp& left,
            const MonotonicTimestamp& right
        ) noexcept {
            return left._nanoseconds < right._nanoseconds;
        }

        /// Indicates whether the left timestamp precedes or equals the right timestamp.
        friend constexpr bool operator <=(
            const MonotonicTimestamp& left,
            const MonotonicTimestamp& right
        ) noexcept {
            return !(right < left);
        }

        /// Indicates whether the left timestamp follows the right timestamp.
        friend constexpr bool operator >(
            const MonotonicTimestamp& left,
            const MonotonicTimestamp& right
        ) noexcept {
            return right < left;
        }

        /// Indicates whether the left timestamp follows or equals the right timestamp.
        friend constexpr bool operator >=(
            const MonotonicTimestamp& left,
            const MonotonicTimestamp& right
        ) noexcept {
            return !(left < right);
        }

    };


    static_assert(
        sizeof(MonotonicTimestamp) == sizeof(std::uint64_t),
        "MonotonicTimestamp must remain an exact 8-byte nanosecond coordinate"
    );

    static_assert(
        std::is_trivially_copyable_v<MonotonicTimestamp>,
        "MonotonicTimestamp must remain a trivially copyable value type"
    );

} // ESPressio::Clock
