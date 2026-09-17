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

    // T1 is captured locally as close as possible to request transmission.
    timebase.Advance(1000000000ULL);
    const auto t1 = ESPressio::Clock::MonotonicNow();

    // The transport carries remote/reference T2 and T3 timestamps back with the response. In this
    // deterministic example, the request takes 400 ns to reach the remote reference, the remote
    // system retains it for 200 ns, and the response takes 400 ns to return.
    const auto t2 = ESPressio::Clock::SynchronizedTimestamp::FromNanoseconds(
        42000000400ULL
    );
    const auto t3 = ESPressio::Clock::SynchronizedTimestamp::FromNanoseconds(
        42000000600ULL
    );

    // T4 is captured locally as close as possible to response reception.
    timebase.Advance(1000U);
    const auto t4 = ESPressio::Clock::MonotonicNow();

    const auto exchange = ESPressio::Clock::FourTimestampExchange(
        t1,
        ESPressio::Clock::SynchronizationUncertainty::FromNanoseconds(
            50U
        ),
        t2,
        ESPressio::Clock::SynchronizationUncertainty::FromNanoseconds(
            50U
        ),
        t3,
        ESPressio::Clock::SynchronizationUncertainty::FromNanoseconds(
            50U
        ),
        t4,
        ESPressio::Clock::SynchronizationUncertainty::FromNanoseconds(
            50U
        )
    );

    const auto estimate = ESPressio::Clock::EstimateFourTimestampExchange(
        exchange
    );

    if (!estimate.IsAccepted()) return 1;

    if (
        synchronizedClock.Observe(
            estimate.Observation()
        ) != ESPressio::Clock::SynchronizationObservationStatus::Accepted
    ) {
        return 2;
    }

    // Every consumer gets the synchronized coordinate together with its quality information.
    const auto reading = ESPressio::Clock::SynchronizedNow();

    if (reading.State() != ESPressio::Clock::SynchronizationState::Synchronized) return 3;
    if (reading.Uncertainty().Nanoseconds() >= 1000000U) return 4;
    if (reading.Timestamp().Nanoseconds() != 42000001000ULL) return 5;

    return 0;
}
