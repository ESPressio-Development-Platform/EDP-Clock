#pragma once

#include <cstdint>
#include <limits>

#include "FourTimestampExchange.hpp"
#include "FourTimestampExchangeResult.hpp"

namespace ESPressio::Clock {

    namespace Detail {

        /// Adds one nanosecond uncertainty contribution while saturating at the public uncertainty boundary.
        constexpr std::uint64_t AddExchangeUncertainty(
            std::uint64_t current,
            std::uint64_t contribution
        ) noexcept {
            constexpr auto maximum = static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max());

            if (current >= maximum) return maximum;
            if (contribution >= maximum) return maximum;
            if (contribution > (maximum - current)) return maximum;

            return current + contribution;
        }

    } // ESPressio::Clock::Detail


    /// Reduces a four-timestamp exchange into one source-agnostic synchronization observation.
    ///
    /// T1/T4 are local monotonic coordinates and T2/T3 are remote/reference synchronized
    /// coordinates. The estimator therefore uses only same-domain subtraction:
    ///
    /// `localElapsed = T4 - T1`
    /// `remoteTurnaround = T3 - T2`
    /// `pathRoundTrip = localElapsed - remoteTurnaround`
    /// `referenceAtT4 = T3 + floor(pathRoundTrip / 2)`
    ///
    /// The midpoint path estimate does not assume that the actual forward and reverse paths are
    /// perfectly symmetric. Half of the measured path round-trip is retained as a conservative
    /// path-asymmetry uncertainty bound. All four supplied timestamp uncertainty bounds are added
    /// conservatively. The resulting observation is anchored at local T4 and can therefore be used
    /// for first-ever synchronization while the local SynchronizedClock is still NeverSynchronized.
    ///
    /// @param exchange Complete T1/T2/T3/T4 exchange and timestamp uncertainty bounds.
    /// @return Accepted observation estimate or an explicit rejection status.
    constexpr FourTimestampExchangeResult EstimateFourTimestampExchange(
        const FourTimestampExchange& exchange
    ) noexcept {
        const auto localTransmit = exchange.LocalTransmitTimestamp().Nanoseconds();
        const auto localReceive = exchange.LocalReceiveTimestamp().Nanoseconds();

        if (localReceive < localTransmit)
            return FourTimestampExchangeResult(
                SynchronizationObservation(
                    MonotonicTimestamp::FromNanoseconds(0U),
                    SynchronizedTimestamp::FromNanoseconds(0U),
                    SynchronizationUncertainty::Maximum()
                ),
                0U,
                FourTimestampExchangeStatus::RejectedLocalTimestampOrder
            );

        const auto remoteReceive = exchange.RemoteReceiveTimestamp().Nanoseconds();
        const auto remoteTransmit = exchange.RemoteTransmitTimestamp().Nanoseconds();

        if (remoteTransmit < remoteReceive)
            return FourTimestampExchangeResult(
                SynchronizationObservation(
                    MonotonicTimestamp::FromNanoseconds(0U),
                    SynchronizedTimestamp::FromNanoseconds(0U),
                    SynchronizationUncertainty::Maximum()
                ),
                0U,
                FourTimestampExchangeStatus::RejectedRemoteTimestampOrder
            );

        const auto localElapsed = localReceive - localTransmit;
        const auto remoteTurnaround = remoteTransmit - remoteReceive;

        if (remoteTurnaround > localElapsed)
            return FourTimestampExchangeResult(
                SynchronizationObservation(
                    MonotonicTimestamp::FromNanoseconds(0U),
                    SynchronizedTimestamp::FromNanoseconds(0U),
                    SynchronizationUncertainty::Maximum()
                ),
                0U,
                FourTimestampExchangeStatus::RejectedImpossiblePathDelay
            );

        const auto roundTripPathDelay = localElapsed - remoteTurnaround;
        const auto estimatedReversePathDelay = roundTripPathDelay / 2U;

        if (estimatedReversePathDelay > (std::numeric_limits<std::uint64_t>::max() - remoteTransmit))
            return FourTimestampExchangeResult(
                SynchronizationObservation(
                    MonotonicTimestamp::FromNanoseconds(0U),
                    SynchronizedTimestamp::FromNanoseconds(0U),
                    SynchronizationUncertainty::Maximum()
                ),
                0U,
                FourTimestampExchangeStatus::RejectedReferenceOverflow
            );

        const auto referenceAtLocalReceive = remoteTransmit + estimatedReversePathDelay;
        const auto asymmetryUncertainty =
            (roundTripPathDelay / 2U) +
            ((roundTripPathDelay & 1U) == 0U ? 0U : 1U);

        auto totalUncertainty = asymmetryUncertainty;
        totalUncertainty = Detail::AddExchangeUncertainty(
            totalUncertainty,
            exchange.LocalTransmitUncertainty().Nanoseconds()
        );
        totalUncertainty = Detail::AddExchangeUncertainty(
            totalUncertainty,
            exchange.RemoteReceiveUncertainty().Nanoseconds()
        );
        totalUncertainty = Detail::AddExchangeUncertainty(
            totalUncertainty,
            exchange.RemoteTransmitUncertainty().Nanoseconds()
        );
        totalUncertainty = Detail::AddExchangeUncertainty(
            totalUncertainty,
            exchange.LocalReceiveUncertainty().Nanoseconds()
        );

        return FourTimestampExchangeResult(
            SynchronizationObservation(
                exchange.LocalReceiveTimestamp(),
                SynchronizedTimestamp::FromNanoseconds(
                    referenceAtLocalReceive
                ),
                SynchronizationUncertainty::FromNanoseconds(
                    totalUncertainty
                )
            ),
            roundTripPathDelay,
            FourTimestampExchangeStatus::Accepted
        );
    }

} // ESPressio::Clock
