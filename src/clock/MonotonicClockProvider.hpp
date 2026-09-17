#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

#include "ClockComposition.hpp"
#include "MonotonicTimestamp.hpp"

namespace ESPressio::Clock {

    namespace Detail {

        /// Extracts and validates the exact frequency advertised by a MonotonicTimebase provider.
        template<class TTimebaseProvider>
        struct MonotonicTimebaseTraits {

            static_assert(
                TTimebaseProvider::CompositionCapabilities::template Contains<MonotonicTimebase>,
                "MonotonicClockProvider requires a provider that supplies MonotonicTimebase"
            );

            // Timebase properties.

            /// Compile-time property set advertised by the selected timebase provider.
            using Properties =
                typename TTimebaseProvider::CompositionCapabilities::template PropertiesFor<MonotonicTimebase>;

            static_assert(
                Properties::template Contains<TimebaseFrequencyNumerator>,
                "MonotonicTimebase provider must advertise TimebaseFrequencyNumerator"
            );

            static_assert(
                Properties::template Contains<TimebaseFrequencyDenominator>,
                "MonotonicTimebase provider must advertise TimebaseFrequencyDenominator"
            );

            /// Exact frequency numerator advertised by the selected timebase.
            static constexpr std::uint32_t FrequencyNumerator =
                Properties::template Value<TimebaseFrequencyNumerator>;

            /// Exact frequency denominator advertised by the selected timebase.
            static constexpr std::uint32_t FrequencyDenominator =
                Properties::template Value<TimebaseFrequencyDenominator>;

            static_assert(
                FrequencyNumerator > 0U,
                "MonotonicTimebase frequency numerator must be non-zero"
            );

            static_assert(
                FrequencyDenominator > 0U,
                "MonotonicTimebase frequency denominator must be non-zero"
            );

            // Timebase operation validation.

            /// Return type produced by the selected provider's CurrentCount operation.
            using CountResult = decltype(
                std::declval<const TTimebaseProvider&>().CurrentCount()
            );

            static_assert(
                std::is_same_v<CountResult, std::uint64_t>,
                "MonotonicTimebase CurrentCount() must return std::uint64_t"
            );

            static_assert(
                noexcept(
                    std::declval<const TTimebaseProvider&>().CurrentCount()
                ),
                "MonotonicTimebase CurrentCount() must be noexcept"
            );

            // Derived clock properties.

            /// Number of canonical nanoseconds represented by the timebase frequency denominator.
            static constexpr std::uint64_t NanosecondScale =
                1000000000ULL * static_cast<std::uint64_t>(FrequencyDenominator);

            /// Whole-nanosecond resolution conservatively advertised to clock consumers.
            static constexpr std::uint64_t ResolutionNanoseconds =
                (NanosecondScale / static_cast<std::uint64_t>(FrequencyNumerator)) +
                (
                    (NanosecondScale % static_cast<std::uint64_t>(FrequencyNumerator)) == 0U
                        ? 0U
                        : 1U
                );

        };


        /// Converts an absolute timebase count to canonical nanoseconds without cumulative rounding.
        template<class TTimebaseProvider>
        constexpr std::uint64_t ConvertCountToNanoseconds(
            std::uint64_t count
        ) noexcept {
            using Traits = MonotonicTimebaseTraits<TTimebaseProvider>;

            constexpr auto frequencyNumerator =
                static_cast<std::uint64_t>(Traits::FrequencyNumerator);
            constexpr auto nanosecondScale = Traits::NanosecondScale;
            constexpr auto wholeNanosecondsPerCount =
                nanosecondScale / frequencyNumerator;
            constexpr auto fractionalNumerator =
                nanosecondScale % frequencyNumerator;
            constexpr auto maximum =
                std::numeric_limits<std::uint64_t>::max();

            std::uint64_t result = 0U;

            if constexpr (wholeNanosecondsPerCount > 0U) {
                if (count > (maximum / wholeNanosecondsPerCount)) return maximum;

                result = count * wholeNanosecondsPerCount;
            }

            if constexpr (fractionalNumerator == 0U) return result;

            const auto countQuotient = count / frequencyNumerator;
            const auto countRemainder = count % frequencyNumerator;

            if (countQuotient > ((maximum - result) / fractionalNumerator)) return maximum;

            result += countQuotient * fractionalNumerator;

            // Both factors are strictly smaller than the 32-bit frequency numerator, so this
            // product is representable in std::uint64_t without requiring 128-bit arithmetic.
            const auto fractionalTail =
                (countRemainder * fractionalNumerator) / frequencyNumerator;

            if (fractionalTail > (maximum - result)) return maximum;

            return result + fractionalTail;
        }

    } // ESPressio::Clock::Detail


    /// Generic MonotonicClock provider backed by one application-selected MonotonicTimebase provider.
    ///
    /// The provider retains only a borrowed pointer to the lifetime-stable timebase. It owns no
    /// timer hardware, thread primitive, heap allocation, worker, callback registry, or mutable
    /// timing state. Thread safety of physical count acquisition is the timebase provider's contract.
    ///
    /// @tparam TTimebaseProvider Concrete provider selected by application Bootstrap.
    template<class TTimebaseProvider>
    class MonotonicClockProvider final : public Framework::Provider<
        Domain,
        Framework::Provides<
            Framework::Offer<
                MonotonicClock,
                Framework::PropertyValue<
                    ClockResolutionNanoseconds,
                    Detail::MonotonicTimebaseTraits<TTimebaseProvider>::ResolutionNanoseconds
                >
            >
        >,
        Framework::Requires<
            Framework::Need<MonotonicTimebase>
        >
    > {
    private:

        // Selected timebase.

        /// Borrowed lifetime-stable timebase selected by application Bootstrap.
        const TTimebaseProvider* _timebase;

    public:

        // Construction and lifetime.

        /// Creates the generic monotonic clock over the selected timebase provider.
        ///
        /// @param timebase Lifetime-stable concrete timebase provider.
        explicit constexpr MonotonicClockProvider(
            const TTimebaseProvider& timebase
        ) noexcept :
            _timebase(&timebase) {}

        /// Prevents duplicating a lifetime clock provider accidentally.
        MonotonicClockProvider(const MonotonicClockProvider&) = delete;

        /// Prevents replacing a lifetime clock provider by copy assignment.
        MonotonicClockProvider& operator =(const MonotonicClockProvider&) = delete;

        /// Prevents relocation after other facilities have borrowed this clock.
        MonotonicClockProvider(MonotonicClockProvider&&) = delete;

        /// Prevents relocation by move assignment.
        MonotonicClockProvider& operator =(MonotonicClockProvider&&) = delete;


        // Clock metadata.

        /// Exact numerator of the selected timebase count frequency.
        static constexpr std::uint32_t FrequencyNumerator =
            Detail::MonotonicTimebaseTraits<TTimebaseProvider>::FrequencyNumerator;

        /// Exact denominator of the selected timebase count frequency.
        static constexpr std::uint32_t FrequencyDenominator =
            Detail::MonotonicTimebaseTraits<TTimebaseProvider>::FrequencyDenominator;

        /// Conservative whole-nanosecond resolution advertised by this monotonic clock.
        static constexpr std::uint64_t ResolutionNanoseconds =
            Detail::MonotonicTimebaseTraits<TTimebaseProvider>::ResolutionNanoseconds;


        // Clock access.

        /// Returns the current monotonic timestamp without mutating Clock-owned state.
        ///
        /// Each call reads the selected timebase exactly once and converts that absolute count to
        /// nanoseconds. Conversion is derived from the exact rational source frequency, so rounded
        /// per-count error is never accumulated across successive reads.
        MonotonicTimestamp Now() const noexcept {
            const auto count = _timebase->CurrentCount();

            return MonotonicTimestamp::FromNanoseconds(
                Detail::ConvertCountToNanoseconds<TTimebaseProvider>(count)
            );
        }

    };

} // ESPressio::Clock
