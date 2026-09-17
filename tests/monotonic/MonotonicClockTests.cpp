#include <atomic>
#include <cstdint>
#include <limits>
#include <thread>
#include <type_traits>
#include <vector>

#include "ESPressio_Clock.hpp"

namespace ESPressio::Clock::Tests::Monotonic {

    /// Reads MonotonicNow() from another linked code unit.
    MonotonicTimestamp ReadFromAnotherTranslationUnit() noexcept;


    /// Thread-safe deterministic timebase used to validate the generic Clock foundation.
    class TestTimebase final : public MonotonicTimebaseProvider<32768U> {
    private:

        // Simulated physical count.

        /// Coherent count exposed to concurrent readers.
        std::atomic<std::uint64_t> _count{0U};

    public:

        // Timebase access.

        /// Returns the current simulated physical count coherently across host threads.
        std::uint64_t CurrentCount() const noexcept {
            return _count.load(std::memory_order_relaxed);
        }


        // Test control.

        /// Sets the simulated physical count before or during concurrent read validation.
        ///
        /// @param count New simulated timebase count.
        void SetCount(
            std::uint64_t count
        ) noexcept {
            _count.store(
                count,
                std::memory_order_relaxed
            );
        }

        /// Advances the simulated physical count by one.
        void Advance() noexcept {
            _count.fetch_add(
                1U,
                std::memory_order_relaxed
            );
        }

    };


    /// Generic monotonic clock selected for the deterministic test timebase.
    using TestClock = MonotonicClockProvider<TestTimebase>;


    /// Deterministic source with a rational 3/2 counts-per-second frequency.
    class RationalTimebase final : public MonotonicTimebaseProvider<3U, 2U> {
    private:

        // Simulated physical count.

        /// Stable count used to validate exact rational-frequency conversion.
        std::uint64_t _count{0U};

    public:

        // Timebase access.

        /// Returns the current deterministic count.
        std::uint64_t CurrentCount() const noexcept {
            return _count;
        }


        // Test control.

        /// Sets the deterministic count used by the next read.
        ///
        /// @param count New source count.
        void SetCount(
            std::uint64_t count
        ) noexcept {
            _count = count;
        }

    };


    /// Generic monotonic clock selected for the rational-frequency test source.
    using RationalClock = MonotonicClockProvider<RationalTimebase>;


    /// Valid composition proving that the generic clock's MonotonicTimebase requirement is satisfied.
    using TestComposition = Framework::Composition<
        Domain,
        TestTimebase,
        TestClock
    >;


    static_assert(
        sizeof(MonotonicTimestamp) == 8U,
        "MonotonicTimestamp must occupy exactly eight bytes"
    );

    static_assert(
        sizeof(Duration) == 8U,
        "Duration must occupy exactly eight bytes"
    );

    static_assert(
        std::is_trivially_copyable_v<MonotonicTimestamp>,
        "MonotonicTimestamp must remain trivially copyable"
    );

    static_assert(
        std::is_trivially_copyable_v<Duration>,
        "Duration must remain trivially copyable"
    );

    static_assert(
        sizeof(TestClock) == sizeof(const TestTimebase*),
        "Generic MonotonicClockProvider must retain only one borrowed timebase pointer"
    );

    static_assert(
        !std::is_copy_constructible_v<TestClock> &&
        !std::is_copy_assignable_v<TestClock> &&
        !std::is_move_constructible_v<TestClock> &&
        !std::is_move_assignable_v<TestClock>,
        "Generic MonotonicClockProvider must remain lifetime-stable after construction"
    );

    static_assert(
        std::is_trivially_destructible_v<TestClock>,
        "Generic MonotonicClockProvider must not require destruction machinery"
    );

    static_assert(
        TestComposition::IsValid,
        "Expected the test Clock composition to be valid"
    );

    static_assert(
        std::is_same_v<
            TestComposition::ProviderFor<MonotonicTimebase>,
            TestTimebase
        >,
        "Expected TestTimebase to resolve as the MonotonicTimebase provider"
    );

    static_assert(
        std::is_same_v<
            TestComposition::ProviderFor<MonotonicClock>,
            TestClock
        >,
        "Expected TestClock to resolve as the MonotonicClock provider"
    );

    static_assert(
        TestClock::FrequencyNumerator == 32768U,
        "Expected the exact 32.768 kHz frequency numerator"
    );

    static_assert(
        TestClock::FrequencyDenominator == 1U,
        "Expected an integral 32.768 kHz frequency"
    );

    static_assert(
        TestClock::ResolutionNanoseconds == 30518U,
        "Expected conservative whole-nanosecond resolution for 32.768 kHz"
    );

    static_assert(
        TestComposition::PropertyValue<
            MonotonicClock,
            ClockResolutionNanoseconds
        > == 30518U,
        "Expected Clock resolution to be advertised through Composition"
    );

    static_assert(
        RationalClock::FrequencyNumerator == 3U &&
        RationalClock::FrequencyDenominator == 2U,
        "Expected the exact rational 3/2 counts-per-second frequency"
    );

    static_assert(
        RationalClock::ResolutionNanoseconds == 666666667ULL,
        "Expected conservative nanosecond resolution for a 3/2 Hz timebase"
    );


    /// Validates value semantics, exact frequency conversion, global binding, and concurrent reads.
    int RunTests() noexcept {
        const auto origin = MonotonicTimestamp::FromNanoseconds(0U);
        const auto later = MonotonicTimestamp::FromNanoseconds(2500U);

        if (origin.Nanoseconds() != 0U) return 1;
        if (!(origin < later)) return 2;
        if (Delta(
            origin,
            later
        ).Nanoseconds() != 2500) return 3;

        if (Delta(
            later,
            origin
        ).Nanoseconds() != -2500) return 4;

        TestTimebase timebase;
        TestClock clock(timebase);

        if (clock.Now().Nanoseconds() != 0U) return 5;

        timebase.SetCount(1U);
        if (clock.Now().Nanoseconds() != 30517U) return 6;

        timebase.SetCount(32768U);
        if (clock.Now().Nanoseconds() != 1000000000ULL) return 7;

        timebase.SetCount(65536U);
        if (clock.Now().Nanoseconds() != 2000000000ULL) return 8;

        RationalTimebase rationalTimebase;
        RationalClock rationalClock(rationalTimebase);

        rationalTimebase.SetCount(1U);
        if (rationalClock.Now().Nanoseconds() != 666666666ULL) return 9;

        rationalTimebase.SetCount(3U);
        if (rationalClock.Now().Nanoseconds() != 2000000000ULL) return 10;

        timebase.SetCount(std::numeric_limits<std::uint64_t>::max());
        if (clock.Now().Nanoseconds() != std::numeric_limits<std::uint64_t>::max()) return 11;

        timebase.SetCount(65536U);

        if (IsMonotonicClockBound()) return 12;
        if (!BindMonotonicClock(clock)) return 13;
        if (!IsMonotonicClockBound()) return 14;
        if (BindMonotonicClock(clock)) return 15;
        if (MonotonicNow().Nanoseconds() != 2000000000ULL) return 16;
        if (ReadFromAnotherTranslationUnit().Nanoseconds() != 2000000000ULL) return 17;

        timebase.SetCount(0U);

        std::atomic<bool> readsAreMonotonic{true};

        std::thread writer(
            [&timebase]() noexcept {
                for (std::uint32_t index = 0U; index < 100000U; ++index) {
                    timebase.Advance();
                }
            }
        );

        std::vector<std::thread> readers;

        for (std::uint32_t readerIndex = 0U; readerIndex < 4U; ++readerIndex) {
            readers.emplace_back(
                [&readsAreMonotonic]() noexcept {
                    auto previous = MonotonicNow();

                    for (std::uint32_t index = 0U; index < 50000U; ++index) {
                        const auto current = MonotonicNow();

                        if (current < previous) {
                            readsAreMonotonic.store(
                                false,
                                std::memory_order_relaxed
                            );

                            return;
                        }

                        previous = current;
                    }
                }
            );
        }

        writer.join();

        for (auto& reader : readers) {
            reader.join();
        }

        if (!readsAreMonotonic.load(std::memory_order_relaxed)) return 18;

        return 0;
    }

} // ESPressio::Clock::Tests::Monotonic


/// Executes the host-side Monotonic Clock foundation tests.
int main() {
    return ESPressio::Clock::Tests::Monotonic::RunTests();
}
