#include <atomic>
#include <cassert>
#include <cstdint>

#include <ESPressio_Clock.hpp>

namespace {

    namespace Framework = ESPressio::System::CompositionFramework;

    /// Host-test concrete provider for the Platform AtomicWord32 capability.
    class TestAtomicWord32Provider final : public Framework::Provider<
        ESPressio::Platform::Domain,
        Framework::Offers<
            Framework::Offer<
                ESPressio::Platform::Concurrency::AtomicWord32,
                Framework::PropertyValue<ESPressio::Platform::Concurrency::LockFree, true>,
                Framework::PropertyValue<ESPressio::Platform::Concurrency::AtomicWordStorageBytes, 4U>
            >
        >
    > {
    public:

        /// Four-byte lock-free host-test atomic word.
        class Word final {
        private:

            /// Native atomic value used only by host validation.
            std::atomic<std::uint32_t> _value;

        public:

            /// Creates an atomic word initialized to zero.
            constexpr Word() noexcept :
                _value(0U) {}

            /// Prevents copying live atomic storage.
            Word(const Word&) = delete;

            /// Prevents copy assignment of live atomic storage.
            Word& operator =(const Word&) = delete;

            /// Prevents moving live atomic storage.
            Word(Word&&) = delete;

            /// Prevents move assignment of live atomic storage.
            Word& operator =(Word&&) = delete;

            /// Reads without cross-word ordering.
            std::uint32_t LoadRelaxed() const noexcept {
                return _value.load(std::memory_order_relaxed);
            }

            /// Reads with acquire ordering.
            std::uint32_t LoadAcquire() const noexcept {
                return _value.load(std::memory_order_acquire);
            }

            /// Stores without cross-word ordering.
            void StoreRelaxed(
                std::uint32_t value
            ) noexcept {
                _value.store(value, std::memory_order_relaxed);
            }

            /// Stores with release ordering.
            void StoreRelease(
                std::uint32_t value
            ) noexcept {
                _value.store(value, std::memory_order_release);
            }

            /// Performs acquire/release compare-exchange.
            bool CompareExchangeAcqRel(
                std::uint32_t& expected,
                std::uint32_t desired
            ) noexcept {
                return _value.compare_exchange_strong(
                    expected,
                    desired,
                    std::memory_order_acq_rel,
                    std::memory_order_acquire
                );
            }

        };

    };


    static_assert(std::atomic<std::uint32_t>::is_always_lock_free);
    static_assert(sizeof(std::atomic<std::uint32_t>) == sizeof(std::uint32_t));


    /// Deterministic nanosecond timebase for correlation/provider integration tests.
    class TestTimebase final : public ESPressio::Clock::MonotonicTimebaseProvider<1000000000U> {
    private:

        /// Current source count; one count represents one nanosecond.
        std::uint64_t _count{0U};

    public:

        /// Returns the current deterministic source count.
        std::uint64_t CurrentCount() const noexcept {
            return _count;
        }

        /// Replaces the current deterministic source count.
        void Set(
            std::uint64_t count
        ) noexcept {
            _count = count;
        }

    };


    using MonotonicClock = ESPressio::Clock::MonotonicClockProvider<TestTimebase>;
    using SynchronizedClock = ESPressio::Clock::SynchronizedClockProvider<
        MonotonicClock,
        TestAtomicWord32Provider
    >;


    /// Creates one deterministic synchronization observation.
    ESPressio::Clock::SynchronizationObservation Observation(
        std::uint64_t monotonicNanoseconds,
        std::uint64_t referenceNanoseconds,
        std::uint64_t uncertaintyNanoseconds
    ) noexcept {
        return ESPressio::Clock::SynchronizationObservation(
            ESPressio::Clock::MonotonicTimestamp::FromNanoseconds(monotonicNanoseconds),
            ESPressio::Clock::SynchronizedTimestamp::FromNanoseconds(referenceNanoseconds),
            ESPressio::Clock::SynchronizationUncertainty::FromNanoseconds(uncertaintyNanoseconds)
        );
    }

} // anonymous namespace


int main() {
    using ESPressio::Clock::ClockCorrelationState;
    using ESPressio::Clock::SynchronizationObservationStatus;
    using ESPressio::Clock::SynchronizationState;

    // Correlation remains unavailable until Clock accepts reference evidence.
    TestTimebase poorTimebase;
    MonotonicClock poorMonotonic(poorTimebase);
    SynchronizedClock poorClock(poorMonotonic);
    assert(poorClock.Correlation().State() == ClockCorrelationState::Unavailable);

    // Evidence may be useful for correlation even when it cannot satisfy the <1 ms public-clock contract.
    poorTimebase.Set(1000000000ULL);
    assert(
        poorClock.Observe(
            Observation(
                1000000000ULL,
                10000000000ULL,
                1000000U
            )
        ) == SynchronizationObservationStatus::Accepted
    );
    assert(poorClock.Now().State() == SynchronizationState::NeverSynchronized);
    const auto poorCorrelation = poorClock.Correlation();
    assert(poorCorrelation.IsAvailable());
    assert(poorCorrelation.MonotonicAnchor().Nanoseconds() == 1000000000ULL);
    assert(poorCorrelation.SynchronizedAnchor().Nanoseconds() == 10000000000ULL);
    assert(poorCorrelation.AnchorUncertainty().Nanoseconds() == 1000001U);

    // Once synchronized, a newer raw reference may improve event correlation immediately while the
    // published clock deliberately remains non-regressing and therefore enters Reacquiring.
    TestTimebase slewTimebase;
    MonotonicClock slewMonotonic(slewTimebase);
    SynchronizedClock slewClock(slewMonotonic);
    slewTimebase.Set(1000000000ULL);
    assert(
        slewClock.Observe(
            Observation(
                1000000000ULL,
                10000000000ULL,
                100U
            )
        ) == SynchronizationObservationStatus::Accepted
    );
    assert(slewClock.Now().State() == SynchronizationState::Synchronized);

    assert(
        slewClock.Observe(
            Observation(
                1000000000ULL,
                9998000000ULL,
                100U
            )
        ) == SynchronizationObservationStatus::Accepted
    );
    const auto published = slewClock.Now();
    const auto rawCorrelation = slewClock.Correlation();
    assert(published.State() == SynchronizationState::Reacquiring);
    assert(published.Timestamp().Nanoseconds() == 10000000000ULL);
    assert(rawCorrelation.SynchronizedAnchor().Nanoseconds() == 9998000000ULL);

    const auto atAnchor = rawCorrelation.Correlate(
        ESPressio::Clock::MonotonicTimestamp::FromNanoseconds(1000000000ULL),
        ESPressio::Clock::SynchronizationUncertainty::Zero()
    );
    assert(atAnchor.IsCorrelated());
    assert(atAnchor.Timestamp().Nanoseconds() == 9998000000ULL);

    // The retained long-term frequency estimate is included in newly captured correlations.
    TestTimebase rateTimebase;
    MonotonicClock rateMonotonic(rateTimebase);
    SynchronizedClock rateClock(rateMonotonic);
    rateTimebase.Set(1000000000ULL);
    assert(
        rateClock.Observe(
            Observation(
                1000000000ULL,
                10000000000ULL,
                100U
            )
        ) == SynchronizationObservationStatus::Accepted
    );
    rateTimebase.Set(2000000000ULL);
    assert(
        rateClock.Observe(
            Observation(
                2000000000ULL,
                11000100000ULL,
                100U
            )
        ) == SynchronizationObservationStatus::Accepted
    );
    assert(rateClock.Correlation().FrequencyCorrectionPartsPerBillion() == 100000);

    return 0;
}
