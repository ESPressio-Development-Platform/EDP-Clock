#include <atomic>
#include <cassert>
#include <cstdint>
#include <limits>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include <ESPressio_Clock.hpp>

ESPressio::Clock::SynchronizedReading ReadSynchronizedFromAnotherTranslationUnit() noexcept;

namespace {

    namespace Framework = ESPressio::System::CompositionFramework;

    /// Host-test concrete provider for the Platform AtomicWord32 capability.
    class TestAtomicWord32Provider final : public Framework::Provider<
        ESPressio::Platform::Domain,
        Framework::Provides<
            Framework::Offer<
                ESPressio::Platform::Concurrency::AtomicWord32,
                Framework::PropertyValue<ESPressio::Platform::Concurrency::LockFree, true>,
                Framework::PropertyValue<ESPressio::Platform::Concurrency::AtomicWordStorageBytes, 4U>
            >
        >
    > {
    public:

        /// Lock-free host-test atomic word.
        class Word final {
        private:

            // Atomic storage.

            /// Native atomic value used only by host validation.
            std::atomic<std::uint32_t> _value;

        public:

            // Construction and lifetime.

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


            // Atomic loads.

            /// Reads without cross-word ordering.
            std::uint32_t LoadRelaxed() const noexcept {
                return _value.load(
                    std::memory_order_relaxed
                );
            }

            /// Reads with acquire ordering.
            std::uint32_t LoadAcquire() const noexcept {
                return _value.load(
                    std::memory_order_acquire
                );
            }


            // Atomic stores.

            /// Stores without cross-word ordering.
            void StoreRelaxed(
                std::uint32_t value
            ) noexcept {
                _value.store(
                    value,
                    std::memory_order_relaxed
                );
            }

            /// Stores with release ordering.
            void StoreRelease(
                std::uint32_t value
            ) noexcept {
                _value.store(
                    value,
                    std::memory_order_release
                );
            }


            // Atomic compare/exchange.

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


    static_assert(
        std::atomic<std::uint32_t>::is_always_lock_free,
        "Host synchronized-clock tests require lock-free 32-bit atomics"
    );

    static_assert(
        sizeof(std::atomic<std::uint32_t>) == sizeof(std::uint32_t),
        "Host synchronized-clock tests require four-byte 32-bit atomics"
    );


    /// Thread-safe nanosecond timebase used to control deterministic test time.
    class TestTimebase final : public ESPressio::Clock::MonotonicTimebaseProvider<1000000000U> {
    private:

        // Controlled source state.

        /// Current source count; one count represents one nanosecond.
        std::atomic<std::uint64_t> _count;

    public:

        // Construction.

        /// Creates the test timebase at the monotonic origin.
        TestTimebase() noexcept :
            _count(0U) {}


        // Timebase access.

        /// Returns the current test count coherently to concurrent readers.
        std::uint64_t CurrentCount() const noexcept {
            return _count.load(
                std::memory_order_acquire
            );
        }

        /// Replaces the current test count.
        void Set(
            std::uint64_t count
        ) noexcept {
            _count.store(
                count,
                std::memory_order_release
            );
        }

        /// Advances the current test count by a fixed amount.
        void Advance(
            std::uint64_t delta
        ) noexcept {
            _count.fetch_add(
                delta,
                std::memory_order_acq_rel
            );
        }

    };


    using MonotonicClock = ESPressio::Clock::MonotonicClockProvider<TestTimebase>;
    using SynchronizedClock = ESPressio::Clock::SynchronizedClockProvider<
        MonotonicClock,
        TestAtomicWord32Provider
    >;


    /// Creates an observation at the supplied monotonic/reference coordinates.
    ESPressio::Clock::SynchronizationObservation Observation(
        std::uint64_t monotonicNanoseconds,
        std::uint64_t referenceNanoseconds,
        std::uint64_t uncertaintyNanoseconds
    ) noexcept {
        return ESPressio::Clock::SynchronizationObservation(
            ESPressio::Clock::MonotonicTimestamp::FromNanoseconds(
                monotonicNanoseconds
            ),
            ESPressio::Clock::SynchronizedTimestamp::FromNanoseconds(
                referenceNanoseconds
            ),
            ESPressio::Clock::SynchronizationUncertainty::FromNanoseconds(
                uncertaintyNanoseconds
            )
        );
    }


    template<class TLeft, class TRight, class = void>
    struct CanDelta : std::false_type {};


    /// Compile-time detection for a valid Delta overload between two value types.
    template<class TLeft, class TRight>
    struct CanDelta<
        TLeft,
        TRight,
        std::void_t<decltype(
            ESPressio::Clock::Delta(
                std::declval<TLeft>(),
                std::declval<TRight>()
            )
        )>
    > : std::true_type {};

} // anonymous namespace


int main() {
    using ESPressio::Clock::SynchronizationObservationStatus;
    using ESPressio::Clock::SynchronizationState;

    // Value types and cross-domain arithmetic boundaries.
    static_assert(sizeof(ESPressio::Clock::SynchronizedTimestamp) == 8U);
    static_assert(sizeof(ESPressio::Clock::SynchronizationUncertainty) == 4U);
    static_assert(sizeof(ESPressio::Clock::SynchronizedReading) == 16U);
    static_assert(std::is_trivially_copyable_v<ESPressio::Clock::SynchronizedTimestamp>);
    static_assert(std::is_trivially_copyable_v<ESPressio::Clock::SynchronizationUncertainty>);
    static_assert(std::is_trivially_copyable_v<ESPressio::Clock::SynchronizedReading>);
    static_assert(
        CanDelta<
            ESPressio::Clock::SynchronizedTimestamp,
            ESPressio::Clock::SynchronizedTimestamp
        >::value
    );
    static_assert(
        !CanDelta<
            ESPressio::Clock::MonotonicTimestamp,
            ESPressio::Clock::SynchronizedTimestamp
        >::value
    );
    static_assert(
        !CanDelta<
            ESPressio::Clock::SynchronizedTimestamp,
            ESPressio::Clock::MonotonicTimestamp
        >::value
    );

    const auto synchronizedFrom = ESPressio::Clock::SynchronizedTimestamp::FromNanoseconds(5000U);
    const auto synchronizedTo = ESPressio::Clock::SynchronizedTimestamp::FromNanoseconds(9000U);
    assert(
        ESPressio::Clock::Delta(
            synchronizedFrom,
            synchronizedTo
        ).Nanoseconds() == 4000
    );
    assert(
        ESPressio::Clock::Delta(
            synchronizedTo,
            synchronizedFrom
        ).Nanoseconds() == -4000
    );

    // Composition and fixed-memory policy metadata.
    static_assert(SynchronizedClock::SynchronizationUncertaintyLimitNanoseconds == 1000000U);
    static_assert(SynchronizedClock::MaximumFrequencyCorrectionPartsPerBillion == 1000000U);
    static_assert(SynchronizedClock::MaximumPhaseSlewPartsPerBillion == 5000000U);
    static_assert(SynchronizedClock::DisciplineStateBytes == 48U);
    static_assert(SynchronizedClock::ConcurrentStateStorageBytes == 104U);
    static_assert(
        SynchronizedClock::CompositionCapabilities::template Contains<ESPressio::Clock::SynchronizedClock>
    );
    using SynchronizedProperties =
        typename SynchronizedClock::CompositionCapabilities::template PropertiesFor<ESPressio::Clock::SynchronizedClock>;
    static_assert(
        SynchronizedProperties::template Value<ESPressio::Clock::SynchronizedClockUncertaintyLimitNanoseconds> == 1000000U
    );

    // Initial state is explicitly NeverSynchronized and maximally uncertain.
    TestTimebase initialTimebase;
    MonotonicClock initialMonotonic(initialTimebase);
    SynchronizedClock initialClock(initialMonotonic);
    const auto initialReading = initialClock.Now();
    assert(initialReading.State() == SynchronizationState::NeverSynchronized);
    assert(initialReading.Timestamp().Nanoseconds() == 0U);
    assert(initialReading.Uncertainty().IsSaturated());

    // A first observation that already satisfies the quality contract establishes the mapping immediately.
    initialTimebase.Set(1000U);
    assert(
        initialClock.Observe(
            Observation(
                1000U,
                9000001000ULL,
                100U
            )
        ) == SynchronizationObservationStatus::Accepted
    );
    const auto established = initialClock.Now();
    assert(established.State() == SynchronizationState::Synchronized);
    assert(established.Timestamp().Nanoseconds() == 9000001000ULL);
    assert(established.Uncertainty().Nanoseconds() == 101U);

    initialTimebase.Advance(100000U);
    const auto progressed = initialClock.Now();
    assert(progressed.Timestamp().Nanoseconds() > established.Timestamp().Nanoseconds());

    // Poor first observations may improve a candidate mapping but cannot falsely claim synchronization.
    TestTimebase poorTimebase;
    MonotonicClock poorMonotonic(poorTimebase);
    SynchronizedClock poorClock(poorMonotonic);
    poorTimebase.Set(100U);
    assert(
        poorClock.Observe(
            Observation(
                100U,
                100000U,
                1000000U
            )
        ) == SynchronizationObservationStatus::Accepted
    );
    assert(poorClock.Now().State() == SynchronizationState::NeverSynchronized);

    poorTimebase.Set(1000100U);
    assert(
        poorClock.Observe(
            Observation(
                1000100U,
                1100000U,
                100U
            )
        ) == SynchronizationObservationStatus::Accepted
    );
    assert(poorClock.Now().State() == SynchronizationState::Synchronized);

    // Once the runtime has synchronized, subsequent negative phase correction never steps time backwards.
    TestTimebase slewTimebase;
    MonotonicClock slewMonotonic(slewTimebase);
    SynchronizedClock slewClock(slewMonotonic);
    slewTimebase.Set(1000000000ULL);
    assert(
        slewClock.Observe(
            Observation(
                1000000000ULL,
                10000000000ULL,
                0U
            )
        ) == SynchronizationObservationStatus::Accepted
    );
    const auto beforeNegativeCorrection = slewClock.Now();

    assert(
        slewClock.Observe(
            Observation(
                1000000000ULL,
                9998000000ULL,
                0U
            )
        ) == SynchronizationObservationStatus::Accepted
    );
    const auto immediatelyAfterNegativeCorrection = slewClock.Now();
    assert(
        immediatelyAfterNegativeCorrection.Timestamp().Nanoseconds() ==
        beforeNegativeCorrection.Timestamp().Nanoseconds()
    );
    assert(immediatelyAfterNegativeCorrection.State() == SynchronizationState::Reacquiring);

    slewTimebase.Advance(100000000ULL);
    const auto duringNegativeSlew = slewClock.Now();
    assert(
        duringNegativeSlew.Timestamp().Nanoseconds() >
        immediatelyAfterNegativeCorrection.Timestamp().Nanoseconds()
    );
    assert(
        ESPressio::Clock::Delta(
            immediatelyAfterNegativeCorrection.Timestamp(),
            duringNegativeSlew.Timestamp()
        ).Nanoseconds() == 99500000
    );

    // A 2 ms phase error is removed at the default 0.5% slew rate in 400 ms without stepping.
    TestTimebase reacquireTimebase;
    MonotonicClock reacquireMonotonic(reacquireTimebase);
    SynchronizedClock reacquireClock(reacquireMonotonic);
    reacquireTimebase.Set(1U);
    assert(
        reacquireClock.Observe(
            Observation(
                1U,
                1000000001ULL,
                0U
            )
        ) == SynchronizationObservationStatus::Accepted
    );
    assert(
        reacquireClock.Observe(
            Observation(
                1U,
                1002000001ULL,
                0U
            )
        ) == SynchronizationObservationStatus::Accepted
    );
    assert(reacquireClock.Now().State() == SynchronizationState::Reacquiring);

    reacquireTimebase.Advance(400000000ULL);
    const auto reacquired = reacquireClock.Now();
    assert(reacquired.State() == SynchronizationState::Synchronized);
    assert(reacquired.Uncertainty().Nanoseconds() < 1000000U);

    // No fresh observation causes uncertainty to grow into Holdover and eventually LostSynchronization.
    TestTimebase holdoverTimebase;
    MonotonicClock holdoverMonotonic(holdoverTimebase);
    SynchronizedClock holdoverClock(holdoverMonotonic);
    holdoverTimebase.Set(1U);
    assert(
        holdoverClock.Observe(
            Observation(
                1U,
                5000000001ULL,
                0U
            )
        ) == SynchronizationObservationStatus::Accepted
    );

    holdoverTimebase.Advance(999998000ULL);
    const auto justInsideLimit = holdoverClock.Now();
    assert(justInsideLimit.State() == SynchronizationState::Synchronized);
    assert(justInsideLimit.Uncertainty().Nanoseconds() == 999999U);

    holdoverTimebase.Advance(1000ULL);
    const auto exactLimit = holdoverClock.Now();
    assert(exactLimit.Uncertainty().Nanoseconds() == 1000000U);
    assert(exactLimit.State() == SynchronizationState::Holdover);

    holdoverTimebase.Set(5000000000001ULL);
    const auto lost = holdoverClock.Now();
    assert(lost.State() == SynchronizationState::LostSynchronization);
    assert(lost.Uncertainty().IsSaturated());
    assert(lost.State() != SynchronizationState::NeverSynchronized);

    // Implausible frequency estimates are rejected rather than silently clamped.
    TestTimebase rateTimebase;
    MonotonicClock rateMonotonic(rateTimebase);
    SynchronizedClock rateClock(rateMonotonic);
    rateTimebase.Set(1000000000ULL);
    assert(
        rateClock.Observe(
            Observation(
                1000000000ULL,
                1000000000ULL,
                0U
            )
        ) == SynchronizationObservationStatus::Accepted
    );

    rateTimebase.Set(2000000000ULL);
    assert(
        rateClock.Observe(
            Observation(
                2000000000ULL,
                2002000000ULL,
                0U
            )
        ) == SynchronizationObservationStatus::RejectedFrequencyCorrection
    );
    assert(rateClock.Now().State() == SynchronizationState::Reacquiring);

    // A valid +100 ppm observation pair is accepted and becomes the long-term rate correction.
    TestTimebase correctedTimebase;
    MonotonicClock correctedMonotonic(correctedTimebase);
    SynchronizedClock correctedClock(correctedMonotonic);
    correctedTimebase.Set(1000000000ULL);
    assert(
        correctedClock.Observe(
            Observation(
                1000000000ULL,
                10000000000ULL,
                100U
            )
        ) == SynchronizationObservationStatus::Accepted
    );
    correctedTimebase.Set(2000000000ULL);
    assert(
        correctedClock.Observe(
            Observation(
                2000000000ULL,
                11000100000ULL,
                100U
            )
        ) == SynchronizationObservationStatus::Accepted
    );
    const auto correctedAnchor = correctedClock.Now();
    correctedTimebase.Advance(1000000000ULL);
    const auto correctedAfterSecond = correctedClock.Now();
    assert(
        ESPressio::Clock::Delta(
            correctedAnchor.Timestamp(),
            correctedAfterSecond.Timestamp()
        ).Nanoseconds() == 1000200000
    );

    correctedTimebase.Advance(1000000000ULL);
    const auto correctedSteadyState = correctedClock.Now();
    assert(
        ESPressio::Clock::Delta(
            correctedAfterSecond.Timestamp(),
            correctedSteadyState.Timestamp()
        ).Nanoseconds() == 1000100000
    );

    // Future and out-of-order observations are rejected without mutating the valid mapping.
    const auto beforeRejectedOrdering = correctedClock.Now();
    assert(
        correctedClock.Observe(
            Observation(
                correctedTimebase.CurrentCount() + 1U,
                correctedAfterSecond.Timestamp().Nanoseconds(),
                0U
            )
        ) == SynchronizationObservationStatus::RejectedFutureObservation
    );
    assert(
        correctedClock.Observe(
            Observation(
                1500000000ULL,
                10500000000ULL,
                0U
            )
        ) == SynchronizationObservationStatus::RejectedOutOfOrderObservation
    );
    assert(
        correctedClock.Now().Timestamp().Nanoseconds() ==
        beforeRejectedOrdering.Timestamp().Nanoseconds()
    );

    // Application-wide SynchronizedNow binding is shared across translation units.
    assert(ESPressio::Clock::BindSynchronizedClock(correctedClock));
    assert(!ESPressio::Clock::BindSynchronizedClock(correctedClock));
    assert(ESPressio::Clock::IsSynchronizedClockBound());
    const auto crossTranslationUnit = ReadSynchronizedFromAnotherTranslationUnit();
    assert(
        crossTranslationUnit.Timestamp().Nanoseconds() ==
        correctedClock.Now().Timestamp().Nanoseconds()
    );

    // Concurrent readers always observe coherent, non-regressing synchronized readings while the
    // single discipline writer publishes a long stream of exact observations.
    TestTimebase concurrentTimebase;
    MonotonicClock concurrentMonotonic(concurrentTimebase);
    SynchronizedClock concurrentClock(concurrentMonotonic);
    concurrentTimebase.Set(1U);
    assert(
        concurrentClock.Observe(
            Observation(
                1U,
                1000000001ULL,
                10U
            )
        ) == SynchronizationObservationStatus::Accepted
    );

    std::atomic<bool> stopReaders{false};
    std::atomic<bool> readerFailure{false};
    std::vector<std::thread> readers;

    for (std::uint32_t readerIndex = 0U; readerIndex < 8U; ++readerIndex) {
        readers.emplace_back([&]() {
            std::uint64_t previousTimestamp = 0U;

            while (!stopReaders.load(std::memory_order_acquire)) {
                const auto reading = concurrentClock.Now();
                const auto timestamp = reading.Timestamp().Nanoseconds();

                if (timestamp < previousTimestamp)
                    readerFailure.store(
                true,
                std::memory_order_release
            );

                if (reading.State() == SynchronizationState::NeverSynchronized)
                    readerFailure.store(
                true,
                std::memory_order_release
            );

                if (
                    (reading.State() == SynchronizationState::Synchronized) &&
                    (reading.Uncertainty().Nanoseconds() >= 1000000U)
                ) {
                    readerFailure.store(
                true,
                std::memory_order_release
            );
                }

                previousTimestamp = timestamp;
            }
        });
    }

    for (std::uint64_t publication = 1U; publication <= 10000U; ++publication) {
        concurrentTimebase.Advance(1000000U);
        const auto local = concurrentTimebase.CurrentCount();
        const auto reference = 1000000000ULL + local;

        if (
            concurrentClock.Observe(
                Observation(
                    local,
                    reference,
                    10U
                )
            ) != SynchronizationObservationStatus::Accepted
        ) {
            readerFailure.store(
                true,
                std::memory_order_release
            );
        }
    }

    stopReaders.store(
        true,
        std::memory_order_release
    );

    for (auto& reader : readers)
        reader.join();

    assert(!readerFailure.load(std::memory_order_acquire));

    return 0;
}
