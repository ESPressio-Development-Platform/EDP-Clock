#pragma once

#include <cstdint>
#include <type_traits>

namespace ESPressio::Clock {

    /// Position on the shared synchronized clock coordinate system, expressed in nanoseconds.
    ///
    /// The synchronized epoch is deliberately opaque to EDP-Clock. It is established by the
    /// accepted synchronization reference and carries no calendar, civil-time, or time-zone
    /// meaning. Zero is a valid synchronized coordinate.
    class SynchronizedTimestamp final {
    private:

        // Timestamp value.

        /// Nanoseconds elapsed from the shared synchronized clock origin.
        std::uint64_t _nanoseconds{0U};

        // Construction.

        /// Creates a timestamp from an already-normalized nanosecond coordinate.
        explicit constexpr SynchronizedTimestamp(
            std::uint64_t nanoseconds
        ) noexcept :
            _nanoseconds(nanoseconds) {}

    public:

        // Construction.

        /// Creates the synchronized timestamp at the shared clock origin.
        constexpr SynchronizedTimestamp() noexcept = default;

        /// Creates a synchronized timestamp from a nanosecond coordinate.
        ///
        /// @param nanoseconds Unsigned nanoseconds from the shared synchronized origin.
        static constexpr SynchronizedTimestamp FromNanoseconds(
            std::uint64_t nanoseconds
        ) noexcept {
            return SynchronizedTimestamp(nanoseconds);
        }


        // Value access.

        /// Returns this timestamp's coordinate in nanoseconds from the synchronized origin.
        constexpr std::uint64_t Nanoseconds() const noexcept {
            return _nanoseconds;
        }


        // Comparison.

        /// Indicates whether two timestamps represent the same synchronized coordinate.
        friend constexpr bool operator ==(
            const SynchronizedTimestamp& left,
            const SynchronizedTimestamp& right
        ) noexcept {
            return left._nanoseconds == right._nanoseconds;
        }

        /// Indicates whether two timestamps represent different synchronized coordinates.
        friend constexpr bool operator !=(
            const SynchronizedTimestamp& left,
            const SynchronizedTimestamp& right
        ) noexcept {
            return !(left == right);
        }

        /// Indicates whether the left timestamp precedes the right timestamp.
        friend constexpr bool operator <(
            const SynchronizedTimestamp& left,
            const SynchronizedTimestamp& right
        ) noexcept {
            return left._nanoseconds < right._nanoseconds;
        }

        /// Indicates whether the left timestamp precedes or equals the right timestamp.
        friend constexpr bool operator <=(
            const SynchronizedTimestamp& left,
            const SynchronizedTimestamp& right
        ) noexcept {
            return !(right < left);
        }

        /// Indicates whether the left timestamp follows the right timestamp.
        friend constexpr bool operator >(
            const SynchronizedTimestamp& left,
            const SynchronizedTimestamp& right
        ) noexcept {
            return right < left;
        }

        /// Indicates whether the left timestamp follows or equals the right timestamp.
        friend constexpr bool operator >=(
            const SynchronizedTimestamp& left,
            const SynchronizedTimestamp& right
        ) noexcept {
            return !(left < right);
        }

    };


    static_assert(
        sizeof(SynchronizedTimestamp) == sizeof(std::uint64_t),
        "SynchronizedTimestamp must remain an exact 8-byte nanosecond coordinate"
    );

    static_assert(
        std::is_trivially_copyable_v<SynchronizedTimestamp>,
        "SynchronizedTimestamp must remain a trivially copyable value type"
    );

} // ESPressio::Clock
