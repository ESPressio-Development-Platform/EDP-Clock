#include <Arduino.h>\n\n#include <ESPressio_Platform_Arduino.hpp>

#include <cstdint>

#include <ESPressio_Clock.hpp>

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

        /// Current nanosecond count.
        CountSnapshot _count;

    public:

        /// Creates the timebase at zero.
        Timebase() noexcept :
            _count(0U) {}

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


    /// Demonstrates canonical capture bounds, Era qualification and explicit transitions.
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

        timebase.Advance(1000U);
+
        if (
            synchronizedClock.Observe(
                Clock::SynchronizationObservation(
                    monotonicClock.Now(),
                    Clock::SynchronizedTimestamp::FromNanoseconds(1000001000ULL),
                    Clock::SynchronizationUncertainty::FromNanoseconds(100U)
                )
            ) != Clock::SynchronizationObservationStatus::Accepted
        ) {
            return 1;
        }

        const auto capture = Clock::MonotonicCaptureBounds::Between(
            Clock::MonotonicTimestamp::FromNanoseconds(996U),
            Clock::MonotonicTimestamp::FromNanoseconds(1004U)
        );
        const auto era = Clock::ClockEra::FromValue(1U);
        const auto correlation = Clock::EraQualifiedClockCorrelation(
            era,
            synchronizedClock.Correlation()
        );
        const auto projection = correlation.Correlate(
            capture.Midpoint(),
            capture.Uncertainty()
        );
        const auto assessment = Clock::EraQualifiedClockAssessment(
            projection,
            Clock::TemporalQualityAssessment::Reliable
        );

        if (!capture.IsAvailable()) return 2;
        if (!assessment.IsCorrelated()) return 3;
        if (assessment.Era() != era) return 4;
        if (assessment.Assessment() != Clock::TemporalQualityAssessment::Reliable) return 5;

        if (
            synchronizedClock.BeginSameEraTransition() !=
            Clock::ClockDisciplineTransitionStatus::Applied
        ) {
            return 6;
        }

        if (synchronizedClock.Now().State() != Clock::SynchronizationState::Reacquiring) return 7;

        if (
            synchronizedClock.Observe(
                Clock::SynchronizationObservation(
                    monotonicClock.Now(),
                    Clock::SynchronizedTimestamp::FromNanoseconds(1000001000ULL),
                    Clock::SynchronizationUncertainty::FromNanoseconds(100U)
                )
            ) != Clock::SynchronizationObservationStatus::Accepted
        ) {
            return 8;
        }

        if (
            synchronizedClock.BeginCrossEraTransition(
                Clock::CrossEraTransitionMode::PreservePublishedTimeline
            ) != Clock::ClockDisciplineTransitionStatus::Applied
        ) {
            return 9;
        }

        if (synchronizedClock.Correlation().IsAvailable()) return 10;
        if (synchronizedClock.Now().State() != Clock::SynchronizationState::LostSynchronization) return 11;

        if (
            synchronizedClock.BeginCrossEraTransition(
                Clock::CrossEraTransitionMode::ReconstructTimeline
            ) != Clock::ClockDisciplineTransitionStatus::Applied
        ) {
            return 12;
        }

        if (synchronizedClock.Now().State() != Clock::SynchronizationState::NeverSynchronized) return 13;

        return 0;
    }

} // Demo

/// Runs the demonstration once during Arduino initialization.\nvoid setup() {\n    static_cast<void>(Demo::Run());\n}\n\n/// Leaves the demonstration idle after the one-time run.\nvoid loop() {}
