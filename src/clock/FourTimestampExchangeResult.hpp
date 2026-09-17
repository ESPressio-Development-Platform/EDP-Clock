#pragma once

#include <cstdint>
#include <type_traits>

#include "FourTimestampExchange.hpp"
#include "FourTimestampExchangeStatus.hpp"
#include "SynchronizationObservation.hpp"

namespace ESPressio::Clock {

    class FourTimestampExchangeResult;


    /// Reduces a four-timestamp exchange into one source-agnostic synchronization observation.
    constexpr FourTimestampExchangeResult EstimateFourTimestampExchange(
        const FourTimestampExchange& exchange
    ) noexcept;


    /// Result of reducing one four-timestamp time-transfer exchange into a synchronization observation.
    class FourTimestampExchangeResult final {
    private:

        // Reduction result.

        /// Observation produced for Clock discipline when the reduction succeeds.
        SynchronizationObservation _observation;

        /// Estimated aggregate transport path delay excluding remote turnaround time.
        std::uint64_t _roundTripPathDelayNanoseconds;

        /// Admission status produced by the four-timestamp reduction.
        FourTimestampExchangeStatus _status;

        // Construction.

        /// Creates one reduction result from fully prepared values.
        constexpr FourTimestampExchangeResult(
            const SynchronizationObservation& observation,
            std::uint64_t roundTripPathDelayNanoseconds,
            FourTimestampExchangeStatus status
        ) noexcept :
            _observation(observation),
            _roundTripPathDelayNanoseconds(roundTripPathDelayNanoseconds),
            _status(status) {}

        /// Allows only the Clock-owned estimator to construct reduction results.
        friend constexpr FourTimestampExchangeResult EstimateFourTimestampExchange(
            const FourTimestampExchange& exchange
        ) noexcept;

    public:

        // Result inspection.

        /// Indicates whether the exchange was reduced successfully.
        constexpr bool IsAccepted() const noexcept {
            return _status == FourTimestampExchangeStatus::Accepted;
        }

        /// Returns the reduction status.
        constexpr FourTimestampExchangeStatus Status() const noexcept {
            return _status;
        }

        /// Returns the derived synchronization observation.
        ///
        /// Consumers must inspect IsAccepted() or Status() before submitting this value to Clock
        /// discipline. Rejected results expose a saturated-uncertainty sentinel observation only so
        /// the result remains a compact allocation-free value type without optional storage.
        constexpr const SynchronizationObservation& Observation() const noexcept {
            return _observation;
        }

        /// Returns the estimated aggregate path round-trip delay excluding remote turnaround time.
        constexpr std::uint64_t RoundTripPathDelayNanoseconds() const noexcept {
            return _roundTripPathDelayNanoseconds;
        }

    };


    static_assert(
        sizeof(FourTimestampExchangeResult) == 40U,
        "FourTimestampExchangeResult must remain a compact 40-byte value"
    );

    static_assert(
        std::is_trivially_copyable_v<FourTimestampExchangeResult>,
        "FourTimestampExchangeResult must remain trivially copyable"
    );

} // ESPressio::Clock
