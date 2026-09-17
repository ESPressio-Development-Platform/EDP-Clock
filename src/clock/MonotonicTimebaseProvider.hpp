#pragma once

#include <cstdint>

#include "ClockComposition.hpp"

namespace ESPressio::Clock {

    /// Base Composition declaration for a concrete MonotonicTimebase provider.
    ///
    /// The exact nominal frequency is expressed as:
    ///
    /// `counts per second = TFrequencyNumerator / TFrequencyDenominator`
    ///
    /// A concrete provider inheriting this declaration must expose:
    ///
    /// `std::uint64_t CurrentCount() const noexcept`
    ///
    /// The returned count must be safe for concurrent reads, coherent, non-regressing, and
    /// extended across any underlying hardware rollover for the supported application lifetime.
    ///
    /// @tparam TFrequencyNumerator Exact count-frequency numerator.
    /// @tparam TFrequencyDenominator Exact count-frequency denominator.
    template<
        std::uint32_t TFrequencyNumerator,
        std::uint32_t TFrequencyDenominator = 1U
    >
    struct MonotonicTimebaseProvider : Framework::Provider<
        Domain,
        Framework::Provides<
            Framework::Offer<
                MonotonicTimebase,
                Framework::PropertyValue<
                    TimebaseFrequencyNumerator,
                    TFrequencyNumerator
                >,
                Framework::PropertyValue<
                    TimebaseFrequencyDenominator,
                    TFrequencyDenominator
                >
            >
        >
    > {

        static_assert(
            TFrequencyNumerator > 0U,
            "MonotonicTimebase frequency numerator must be non-zero"
        );

        static_assert(
            TFrequencyDenominator > 0U,
            "MonotonicTimebase frequency denominator must be non-zero"
        );

        // Frequency metadata.

        /// Exact numerator of this timebase's nominal count frequency.
        static constexpr std::uint32_t FrequencyNumerator = TFrequencyNumerator;

        /// Exact denominator of this timebase's nominal count frequency.
        static constexpr std::uint32_t FrequencyDenominator = TFrequencyDenominator;

    };

} // ESPressio::Clock
