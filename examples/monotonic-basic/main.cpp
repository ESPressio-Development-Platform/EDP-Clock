#include <cstdint>

#include "ESPressio_Clock.hpp"

namespace Example {

    namespace Clock = ESPressio::Clock;
    namespace Framework = ESPressio::System::CompositionFramework;

    /// Minimal example timebase representing a 1 MHz physical/electronic counter.
    ///
    /// A real provider would normally live in a platform, hardware, RTC, or application library.
    /// This example keeps the source local so the Clock-domain bootstrap can be read end-to-end.
    class ApplicationTimebase final : public Clock::MonotonicTimebaseProvider<1000000U> {
    private:

        // Demonstration count.

        /// Simulated stable counter value used by this single-threaded example.
        std::uint64_t _count{0U};

    public:

        // Timebase access.

        /// Returns the current physical count.
        ///
        /// Production providers must uphold the MonotonicTimebase concurrent-read contract.
        std::uint64_t CurrentCount() const noexcept {
            return _count;
        }


        // Demonstration control.

        /// Advances this example by a specified number of 1 MHz counts.
        ///
        /// @param count Number of counts to advance before the next read.
        void Advance(
            std::uint64_t count
        ) noexcept {
            _count += count;
        }

    };


    /// Standard EDP-Clock monotonic clock built over the application-selected timebase.
    using ApplicationClock =
        Clock::MonotonicClockProvider<ApplicationTimebase>;


    /// Compile-time Clock architecture selected by application Bootstrap.
    using ApplicationComposition = Framework::Composition<
        Clock::Domain,
        ApplicationTimebase,
        ApplicationClock
    >;


    static_assert(
        ApplicationComposition::IsValid,
        "Application Clock composition must be valid"
    );


    /// Demonstrates complete bootstrap, convenience binding, and Delta calculation.
    int Run() noexcept {
        ApplicationTimebase timebase;
        ApplicationClock monotonicClock(timebase);

        if (!Clock::BindMonotonicClock(monotonicClock)) return 1;

        const auto started = Clock::MonotonicNow();

        // At 1 MHz, 2500 counts represent exactly 2,500,000 nanoseconds.
        timebase.Advance(2500U);

        const auto finished = Clock::MonotonicNow();
        const auto elapsed = Clock::Delta(
            started,
            finished
        );

        if (elapsed.Nanoseconds() != 2500000) return 2;

        return 0;
    }

} // Example


/// Executes the basic monotonic Clock example.
int main() {
    return Example::Run();
}
