#include <cstdint>

#include <ESPressio_Clock.hpp>
#include <ESPressio_Platform_ESP_IDF.hpp>

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


    /// Runs the synchronized Clock demonstration.
    int Run() noexcept {
        using AtomicProvider = ESPressio::Platform::ESPIDF::Concurrency::AtomicWord32Provider;
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
        const auto t1 = monotonicClock.Now();

        const auto exchange = Clock::FourTimestampExchange(
            t1,
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

        timebase.Advance(1000U);

        const auto estimate = Clock::EstimateFourTimestampExchange(
            exchange
        );

        if (!estimate.IsAccepted()) return 1;

        if (
            synchronizedClock.Observe(
                estimate.Observation()
            ) != Clock::SynchronizationObservationStatus::Accepted
        ) {
            return 2;
        }

        const auto reading = synchronizedClock.Now();

        if (reading.State() != Clock::SynchronizationState::Synchronized) return 3;
        if (reading.Uncertainty().Nanoseconds() >= 1000000U) return 4;

        return 0;
    }

} // Demo

/// Runs the demonstration from the ESP-IDF application entry point.
extern "C" void app_main() {
    static_cast<void>(Demo::Run());
}
