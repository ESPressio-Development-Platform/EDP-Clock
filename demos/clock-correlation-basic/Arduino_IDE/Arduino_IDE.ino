#include <Arduino.h>

#include <cstdint>

#include <ESPressio_Clock.hpp>
#include <ESPressio_Platform_Arduino.hpp>

namespace Demo {

    namespace Clock = ESPressio::Clock;

    /// Deterministic nanosecond timebase with coherent publication.
    template<class TAtomicProvider>
    class Timebase final : public Clock::MonotonicTimebaseProvider<1000000000U> {
    private:

        /// Coherent count publication.
        using CountSnapshot = ESPressio::Platform::Concurrency::ConcurrentSnapshot<
            std::uint64_t,
            TAtomicProvider
        >;

        // Demonstration source.

        /// Current nanosecond count.
        CountSnapshot _count;

    public:

        // Construction.

        /// Creates the timebase at zero.
        Timebase() noexcept :
            _count(0U) {}


        // Timebase access.

        /// Returns the current physical count.
        std::uint64_t CurrentCount() const noexcept {
            return _count.Read();
        }

        /// Advances the deterministic timebase.
        void Advance(
            std::uint64_t nanoseconds
        ) noexcept {
            _count.Publish(
                _count.Read() + nanoseconds
            );
        }

    };


    /// Runs the Clock Correlation demonstration.
    int Run() noexcept {
        using AtomicProvider = ESPressio::Platform::Arduino::Concurrency::AtomicWord32Provider;
        using ApplicationTimebase = Timebase<AtomicProvider>;
        using MonotonicClock = Clock::MonotonicClockProvider<ApplicationTimebase>;
        using SynchronizedClock = Clock::SynchronizedClockProvider<
            MonotonicClock,
            AtomicProvider
        >;

        ApplicationTimebase timebase;
        MonotonicClock monotonicClock(timebase);
        SynchronizedClock synchronizedClock(monotonicClock);

        timebase.Advance(1000000000ULL);

        const auto observation = Clock::SynchronizationObservation(
            monotonicClock.Now(),
            Clock::SynchronizedTimestamp::FromNanoseconds(
                42000000000ULL
            ),
            Clock::SynchronizationUncertainty::FromNanoseconds(
                200U
            )
        );

        if (
            synchronizedClock.Observe(
                observation
            ) != Clock::SynchronizationObservationStatus::Accepted
        ) {
            return 1;
        }

        const auto correlation = synchronizedClock.Correlation();

        if (!correlation.IsAvailable()) return 2;

        timebase.Advance(250000U);
        const auto captured = monotonicClock.Now();

        // Processing occurs later, but the captured monotonic coordinate is retained.
        timebase.Advance(5000000U);

        const auto projection = correlation.Correlate(
            captured,
            Clock::SynchronizationUncertainty::FromNanoseconds(
                100U
            )
        );

        if (!projection.IsCorrelated()) return 3;
        if (projection.Timestamp().Nanoseconds() != 42000250000ULL) return 4;

        return 0;
    }

} // Demo

/// Runs the demonstration once during Arduino initialization.
void setup() {
    static_cast<void>(Demo::Run());
}

/// Leaves the demonstration idle after the one-time run.
void loop() {}
