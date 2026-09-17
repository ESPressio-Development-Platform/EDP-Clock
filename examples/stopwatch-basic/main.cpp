#include <cstdint>

#include "ESPressio_Clock.hpp"

namespace Example {

    namespace Clock = ESPressio::Clock;

    /// Minimal deterministic one-gigahertz timebase used to demonstrate Stopwatch semantics.
    class ApplicationTimebase final : public Clock::MonotonicTimebaseProvider<1000000000U> {
    private:

        // Demonstration count.

        /// Simulated nanosecond-resolution physical count.
        std::uint64_t _count{0U};

    public:

        // Timebase access.

        /// Returns the current simulated physical count.
        std::uint64_t CurrentCount() const noexcept {
            return _count;
        }


        // Demonstration control.

        /// Advances the simulated physical clock by the specified nanoseconds.
        ///
        /// @param nanoseconds Number of nanoseconds to advance.
        void Advance(
            std::uint64_t nanoseconds
        ) noexcept {
            _count += nanoseconds;
        }

    };


    /// Standard EDP-Clock monotonic provider used by the Stopwatch example.
    using ApplicationClock = Clock::MonotonicClockProvider<ApplicationTimebase>;

    /// Stopwatch bound directly to the application-selected monotonic provider.
    using ApplicationStopwatch = Clock::Stopwatch<ApplicationClock>;


    /// Demonstrates start, stop, resume, reset, and restart semantics.
    int Run() noexcept {
        ApplicationTimebase timebase;
        ApplicationClock monotonicClock(timebase);
        ApplicationStopwatch stopwatch(monotonicClock);

        stopwatch.Start();
        timebase.Advance(2500000U);

        if (stopwatch.Elapsed().Nanoseconds() != 2500000) return 1;

        stopwatch.Stop();
        timebase.Advance(1000000U);

        // A stopped Stopwatch does not accumulate elapsed time.
        if (stopwatch.Elapsed().Nanoseconds() != 2500000) return 2;

        stopwatch.Start();
        timebase.Advance(500000U);
        stopwatch.Stop();

        // Start() resumes the previously accumulated measurement.
        if (stopwatch.Elapsed().Nanoseconds() != 3000000) return 3;

        stopwatch.Reset();

        if (stopwatch.IsRunning()) return 4;
        if (stopwatch.Elapsed().Nanoseconds() != 0) return 5;

        stopwatch.Restart();
        timebase.Advance(125000U);

        if (stopwatch.Elapsed().Nanoseconds() != 125000) return 6;

        return 0;
    }

} // Example


/// Executes the basic Stopwatch example.
int main() {
    return Example::Run();
}
