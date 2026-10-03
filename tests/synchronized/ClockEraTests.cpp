#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>

#include <ESPressio_Clock.hpp>

namespace {

    /// Creates one available reference correlation for era-qualified value tests.
    constexpr ESPressio::Clock::ClockCorrelation Correlation() noexcept {
        return ESPressio::Clock::Detail::ClockCorrelationFactory::Create(
            ESPressio::Clock::MonotonicTimestamp::FromNanoseconds(1000U),
            ESPressio::Clock::SynchronizedTimestamp::FromNanoseconds(10000U),
            0,
            0U,
            ESPressio::Clock::SynchronizationUncertainty::FromNanoseconds(10U)
        );
    }

} // anonymous namespace


int main() {
    using ESPressio::Clock::ClockCorrelationProjectionStatus;
    using ESPressio::Clock::ClockEra;
    using ESPressio::Clock::EraQualifiedClockAssessment;
    using ESPressio::Clock::EraQualifiedClockCorrelation;
    using ESPressio::Clock::EraQualifiedClockCorrelationProjection;
    using ESPressio::Clock::MonotonicCaptureBounds;
    using ESPressio::Clock::MonotonicTimestamp;
    using ESPressio::Clock::TemporalQualityAssessment;

    // Mesh-facing Clock values retain exact, compile-time-accountable representations.
    static_assert(sizeof(ClockEra) == 8U);
    static_assert(sizeof(MonotonicCaptureBounds) == 16U);
    static_assert(sizeof(EraQualifiedClockCorrelation) == 40U);
    static_assert(sizeof(EraQualifiedClockCorrelationProjection) == 24U);
    static_assert(sizeof(EraQualifiedClockAssessment) == 24U);
    static_assert(std::is_trivially_copyable_v<ClockEra>);
    static_assert(std::is_trivially_copyable_v<MonotonicCaptureBounds>);
    static_assert(std::is_trivially_copyable_v<EraQualifiedClockCorrelation>);
    static_assert(std::is_trivially_copyable_v<EraQualifiedClockCorrelationProjection>);
    static_assert(std::is_trivially_copyable_v<EraQualifiedClockAssessment>);

    // ClockEra is opaque, zero-invalid and totally ordered without interpreting provenance.
    constexpr ClockEra unavailableEra;
    constexpr auto firstEra = ClockEra::FromValue(1U);
    constexpr auto laterEra = ClockEra::FromValue(42U);
    static_assert(!unavailableEra.IsValid());
    static_assert(firstEra.IsValid());
    static_assert(firstEra.Value() == 1U);
    static_assert(firstEra == ClockEra::FromValue(1U));
    static_assert(firstEra != laterEra);
    static_assert(firstEra < laterEra);
    static_assert(laterEra > firstEra);

    // Unavailable and reversed native-to-monotonic conversions fail closed.
    constexpr MonotonicCaptureBounds unavailableBounds;
    static_assert(!unavailableBounds.IsAvailable());
    static_assert(unavailableBounds.Midpoint().Nanoseconds() == 0U);
    static_assert(unavailableBounds.Uncertainty().IsSaturated());

    constexpr auto reversed = MonotonicCaptureBounds::Between(
        MonotonicTimestamp::FromNanoseconds(11U),
        MonotonicTimestamp::FromNanoseconds(10U)
    );
    static_assert(!reversed.IsAvailable());

    // Midpoint conversion rounds odd half-width uncertainty upward and does not overflow.
    constexpr auto exact = MonotonicCaptureBounds::Exact(
        MonotonicTimestamp::FromNanoseconds(500U)
    );
    static_assert(exact.IsAvailable());
    static_assert(exact.Midpoint().Nanoseconds() == 500U);
    static_assert(exact.Uncertainty().Nanoseconds() == 0U);

    constexpr auto oddWidth = MonotonicCaptureBounds::Between(
        MonotonicTimestamp::FromNanoseconds(100U),
        MonotonicTimestamp::FromNanoseconds(105U)
    );
    static_assert(oddWidth.Midpoint().Nanoseconds() == 102U);
    static_assert(oddWidth.Uncertainty().Nanoseconds() == 3U);

    constexpr auto fullRange = MonotonicCaptureBounds::Between(
        MonotonicTimestamp::FromNanoseconds(0U),
        MonotonicTimestamp::FromNanoseconds(std::numeric_limits<std::uint64_t>::max())
    );
    static_assert(fullRange.IsAvailable());
    static_assert(
        fullRange.Midpoint().Nanoseconds() ==
        (std::numeric_limits<std::uint64_t>::max() / 2U)
    );
    static_assert(fullRange.Uncertainty().IsSaturated());

    // Era qualification is retained through immutable correlation and policy assessment values.
    constexpr EraQualifiedClockCorrelation qualified(
        laterEra,
        Correlation()
    );
    static_assert(qualified.IsAvailable());
    static_assert(qualified.Era() == laterEra);

    constexpr auto projection = qualified.Correlate(
        MonotonicTimestamp::FromNanoseconds(1500U),
        ESPressio::Clock::SynchronizationUncertainty::FromNanoseconds(5U)
    );
    static_assert(projection.IsCorrelated());
    static_assert(projection.Era() == laterEra);
    static_assert(projection.Projection().Timestamp().Nanoseconds() == 10500U);
    static_assert(projection.Projection().Uncertainty().Nanoseconds() == 15U);

    constexpr EraQualifiedClockAssessment reliable(
        projection,
        TemporalQualityAssessment::Reliable
    );
    static_assert(reliable.IsCorrelated());
    static_assert(reliable.Era() == laterEra);
    static_assert(reliable.Timestamp().Nanoseconds() == 10500U);
    static_assert(reliable.Uncertainty().Nanoseconds() == 15U);
    static_assert(reliable.ProjectionStatus() == ClockCorrelationProjectionStatus::Correlated);
    static_assert(reliable.Assessment() == TemporalQualityAssessment::Reliable);

    // An invalid Era can never manufacture usable temporal provenance from a valid correlation.
    constexpr EraQualifiedClockCorrelation invalidQualified(
        ClockEra(),
        Correlation()
    );
    static_assert(!invalidQualified.IsAvailable());
    constexpr auto invalidProjection = invalidQualified.Correlate(
        MonotonicTimestamp::FromNanoseconds(1500U),
        ESPressio::Clock::SynchronizationUncertainty::Zero()
    );
    static_assert(!invalidProjection.IsCorrelated());

    return 0;
}

