#include <cstdint>

#include <ESPressio_Clock.hpp>
#include <ESPressio_Platform_ESP_IDF.hpp>

namespace Example {

    /// Deterministic nanosecond timebase used only to demonstrate Clock wiring.
    ///
    /// A real application would normally obtain its MonotonicTimebase from the appropriate
    /// hardware/platform library. This small source keeps the example focused on synchronization
    /// while still satisfying the concurrent-read contract using EDP-Platform fixed storage.
    template<class TAtomicWordProvider>
    class ExampleTimebase final : public ESPressio::Clock::MonotonicTimebaseProvider<1000000000U> {
    private:

        /// Coherent fixed-storage count publication for concurrent readers.
        using CountSnapshot = ESPressio::Platform::Concurrency::ConcurrentSnapshot<
            std::uint64_t,
            TAtomicWordProvider
        >;

        // Demonstration source state.

        /// Current example count; one count represents one nanosecond.
        CountSnapshot _count;

    public:

        // Construction.

        /// Creates the demonstration source at the monotonic origin.
        ExampleTimebase() noexcept :
            _count(0U) {}


        // Timebase access.

        /// Returns the current demonstration source count coherently to concurrent readers.
        std::uint64_t CurrentCount() const noexcept {
            return _count.Read();
        }

        /// Advances the deterministic demonstration source.
        ///
        /// The example has one writer, matching ConcurrentSnapshot's publication contract.
        void Advance(
            std::uint64_t nanoseconds
        ) noexcept {
            _count.Publish(
                _count.Read() + nanoseconds
            );
        }

    };

} // Example


int main() {
    using AtomicWordProvider = ESPressio::Platform::ESPIDF::Concurrency::AtomicWord32Provider;
    using Timebase = Example::ExampleTimebase<AtomicWordProvider>;
    using MonotonicClock = ESPressio::Clock::MonotonicClockProvider<Timebase>;
    using SynchronizedClock = ESPressio::Clock::SynchronizedClockProvider<
        MonotonicClock,
        AtomicWordProvider
    >;

    Timebase timebase;
    MonotonicClock monotonicClock(timebase);
    SynchronizedClock synchronizedClock(monotonicClock);

    // Bootstrap permanently exposes the two application Clock convenience calls.
    ESPressio::Clock::BindMonotonicClock(
        monotonicClock
    );
    ESPressio::Clock::BindSynchronizedClock(
        synchronizedClock
    );

    // A synchronization source/transport would normally capture this local timestamp as close as
    // possible to the physical observation event, then calculate reference time and uncertainty.
    timebase.Advance(1000000000ULL);
    const auto localCapture = ESPressio::Clock::MonotonicNow();

    const auto observation = ESPressio::Clock::SynchronizationObservation(
        localCapture,
        ESPressio::Clock::SynchronizedTimestamp::FromNanoseconds(
            42000000000ULL
        ),
        ESPressio::Clock::SynchronizationUncertainty::FromNanoseconds(
            250U
        )
    );

    if (
        synchronizedClock.Observe(
            observation
        ) != ESPressio::Clock::SynchronizationObservationStatus::Accepted
    ) {
        return 1;
    }

    // Every consumer gets the synchronized coordinate together with its quality information.
    const auto reading = ESPressio::Clock::SynchronizedNow();

    if (reading.State() != ESPressio::Clock::SynchronizationState::Synchronized) return 2;
    if (reading.Uncertainty().Nanoseconds() >= 1000000U) return 3;
    if (reading.Timestamp().Nanoseconds() != 42000000000ULL) return 4;

    return 0;
}
