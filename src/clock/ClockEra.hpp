#pragma once

#include <cstdint>
#include <type_traits>

namespace ESPressio::Clock {

    /// Opaque identifier for one synchronized-time coordinate era.
    ///
    /// Clock does not create or interpret this value. An upstream coordinator owns the canonical
    /// mapping from a non-zero era value to its complete reference provenance and must reject a
    /// conflicting mapping. Zero is reserved for unavailable/unspecified temporal provenance.
    class ClockEra final {
    private:

        // Opaque era value.

        /// Coordinator-assigned non-zero era representation.
        std::uint64_t _value{0U};

        // Construction.

        /// Creates an era from one opaque representation.
        explicit constexpr ClockEra(
            std::uint64_t value
        ) noexcept :
            _value(value) {}

    public:

        // Construction.

        /// Creates the invalid/unspecified era.
        constexpr ClockEra() noexcept = default;

        /// Creates an era from an upstream-owned opaque representation.
        ///
        /// Passing zero produces the invalid era. Clock deliberately performs no derivation or
        /// provenance interpretation; those responsibilities remain with the coordinator.
        static constexpr ClockEra FromValue(
            std::uint64_t value
        ) noexcept {
            return ClockEra(value);
        }


        // Value access.

        /// Returns the opaque representation for canonical carriage or comparison.
        constexpr std::uint64_t Value() const noexcept {
            return _value;
        }

        /// Indicates whether this value names an available temporal era.
        constexpr bool IsValid() const noexcept {
            return _value != 0U;
        }


        // Comparison.

        /// Indicates whether two values name the same temporal era.
        friend constexpr bool operator ==(
            const ClockEra& left,
            const ClockEra& right
        ) noexcept {
            return left._value == right._value;
        }

        /// Indicates whether two values name different temporal eras.
        friend constexpr bool operator !=(
            const ClockEra& left,
            const ClockEra& right
        ) noexcept {
            return !(left == right);
        }

        /// Provides deterministic ordering of opaque era values.
        friend constexpr bool operator <(
            const ClockEra& left,
            const ClockEra& right
        ) noexcept {
            return left._value < right._value;
        }

        /// Indicates whether the left era precedes or equals the right opaque value.
        friend constexpr bool operator <=(
            const ClockEra& left,
            const ClockEra& right
        ) noexcept {
            return !(right < left);
        }

        /// Indicates whether the left era follows the right opaque value.
        friend constexpr bool operator >(
            const ClockEra& left,
            const ClockEra& right
        ) noexcept {
            return right < left;
        }

        /// Indicates whether the left era follows or equals the right opaque value.
        friend constexpr bool operator >=(
            const ClockEra& left,
            const ClockEra& right
        ) noexcept {
            return !(left < right);
        }

    };


    static_assert(
        sizeof(ClockEra) == sizeof(std::uint64_t),
        "ClockEra must remain an exact 8-byte opaque value"
    );

    static_assert(
        std::is_trivially_copyable_v<ClockEra>,
        "ClockEra must remain a trivially copyable value type"
    );

} // ESPressio::Clock

