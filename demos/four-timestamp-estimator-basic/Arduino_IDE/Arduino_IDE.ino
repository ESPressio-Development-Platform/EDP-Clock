#include <Arduino.h>

#include <ESPressio_Clock.hpp>

namespace Demo {

    namespace Clock = ESPressio::Clock;

    /// Runs the four-timestamp time-transfer estimator demonstration.
    int Run() noexcept {
        const auto exchange = Clock::FourTimestampExchange(
            Clock::MonotonicTimestamp::FromNanoseconds(
                1000000000ULL
            ),
            Clock::SynchronizationUncertainty::FromNanoseconds(
                50U
            ),
            Clock::SynchronizedTimestamp::FromNanoseconds(
                42000000400ULL
            ),
            Clock::SynchronizationUncertainty::FromNanoseconds(
                50U
            ),
            Clock::SynchronizedTimestamp::FromNanoseconds(
                42000000600ULL
            ),
            Clock::SynchronizationUncertainty::FromNanoseconds(
                50U
            ),
            Clock::MonotonicTimestamp::FromNanoseconds(
                1000001000ULL
            ),
            Clock::SynchronizationUncertainty::FromNanoseconds(
                50U
            )
        );

        const auto result = Clock::EstimateFourTimestampExchange(
            exchange
        );

        if (!result.IsAccepted()) return 1;
        if (result.Observation().LocalTimestamp().Nanoseconds() != 1000001000ULL) return 2;
        if (result.Observation().ReferenceTimestamp().Nanoseconds() != 42000001000ULL) return 3;

        return 0;
    }

} // Demo

/// Runs the demonstration once during Arduino initialization.
void setup() {
    static_cast<void>(Demo::Run());
}

/// Leaves the demonstration idle after the one-time run.
void loop() {}
