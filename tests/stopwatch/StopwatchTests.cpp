#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

#include "ESPressio_Clock.hpp"

namespace ESPressio::Clock::Tests::Stopwatch {

    /// Deterministic nanosecond-resolution timebase that records every physical Clock read.
    class TestTimebase final : public MonotonicTimebaseProvider<1000000000U> {
    private:

        // Deterministic timebase state.

        /// Current nanosecond-resolution physical count.
        std::uint64_t _count{0U};

        /// Number of CurrentCount() reads observed since construction.
        mutable std::uint32_t _readCount{0U};

    public:

        // Timebase access.

        /// Returns the configured physical count and records one Clock read.
        std::uint64_t CurrentCount() const noexcept {
            ++_readCount;
            return _count;
        }


        // Test control.

        /// Sets the physical count returned by subsequent Clock reads.
        ///
        /// @param count New physical timebase count.
        void SetCount(
            std::uint64_t count
        ) noexcept {
            _count = count;
        }

        /// Returns the number of physical Clock reads observed so far.
        std::uint32_t ReadCount() const noexcept {
            return _readCount;
        }

    };


    /// Generic monotonic clock selected for Stopwatch validation.
    using TestClock = MonotonicClockProvider<TestTimebase>;

    /// Stopwatch bound to the deterministic test Clock.
    using TestStopwatch = ESPressio::Clock::Stopwatch<TestClock>;


    static_assert(
        !std::is_copy_constructible_v<TestStopwatch> &&
        !std::is_copy_assignable_v<TestStopwatch>,
        "Stopwatch must not duplicate mutable measurement sessions by copy"
    );

    static_assert(
        std::is_move_constructible_v<TestStopwatch> &&
        std::is_move_assignable_v<TestStopwatch>,
        "Stopwatch measurement sessions must support explicit transfer"
    );

    static_assert(
        std::is_trivially_destructible_v<TestStopwatch>,
        "Stopwatch must require no destruction machinery"
    );


    /// Validates Stopwatch lifecycle, read discipline, saturation, resume, and move semantics.
    int RunTests() noexcept {
        TestTimebase timebase;
        TestClock clock(timebase);
        TestStopwatch stopwatch(clock);

        if (stopwatch.IsRunning()) return 1;
        if (stopwatch.Elapsed().Nanoseconds() != 0) return 2;
        if (timebase.ReadCount() != 0U) return 3;

        timebase.SetCount(100U);
        stopwatch.Start();

        if (!stopwatch.IsRunning()) return 4;
        if (timebase.ReadCount() != 1U) return 5;

        stopwatch.Start();
        if (timebase.ReadCount() != 1U) return 6;

        timebase.SetCount(250U);
        if (stopwatch.Elapsed().Nanoseconds() != 150) return 7;
        if (timebase.ReadCount() != 2U) return 8;

        timebase.SetCount(300U);
        stopwatch.Stop();

        if (stopwatch.IsRunning()) return 9;
        if (stopwatch.Elapsed().Nanoseconds() != 200) return 10;
        if (timebase.ReadCount() != 3U) return 11;

        stopwatch.Stop();
        if (timebase.ReadCount() != 3U) return 12;

        timebase.SetCount(1000U);
        stopwatch.Start();
        if (timebase.ReadCount() != 4U) return 13;

        timebase.SetCount(1050U);
        stopwatch.Stop();
        if (stopwatch.Elapsed().Nanoseconds() != 250) return 14;

        stopwatch.Reset();
        if (stopwatch.IsRunning()) return 15;
        if (stopwatch.Elapsed().Nanoseconds() != 0) return 16;
        if (timebase.ReadCount() != 5U) return 17;

        timebase.SetCount(2000U);
        stopwatch.Restart();
        if (!stopwatch.IsRunning()) return 18;
        if (timebase.ReadCount() != 6U) return 19;

        timebase.SetCount(2125U);
        if (stopwatch.Elapsed().Nanoseconds() != 125) return 20;

        TestStopwatch moved(std::move(stopwatch));

        if (!moved.IsRunning()) return 21;
        if (stopwatch.IsRunning()) return 22;
        if (stopwatch.Elapsed().Nanoseconds() != 0) return 23;

        timebase.SetCount(2200U);
        if (moved.Elapsed().Nanoseconds() != 200) return 24;

        TestStopwatch assigned(clock);
        assigned = std::move(moved);

        if (!assigned.IsRunning()) return 25;
        if (moved.IsRunning()) return 26;
        if (moved.Elapsed().Nanoseconds() != 0) return 27;

        TestTimebase saturationTimebase;
        TestClock saturationClock(saturationTimebase);
        TestStopwatch saturationStopwatch(saturationClock);

        saturationTimebase.SetCount(0U);
        saturationStopwatch.Start();
        saturationTimebase.SetCount(std::numeric_limits<std::uint64_t>::max());
        saturationStopwatch.Stop();

        if (saturationStopwatch.Elapsed().Nanoseconds() != std::numeric_limits<std::int64_t>::max()) return 28;

        saturationTimebase.SetCount(std::numeric_limits<std::uint64_t>::max());
        saturationStopwatch.Start();
        saturationStopwatch.Stop();

        if (saturationStopwatch.Elapsed().Nanoseconds() != std::numeric_limits<std::int64_t>::max()) return 29;

        return 0;
    }

} // ESPressio::Clock::Tests::Stopwatch


/// Executes the host-side Stopwatch tests.
int main() {
    return ESPressio::Clock::Tests::Stopwatch::RunTests();
}
