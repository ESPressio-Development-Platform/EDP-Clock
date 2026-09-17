#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>

namespace ESPressio::Clock {

    /// Conservative non-negative uncertainty associated with synchronized time, in nanoseconds.
    ///
    /// The 32-bit representation intentionally saturates at approximately 4.295 seconds. Saturation
    /// means the synchronized mapping is no longer sufficiently bounded to remain meaningful for
    /// the Clock domain's quality reporting.
    class SynchronizationUncertainty final {
    private:

        // Uncertainty value.

        /// Conservative uncertainty bound expressed in nanoseconds.
        std::uint32_t _nanoseconds{std::numeric_limits<std::uint32_t>::max()};

        // Construction.

        /// Creates uncertainty from an already-saturated 32-bit nanosecond value.
        explicit constexpr SynchronizationUncertainty(
            std::uint32_t nanoseconds
        ) noexcept :
            _nanoseconds(nanoseconds) {}

    public:

        // Construction.

        /// Creates an unknown/saturated uncertainty value.
        constexpr SynchronizationUncertainty() noexcept = default;

        /// Creates uncertainty from a nanosecond value, saturating when it exceeds the 32-bit range.
        ///
        /// @param nanoseconds Conservative uncertainty bound in nanoseconds.
        static constexpr SynchronizationUncertainty FromNanoseconds(
            std::uint64_t nanoseconds
        ) noexcept {
            constexpr auto maximum = std::numeric_limits<std::uint32_t>::max();

            return SynchronizationUncertainty(
                nanoseconds > static_cast<std::uint64_t>(maximum)
                    ? maximum
                    : static_cast<std::uint32_t>(nanoseconds)
            );
        }

        /// Returns an exact zero-uncertainty value.
        static constexpr SynchronizationUncertainty Zero() noexcept {
            return SynchronizationUncertainty(0U);
        }

        /// Returns the saturated uncertainty value.
        static constexpr SynchronizationUncertainty Maximum() noexcept {
            return SynchronizationUncertainty(
                std::numeric_limits<std::uint32_t>::max()
            );
        }


        // Value access.

        /// Returns the conservative uncertainty bound in nanoseconds.
        constexpr std::uint32_t Nanoseconds() const noexcept {
            return _nanoseconds;
        }

        /// Indicates whether the uncertainty has saturated at its maximum representable value.
        constexpr bool IsSaturated() const noexcept {
            return _nanoseconds == std::numeric_limits<std::uint32_t>::max();
        }


        // Comparison.

        /// Indicates whether two uncertainty values are identical.
        friend constexpr bool operator ==(
            const SynchronizationUncertainty& left,
            const SynchronizationUncertainty& right
        ) noexcept {
            return left._nanoseconds == right._nanoseconds;
        }

        /// Indicates whether two uncertainty values differ.
        friend constexpr bool operator !=(
            const SynchronizationUncertainty& left,
            const SynchronizationUncertainty& right
        ) noexcept {
            return !(left == right);
        }

        /// Indicates whether the left uncertainty is smaller than the right uncertainty.
        friend constexpr bool operator <(
            const SynchronizationUncertainty& left,
            const SynchronizationUncertainty& right
        ) noexcept {
            return left._nanoseconds < right._nanoseconds;
        }

        /// Indicates whether the left uncertainty is smaller than or equal to the right uncertainty.
        friend constexpr bool operator <=(
            const SynchronizationUncertainty& left,
            const SynchronizationUncertainty& right
        ) noexcept {
            return !(right < left);
        }

        /// Indicates whether the left uncertainty is greater than the right uncertainty.
        friend constexpr bool operator >(
            const SynchronizationUncertainty& left,
            const SynchronizationUncertainty& right
        ) noexcept {
            return right < left;
        }

        /// Indicates whether the left uncertainty is greater than or equal to the right uncertainty.
        friend constexpr bool operator >=(
            const SynchronizationUncertainty& left,
            const SynchronizationUncertainty& right
        ) noexcept {
            return !(left < right);
        }

    };


    static_assert(
        sizeof(SynchronizationUncertainty) == sizeof(std::uint32_t),
        "SynchronizationUncertainty must remain an exact 4-byte nanosecond value"
    );

    static_assert(
        std::is_trivially_copyable_v<SynchronizationUncertainty>,
        "SynchronizationUncertainty must remain a trivially copyable value type"
    );

} // ESPressio::Clock
