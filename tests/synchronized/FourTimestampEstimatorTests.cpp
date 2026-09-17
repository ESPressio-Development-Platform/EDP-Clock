#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>

#include <ESPressio_Clock.hpp>

namespace {

    using ESPressio::Clock::EstimateFourTimestampExchange;
    using ESPressio::Clock::FourTimestampExchange;
    using ESPressio::Clock::FourTimestampExchangeStatus;
    using ESPressio::Clock::MonotonicTimestamp;
    using ESPressio::Clock::SynchronizationUncertainty;
    using ESPressio::Clock::SynchronizedTimestamp;

    /// Creates one deterministic exchange using explicit nanosecond coordinates.
    FourTimestampExchange Exchange(
        std::uint64_t t1,
        std::uint64_t t2,
        std::uint64_t t3,
        std::uint64_t t4,
        std::uint64_t u1 = 0U,
        std::uint64_t u2 = 0U,
        std::uint64_t u3 = 0U,
        std::uint64_t u4 = 0U
    ) noexcept {
        return FourTimestampExchange(
            MonotonicTimestamp::FromNanoseconds(
                t1
            ),
            SynchronizationUncertainty::FromNanoseconds(
                u1
            ),
            SynchronizedTimestamp::FromNanoseconds(
                t2
            ),
            SynchronizationUncertainty::FromNanoseconds(
                u2
            ),
            SynchronizedTimestamp::FromNanoseconds(
                t3
            ),
            SynchronizationUncertainty::FromNanoseconds(
                u3
            ),
            MonotonicTimestamp::FromNanoseconds(
                t4
            ),
            SynchronizationUncertainty::FromNanoseconds(
                u4
            )
        );
    }

} // anonymous namespace


int main() {
    static_assert(
        sizeof(FourTimestampExchange) == 48U,
        "FourTimestampExchange memory cost changed unexpectedly"
    );

    static_assert(
        std::is_trivially_copyable_v<FourTimestampExchange>,
        "FourTimestampExchange must remain trivially copyable"
    );

    // A symmetric 400 ns + 400 ns path with 200 ns remote turnaround yields an exact T4 reference.
    const auto symmetric = EstimateFourTimestampExchange(
        Exchange(
            1000000U,
            42000400U,
            42000600U,
            1001000U,
            10U,
            20U,
            30U,
            40U
        )
    );

    assert(symmetric.IsAccepted());
    assert(symmetric.Status() == FourTimestampExchangeStatus::Accepted);
    assert(symmetric.RoundTripPathDelayNanoseconds() == 800U);
    assert(symmetric.Observation().LocalTimestamp().Nanoseconds() == 1001000U);
    assert(symmetric.Observation().ReferenceTimestamp().Nanoseconds() == 42001000U);
    assert(symmetric.Observation().Uncertainty().Nanoseconds() == 500U);

    // With no knowledge of direction asymmetry, half of path round-trip remains an uncertainty bound.
    const auto asymmetric = EstimateFourTimestampExchange(
        Exchange(
            1000000U,
            42000200U,
            42000400U,
            1001000U
        )
    );

    assert(asymmetric.IsAccepted());
    assert(asymmetric.Observation().ReferenceTimestamp().Nanoseconds() == 42000800U);
    assert(asymmetric.Observation().Uncertainty().Nanoseconds() == 400U);

    // Odd-nanosecond path delays retain floor midpoint while uncertainty rounds upward.
    const auto oddPath = EstimateFourTimestampExchange(
        Exchange(
            100U,
            1000U,
            1000U,
            105U
        )
    );

    assert(oddPath.IsAccepted());
    assert(oddPath.RoundTripPathDelayNanoseconds() == 5U);
    assert(oddPath.Observation().ReferenceTimestamp().Nanoseconds() == 1002U);
    assert(oddPath.Observation().Uncertainty().Nanoseconds() == 3U);

    const auto localOrder = EstimateFourTimestampExchange(
        Exchange(
            101U,
            1000U,
            1000U,
            100U
        )
    );

    assert(!localOrder.IsAccepted());
    assert(localOrder.Status() == FourTimestampExchangeStatus::RejectedLocalTimestampOrder);
    assert(localOrder.Observation().Uncertainty().IsSaturated());

    const auto remoteOrder = EstimateFourTimestampExchange(
        Exchange(
            100U,
            1001U,
            1000U,
            110U
        )
    );

    assert(!remoteOrder.IsAccepted());
    assert(remoteOrder.Status() == FourTimestampExchangeStatus::RejectedRemoteTimestampOrder);

    const auto impossibleDelay = EstimateFourTimestampExchange(
        Exchange(
            100U,
            1000U,
            1020U,
            110U
        )
    );

    assert(!impossibleDelay.IsAccepted());
    assert(impossibleDelay.Status() == FourTimestampExchangeStatus::RejectedImpossiblePathDelay);

    const auto referenceOverflow = EstimateFourTimestampExchange(
        Exchange(
            0U,
            std::numeric_limits<std::uint64_t>::max() - 10U,
            std::numeric_limits<std::uint64_t>::max() - 5U,
            25U
        )
    );

    assert(!referenceOverflow.IsAccepted());
    assert(referenceOverflow.Status() == FourTimestampExchangeStatus::RejectedReferenceOverflow);

    // Excessive transport/capture uncertainty saturates rather than wrapping.
    const auto saturatedUncertainty = EstimateFourTimestampExchange(
        Exchange(
            0U,
            1000U,
            1000U,
            100U,
            std::numeric_limits<std::uint32_t>::max(),
            1U,
            1U,
            1U
        )
    );

    assert(saturatedUncertainty.IsAccepted());
    assert(saturatedUncertainty.Observation().Uncertainty().IsSaturated());

    return 0;
}
