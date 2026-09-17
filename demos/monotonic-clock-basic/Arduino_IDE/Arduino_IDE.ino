#include <Arduino.h>

#include <cstdint>

#include <ESPressio_Clock.hpp>

namespace Demo {

    namespace Clock = ESPressio::Clock;

    /// Deterministic one-megahertz timebase used by the demonstration.
    class Timebase final : public Clock::MonotonicTimebaseProvider<1000000U> {
    private:

        // Demonstration count.

        /// Simulated physical counter.
        std::uint64_t _count{0U};

    public:

        // Timebase access.

        /// Returns the current physical count.
        std::uint64_t CurrentCount() const noexcept {
            return _count;
        }

        /// Advances the demonstration timebase.
        void Advance(
            std::uint64_t count
        ) noexcept {
            _count += count;
        }

    };


    /// Runs the monotonic Clock demonstration.
    int Run() noexcept {
        Timebase timebase;
        Clock::MonotonicClockProvider<Timebase> clock(timebase);

        const auto started = clock.Now();

        timebase.Advance(2500U);

        const auto finished = clock.Now();
        const auto elapsed = Clock::Delta(
            started,
            finished
        );

        return elapsed.Nanoseconds() == 2500000 ? 0 : 1;
    }

} // Demo

/// Runs the demonstration once during Arduino initialization.
void setup() {
    static_cast<void>(Demo::Run());
}

/// Leaves the demonstration idle after the one-time run.
void loop() {}
