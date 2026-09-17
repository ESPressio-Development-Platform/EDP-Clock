#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>

#include "ClockCorrelationProjection.hpp"
#include "ClockCorrelationState.hpp"
#include "MonotonicTimestamp.hpp"
#include "SynchronizationUncertainty.hpp"
#include "SynchronizedTimestamp.hpp"

namespace ESPressio::Clock {

    namespace Detail {

        /// Internal construction surface used by synchronized-clock providers to create correlations.
        struct ClockCorrelationFactory;


        /// Returns the unsigned magnitude of a signed 32-bit rate correction.
        constexpr std::uint32_t CorrelationRateMagnitude(
            std::int32_t partsPerBillion
        ) noexcept {
            return partsPerBillion < 0
                ? static_cast<std::uint32_t>(-static_cast<std::int64_t>(partsPerBillion))
                : static_cast<std::uint32_t>(partsPerBillion);
        }

        /// Scales elapsed nanoseconds by a sub-billion parts-per-billion magnitude.
        constexpr std::uint64_t ScaleCorrelationElapsed(
            std::uint64_t elapsedNanoseconds,
            std::uint32_t partsPerBillion
        ) noexcept {
            constexpr std::uint64_t billion = 1000000000ULL;
            const auto wholeBillions = elapsedNanoseconds / billion;
            const auto remainder = elapsedNanoseconds % billion;

            return
                (wholeBillions * static_cast<std::uint64_t>(partsPerBillion)) +
                ((remainder * static_cast<std::uint64_t>(partsPerBillion)) / billion);
        }

        /// Scales elapsed nanoseconds by rate uncertainty and saturates at the uncertainty boundary.
        constexpr std::uint32_t ScaleCorrelationUncertainty(
            std::uint64_t elapsedNanoseconds,
            std::uint32_t uncertaintyPartsPerBillion
        ) noexcept {
            constexpr std::uint64_t billion = 1000000000ULL;
            constexpr auto maximum = std::numeric_limits<std::uint32_t>::max();

            if ((elapsedNanoseconds == 0U) || (uncertaintyPartsPerBillion == 0U)) return 0U;

            const auto wholeBillions = elapsedNanoseconds / billion;
            const auto remainder = elapsedNanoseconds % billion;

            if (wholeBillions > (static_cast<std::uint64_t>(maximum) / uncertaintyPartsPerBillion))
                return maximum;

            const auto wholeContribution =
                wholeBillions * static_cast<std::uint64_t>(uncertaintyPartsPerBillion);
            const auto remainderContribution =
                (remainder * static_cast<std::uint64_t>(uncertaintyPartsPerBillion)) / billion;

            if (remainderContribution > (static_cast<std::uint64_t>(maximum) - wholeContribution))
                return maximum;

            return static_cast<std::uint32_t>(wholeContribution + remainderContribution);
        }

        /// Adds one uncertainty contribution and saturates at the representable boundary.
        constexpr std::uint32_t AddCorrelationUncertainty(
            std::uint32_t current,
            std::uint32_t contribution
        ) noexcept {
            constexpr auto maximum = std::numeric_limits<std::uint32_t>::max();

            if (contribution > (maximum - current)) return maximum;

            return static_cast<std::uint32_t>(current + contribution);
        }

    } // ESPressio::Clock::Detail


    /// Immutable snapshot relating local monotonic time to the best accepted synchronized reference.
    ///
    /// A correlation represents the reference mapping known when the snapshot was captured. It is
    /// intentionally independent from the non-regressing published SynchronizedClock timeline and
    /// therefore does not include temporary phase slew. Consumers can retain this compact value and
    /// project many delayed hardware/event timestamps without retaining Clock history or repeatedly
    /// reading the concurrent synchronized-clock discipline state.
    class ClockCorrelation final {
    private:

        // Correlation anchors.

        /// Local monotonic coordinate at which the reference mapping is anchored.
        MonotonicTimestamp _monotonicAnchor;

        /// Best accepted synchronized reference coordinate at the monotonic anchor.
        SynchronizedTimestamp _synchronizedAnchor;

        // Rate and quality metadata.

        /// Long-term signed frequency correction applied when projecting away from the anchor.
        std::int32_t _frequencyCorrectionPartsPerBillion{0};

        /// Conservative uncertainty of the frequency estimate in parts per billion.
        std::uint32_t _frequencyUncertaintyPartsPerBillion{0U};

        /// Conservative synchronization uncertainty at the correlation anchor.
        SynchronizationUncertainty _anchorUncertainty;

        // Availability metadata.

        /// Indicates whether this snapshot contains an accepted reference mapping.
        ClockCorrelationState _state{ClockCorrelationState::Unavailable};

        // Construction.

        /// Creates one available immutable reference mapping.
        constexpr ClockCorrelation(
            const MonotonicTimestamp& monotonicAnchor,
            const SynchronizedTimestamp& synchronizedAnchor,
            std::int32_t frequencyCorrectionPartsPerBillion,
            std::uint32_t frequencyUncertaintyPartsPerBillion,
            const SynchronizationUncertainty& anchorUncertainty
        ) noexcept :
            _monotonicAnchor(monotonicAnchor),
            _synchronizedAnchor(synchronizedAnchor),
            _frequencyCorrectionPartsPerBillion(frequencyCorrectionPartsPerBillion),
            _frequencyUncertaintyPartsPerBillion(frequencyUncertaintyPartsPerBillion),
            _anchorUncertainty(anchorUncertainty),
            _state(ClockCorrelationState::Available) {}

        friend struct Detail::ClockCorrelationFactory;

    public:

        // Construction.

        /// Creates an unavailable correlation containing no accepted synchronized reference mapping.
        constexpr ClockCorrelation() noexcept = default;


        // Correlation metadata.

        /// Returns whether this snapshot contains an accepted synchronized reference mapping.
        constexpr ClockCorrelationState State() const noexcept {
            return _state;
        }

        /// Indicates whether this snapshot can project monotonic occurrences into synchronized time.
        constexpr bool IsAvailable() const noexcept {
            return _state == ClockCorrelationState::Available;
        }

        /// Returns the monotonic coordinate at which this reference mapping is anchored.
        constexpr const MonotonicTimestamp& MonotonicAnchor() const noexcept {
            return _monotonicAnchor;
        }

        /// Returns the synchronized reference coordinate at the monotonic anchor.
        constexpr const SynchronizedTimestamp& SynchronizedAnchor() const noexcept {
            return _synchronizedAnchor;
        }

        /// Returns the long-term signed frequency correction used by this mapping.
        constexpr std::int32_t FrequencyCorrectionPartsPerBillion() const noexcept {
            return _frequencyCorrectionPartsPerBillion;
        }

        /// Returns the conservative uncertainty of the frequency estimate in parts per billion.
        constexpr std::uint32_t FrequencyUncertaintyPartsPerBillion() const noexcept {
            return _frequencyUncertaintyPartsPerBillion;
        }

        /// Returns the synchronization uncertainty associated with the mapping anchor.
        constexpr const SynchronizationUncertainty& AnchorUncertainty() const noexcept {
            return _anchorUncertainty;
        }


        // Timestamp correlation.

        /// Projects one captured monotonic occurrence into synchronized time.
        ///
        /// Projection uses the immutable reference mapping represented by this object, not the
        /// phase-slewed published SynchronizedClock timeline. The supplied capture uncertainty is
        /// added to anchor uncertainty and rate-uncertainty growth. Projection may move forward or
        /// backward from the anchor; synchronized coordinate underflow/overflow is reported rather
        /// than clamped.
        ///
        /// @param timestamp Monotonic timestamp captured as close as possible to the occurrence.
        /// @param captureUncertainty Conservative uncertainty introduced while capturing timestamp.
        /// @return Synchronized coordinate estimate, uncertainty and explicit projection status.
        constexpr ClockCorrelationProjection Correlate(
            const MonotonicTimestamp& timestamp,
            const SynchronizationUncertainty& captureUncertainty
        ) const noexcept {
            if (!IsAvailable())
                return ClockCorrelationProjection(
                    SynchronizedTimestamp(),
                    SynchronizationUncertainty::Maximum(),
                    ClockCorrelationProjectionStatus::CorrelationUnavailable
                );

            const auto anchorMonotonic = _monotonicAnchor.Nanoseconds();
            const auto targetMonotonic = timestamp.Nanoseconds();
            const auto backwards = targetMonotonic < anchorMonotonic;
            const auto monotonicDistance = backwards
                ? anchorMonotonic - targetMonotonic
                : targetMonotonic - anchorMonotonic;
            const auto rateMagnitude = Detail::CorrelationRateMagnitude(
                _frequencyCorrectionPartsPerBillion
            );
            const auto rateAdjustment = Detail::ScaleCorrelationElapsed(
                monotonicDistance,
                rateMagnitude
            );
            auto synchronizedDistance = monotonicDistance;

            if (_frequencyCorrectionPartsPerBillion >= 0) {
                constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();

                if (rateAdjustment > (maximum - synchronizedDistance))
                    return ClockCorrelationProjection(
                        SynchronizedTimestamp(),
                        SynchronizationUncertainty::Maximum(),
                        backwards
                            ? ClockCorrelationProjectionStatus::SynchronizedCoordinateUnderflow
                            : ClockCorrelationProjectionStatus::SynchronizedCoordinateOverflow
                    );

                synchronizedDistance += rateAdjustment;
            } else {
                synchronizedDistance -= rateAdjustment;
            }

            const auto synchronizedAnchor = _synchronizedAnchor.Nanoseconds();
            std::uint64_t synchronizedCoordinate = 0U;

            if (backwards) {
                if (synchronizedDistance > synchronizedAnchor)
                    return ClockCorrelationProjection(
                        SynchronizedTimestamp(),
                        SynchronizationUncertainty::Maximum(),
                        ClockCorrelationProjectionStatus::SynchronizedCoordinateUnderflow
                    );

                synchronizedCoordinate = synchronizedAnchor - synchronizedDistance;
            } else {
                constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();

                if (synchronizedDistance > (maximum - synchronizedAnchor))
                    return ClockCorrelationProjection(
                        SynchronizedTimestamp(),
                        SynchronizationUncertainty::Maximum(),
                        ClockCorrelationProjectionStatus::SynchronizedCoordinateOverflow
                    );

                synchronizedCoordinate = synchronizedAnchor + synchronizedDistance;
            }

            auto uncertainty = _anchorUncertainty.Nanoseconds();
            uncertainty = Detail::AddCorrelationUncertainty(
                uncertainty,
                Detail::ScaleCorrelationUncertainty(
                    monotonicDistance,
                    _frequencyUncertaintyPartsPerBillion
                )
            );
            uncertainty = Detail::AddCorrelationUncertainty(
                uncertainty,
                captureUncertainty.Nanoseconds()
            );

            return ClockCorrelationProjection(
                SynchronizedTimestamp::FromNanoseconds(synchronizedCoordinate),
                SynchronizationUncertainty::FromNanoseconds(uncertainty),
                ClockCorrelationProjectionStatus::Correlated
            );
        }

    };


    namespace Detail {

        /// Creates available ClockCorrelation values from accepted synchronized-clock state.
        struct ClockCorrelationFactory final {

            /// Creates one correlation from a coherent accepted reference mapping.
            static constexpr ClockCorrelation Create(
                const MonotonicTimestamp& monotonicAnchor,
                const SynchronizedTimestamp& synchronizedAnchor,
                std::int32_t frequencyCorrectionPartsPerBillion,
                std::uint32_t frequencyUncertaintyPartsPerBillion,
                const SynchronizationUncertainty& anchorUncertainty
            ) noexcept {
                return ClockCorrelation(
                    monotonicAnchor,
                    synchronizedAnchor,
                    frequencyCorrectionPartsPerBillion,
                    frequencyUncertaintyPartsPerBillion,
                    anchorUncertainty
                );
            }

        };

    } // ESPressio::Clock::Detail


    static_assert(
        sizeof(ClockCorrelation) == 32U,
        "ClockCorrelation must remain a compact 32-byte immutable mapping snapshot"
    );

    static_assert(
        std::is_trivially_copyable_v<ClockCorrelation>,
        "ClockCorrelation must remain a trivially copyable value type"
    );

} // ESPressio::Clock
