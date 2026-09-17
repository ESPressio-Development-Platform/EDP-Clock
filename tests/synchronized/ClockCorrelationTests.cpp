#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>

#include <ESPressio_Clock.hpp>

namespace {

    /// Creates an available correlation through the internal provider-facing factory.
    constexpr ESPressio::Clock::ClockCorrelation Correlation(
        std::uint64_t monotonicAnchorNanoseconds,
        std::uint64_t synchronizedAnchorNanoseconds,
        std::int32_t frequencyCorrectionPartsPerBillion,
        std::uint32_t frequencyUncertaintyPartsPerBillion,
        std::uint64_t anchorUncertaintyNanoseconds
    ) noexcept {
        return ESPressio::Clock::Detail::ClockCorrelationFactory::Create(
            ESPressio::Clock::MonotonicTimestamp::FromNanoseconds(
                monotonicAnchorNanoseconds
            ),
            ESPressio::Clock::SynchronizedTimestamp::FromNanoseconds(
                synchronizedAnchorNanoseconds
            ),
            frequencyCorrectionPartsPerBillion,
            frequencyUncertaintyPartsPerBillion,
            ESPressio::Clock::SynchronizationUncertainty::FromNanoseconds(
                anchorUncertaintyNanoseconds
            )
        );
    }

} // anonymous namespace


int main() {
    using ESPressio::Clock::ClockCorrelationProjectionStatus;
    using ESPressio::Clock::ClockCorrelationState;

    static_assert(sizeof(ESPressio::Clock::ClockCorrelation) == 32U);
    static_assert(sizeof(ESPressio::Clock::ClockCorrelationProjection) == 16U);
    static_assert(std::is_trivially_copyable_v<ESPressio::Clock::ClockCorrelation>);
    static_assert(std::is_trivially_copyable_v<ESPressio::Clock::ClockCorrelationProjection>);

    // A default correlation is explicitly unavailable and cannot manufacture a timestamp.
    const ESPressio::Clock::ClockCorrelation unavailable;
    assert(unavailable.State() == ClockCorrelationState::Unavailable);
    assert(!unavailable.IsAvailable());
    const auto unavailableProjection = unavailable.Correlate(
        ESPressio::Clock::MonotonicTimestamp::FromNanoseconds(123U),
        ESPressio::Clock::SynchronizationUncertainty::Zero()
    );
    assert(!unavailableProjection.IsCorrelated());
    assert(
        unavailableProjection.Status() ==
        ClockCorrelationProjectionStatus::CorrelationUnavailable
    );
    assert(unavailableProjection.Uncertainty().IsSaturated());

    // Positive rate correction applies equally to forward and backward reference projection.
    const auto positive = Correlation(
        1000000000ULL,
        10000000000ULL,
        100000,
        500U,
        100U
    );
    assert(positive.State() == ClockCorrelationState::Available);
    assert(positive.IsAvailable());
    assert(positive.MonotonicAnchor().Nanoseconds() == 1000000000ULL);
    assert(positive.SynchronizedAnchor().Nanoseconds() == 10000000000ULL);
    assert(positive.FrequencyCorrectionPartsPerBillion() == 100000);
    assert(positive.FrequencyUncertaintyPartsPerBillion() == 500U);
    assert(positive.AnchorUncertainty().Nanoseconds() == 100U);

    const auto forward = positive.Correlate(
        ESPressio::Clock::MonotonicTimestamp::FromNanoseconds(
            3000000000ULL
        ),
        ESPressio::Clock::SynchronizationUncertainty::FromNanoseconds(
            50U
        )
    );
    assert(forward.IsCorrelated());
    assert(forward.Status() == ClockCorrelationProjectionStatus::Correlated);
    assert(forward.Timestamp().Nanoseconds() == 12000200000ULL);
    assert(forward.Uncertainty().Nanoseconds() == 1150U);

    const auto backward = positive.Correlate(
        ESPressio::Clock::MonotonicTimestamp::FromNanoseconds(
            500000000ULL
        ),
        ESPressio::Clock::SynchronizationUncertainty::FromNanoseconds(
            50U
        )
    );
    assert(backward.IsCorrelated());
    assert(backward.Timestamp().Nanoseconds() == 9499950000ULL);
    assert(backward.Uncertainty().Nanoseconds() == 400U);

    // Negative rate correction reduces synchronized elapsed time in both projection directions.
    const auto negative = Correlation(
        1000000000ULL,
        10000000000ULL,
        -100000,
        0U,
        10U
    );
    const auto negativeForward = negative.Correlate(
        ESPressio::Clock::MonotonicTimestamp::FromNanoseconds(
            3000000000ULL
        ),
        ESPressio::Clock::SynchronizationUncertainty::Zero()
    );
    assert(negativeForward.IsCorrelated());
    assert(negativeForward.Timestamp().Nanoseconds() == 11999800000ULL);

    // Coordinate range failures are explicit rather than silently clamped.
    const auto nearOrigin = Correlation(
        1000U,
        100U,
        0,
        0U,
        0U
    );
    const auto underflow = nearOrigin.Correlate(
        ESPressio::Clock::MonotonicTimestamp::FromNanoseconds(0U),
        ESPressio::Clock::SynchronizationUncertainty::Zero()
    );
    assert(!underflow.IsCorrelated());
    assert(
        underflow.Status() ==
        ClockCorrelationProjectionStatus::SynchronizedCoordinateUnderflow
    );
    assert(underflow.Uncertainty().IsSaturated());

    constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();
    const auto nearMaximum = Correlation(
        0U,
        maximum - 100U,
        0,
        0U,
        0U
    );
    const auto overflow = nearMaximum.Correlate(
        ESPressio::Clock::MonotonicTimestamp::FromNanoseconds(1000U),
        ESPressio::Clock::SynchronizationUncertainty::Zero()
    );
    assert(!overflow.IsCorrelated());
    assert(
        overflow.Status() ==
        ClockCorrelationProjectionStatus::SynchronizedCoordinateOverflow
    );
    assert(overflow.Uncertainty().IsSaturated());

    // Quality degradation saturates rather than wrapping while preserving a valid coordinate.
    const auto uncertain = Correlation(
        0U,
        10000000000ULL,
        0,
        std::numeric_limits<std::uint32_t>::max(),
        100U
    );
    const auto saturated = uncertain.Correlate(
        ESPressio::Clock::MonotonicTimestamp::FromNanoseconds(
            2000000000ULL
        ),
        ESPressio::Clock::SynchronizationUncertainty::FromNanoseconds(
            50U
        )
    );
    assert(saturated.IsCorrelated());
    assert(saturated.Timestamp().Nanoseconds() == 12000000000ULL);
    assert(saturated.Uncertainty().IsSaturated());

    return 0;
}
