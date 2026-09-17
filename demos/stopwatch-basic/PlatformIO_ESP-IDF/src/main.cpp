#include <cstdint>

#include <ESPressio_Clock.hpp>

namespace Demo {

    namespace Clock = ESPressio::Clock;

    /// Deterministic nanosecond timebase used by the Stopwatch demonstration.
    class Timebase final : public Clock::MonotonicTimebaseProvider<1000000000U> {
    private:

        // Demonstration count.

        /// Simulated physical counter.
        std::uint64_t _count{0U};

    public:

        // Timebase access.

        /// Returns the current count.
        std::uint64_t CurrentCount() const noexcept {
            return _count;
        }

        /// Advances the demonstration timebase.
        void Advance(
            std::uint64_t nanoseconds
        ) noexcept {
            _count += nanoseconds;
        }

    };


    /// Runs the Stopwatch demonstration.
    int Run() noexcept {
        Timebase timebase;
        Clock::MonotonicClockProvider<Timebase> clock(timebase);
        Clock::Stopwatch<Clock::MonotonicClockProvider<Timebase>> stopwatch(clock);

        stopwatch.Start();
        timebase.Advance(2000000U);
        stopwatch.Stop();

        stopwatch.Start();
        timebase.Advance(500000U);
        stopwatch.Stop();

        if (stopwatch.Elapsed().Nanoseconds() != 2500000) return 1;

        stopwatch.Restart();
        timebase.Advance(125000U);

        return stopwatch.Elapsed().Nanoseconds() == 125000 ? 0 : 2;
    }

} // Demo

/// Runs the demonstration from the ESP-IDF application entry point.
extern "C" void app_main() {
    static_cast<void>(Demo::Run());
}
