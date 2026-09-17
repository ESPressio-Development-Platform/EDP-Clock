#pragma once

#include <type_traits>

#include "MonotonicTimestamp.hpp"
#include "SynchronizationUncertainty.hpp"
#include "SynchronizedTimestamp.hpp"

namespace ESPressio::Clock {

    /// Source-agnostic four-timestamp exchange used to estimate synchronized time at a local monotonic coordinate.
    ///
    /// T1 and T4 belong to the requesting system's local MonotonicClock. T2 and T3 belong to the
    /// responding reference system's SynchronizedClock. Transport code owns timestamp capture and
    /// carriage; EDP-Clock owns the generic reduction mathematics represented by this value.
    class FourTimestampExchange final {
    private:

        // Exchange timestamps.

        /// T1: local monotonic coordinate captured when the request leaves the local system.
        MonotonicTimestamp _localTransmitTimestamp;

        /// T2: reference synchronized coordinate captured when the request reaches the remote system.
        SynchronizedTimestamp _remoteReceiveTimestamp;

        /// T3: reference synchronized coordinate captured when the response leaves the remote system.
        SynchronizedTimestamp _remoteTransmitTimestamp;

        /// T4: local monotonic coordinate captured when the response reaches the local system.
        MonotonicTimestamp _localReceiveTimestamp;

        // Timestamp uncertainty.

        /// Conservative uncertainty associated with the T1 capture.
        SynchronizationUncertainty _localTransmitUncertainty;

        /// Conservative uncertainty associated with the T2 synchronized timestamp and capture.
        SynchronizationUncertainty _remoteReceiveUncertainty;

        /// Conservative uncertainty associated with the T3 synchronized timestamp and capture.
        SynchronizationUncertainty _remoteTransmitUncertainty;

        /// Conservative uncertainty associated with the T4 capture.
        SynchronizationUncertainty _localReceiveUncertainty;

    public:

        // Construction.

        /// Creates one complete four-timestamp exchange.
        ///
        /// @param localTransmitTimestamp T1 local monotonic request-transmit coordinate.
        /// @param localTransmitUncertainty Conservative T1 capture uncertainty.
        /// @param remoteReceiveTimestamp T2 remote/reference synchronized receive coordinate.
        /// @param remoteReceiveUncertainty Conservative T2 reference/capture uncertainty.
        /// @param remoteTransmitTimestamp T3 remote/reference synchronized transmit coordinate.
        /// @param remoteTransmitUncertainty Conservative T3 reference/capture uncertainty.
        /// @param localReceiveTimestamp T4 local monotonic response-receive coordinate.
        /// @param localReceiveUncertainty Conservative T4 capture uncertainty.
        constexpr FourTimestampExchange(
            const MonotonicTimestamp& localTransmitTimestamp,
            const SynchronizationUncertainty& localTransmitUncertainty,
            const SynchronizedTimestamp& remoteReceiveTimestamp,
            const SynchronizationUncertainty& remoteReceiveUncertainty,
            const SynchronizedTimestamp& remoteTransmitTimestamp,
            const SynchronizationUncertainty& remoteTransmitUncertainty,
            const MonotonicTimestamp& localReceiveTimestamp,
            const SynchronizationUncertainty& localReceiveUncertainty
        ) noexcept :
            _localTransmitTimestamp(localTransmitTimestamp),
            _remoteReceiveTimestamp(remoteReceiveTimestamp),
            _remoteTransmitTimestamp(remoteTransmitTimestamp),
            _localReceiveTimestamp(localReceiveTimestamp),
            _localTransmitUncertainty(localTransmitUncertainty),
            _remoteReceiveUncertainty(remoteReceiveUncertainty),
            _remoteTransmitUncertainty(remoteTransmitUncertainty),
            _localReceiveUncertainty(localReceiveUncertainty) {}


        // Timestamp access.

        /// Returns T1, the local monotonic request-transmit coordinate.
        constexpr const MonotonicTimestamp& LocalTransmitTimestamp() const noexcept {
            return _localTransmitTimestamp;
        }

        /// Returns T2, the remote/reference synchronized receive coordinate.
        constexpr const SynchronizedTimestamp& RemoteReceiveTimestamp() const noexcept {
            return _remoteReceiveTimestamp;
        }

        /// Returns T3, the remote/reference synchronized transmit coordinate.
        constexpr const SynchronizedTimestamp& RemoteTransmitTimestamp() const noexcept {
            return _remoteTransmitTimestamp;
        }

        /// Returns T4, the local monotonic response-receive coordinate.
        constexpr const MonotonicTimestamp& LocalReceiveTimestamp() const noexcept {
            return _localReceiveTimestamp;
        }


        // Uncertainty access.

        /// Returns the conservative T1 capture uncertainty.
        constexpr const SynchronizationUncertainty& LocalTransmitUncertainty() const noexcept {
            return _localTransmitUncertainty;
        }

        /// Returns the conservative T2 reference/capture uncertainty.
        constexpr const SynchronizationUncertainty& RemoteReceiveUncertainty() const noexcept {
            return _remoteReceiveUncertainty;
        }

        /// Returns the conservative T3 reference/capture uncertainty.
        constexpr const SynchronizationUncertainty& RemoteTransmitUncertainty() const noexcept {
            return _remoteTransmitUncertainty;
        }

        /// Returns the conservative T4 capture uncertainty.
        constexpr const SynchronizationUncertainty& LocalReceiveUncertainty() const noexcept {
            return _localReceiveUncertainty;
        }

    };


    static_assert(
        sizeof(FourTimestampExchange) == 48U,
        "FourTimestampExchange must remain a compact 48-byte value"
    );

    static_assert(
        std::is_trivially_copyable_v<FourTimestampExchange>,
        "FourTimestampExchange must remain trivially copyable"
    );

} // ESPressio::Clock
