#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

#include <ESPressio_Platform.hpp>

#include "ClockComposition.hpp"
#include "ClockCorrelation.hpp"
#include "MonotonicTimestamp.hpp"
#include "SynchronizationObservation.hpp"
#include "SynchronizationObservationStatus.hpp"
#include "SynchronizationState.hpp"
#include "SynchronizationUncertainty.hpp"
#include "SynchronizedReading.hpp"
#include "SynchronizedTimestamp.hpp"

namespace ESPressio::Clock {

    namespace Detail {

        /// Validates the MonotonicClock provider used as the physical basis of synchronized time.
        template<class TMonotonicClockProvider>
        struct SynchronizedMonotonicClockTraits {

            static_assert(
                TMonotonicClockProvider::CompositionCapabilities::template Contains<MonotonicClock>,
                "SynchronizedClockProvider requires a provider that supplies MonotonicClock"
            );

            // Monotonic-clock metadata.

            /// Properties advertised by the selected MonotonicClock provider.
            using Properties = typename TMonotonicClockProvider::CompositionCapabilities::template PropertiesFor<MonotonicClock>;

            static_assert(
                Properties::template Contains<ClockResolutionNanoseconds>,
                "MonotonicClock provider must advertise ClockResolutionNanoseconds"
            );

            /// Conservative whole-nanosecond resolution of the selected monotonic clock.
            static constexpr std::uint64_t ResolutionNanoseconds =
                Properties::template Value<ClockResolutionNanoseconds>;

            // Monotonic-clock operation validation.

            /// Return type produced by the selected provider's Now operation.
            using NowResult = decltype(
                std::declval<const TMonotonicClockProvider&>().Now()
            );

            static_assert(
                std::is_same_v<NowResult, MonotonicTimestamp>,
                "MonotonicClock Now() must return MonotonicTimestamp"
            );

            static_assert(
                noexcept(
                    std::declval<const TMonotonicClockProvider&>().Now()
                ),
                "MonotonicClock Now() must be noexcept"
            );

        };


        /// Compact mutable mapping state published coherently to synchronized-clock readers.
        struct SynchronizedClockState final {

            // Mapping anchors.

            /// Monotonic coordinate at which the current mapping state is anchored.
            std::uint64_t MonotonicAnchorNanoseconds;

            /// Published synchronized coordinate at the mapping anchor.
            std::uint64_t SynchronizedAnchorNanoseconds;

            /// Most recent accepted raw reference coordinate at the mapping anchor.
            std::uint64_t ReferenceAnchorNanoseconds;

            /// Remaining signed phase correction to apply through bounded slew.
            std::int64_t RemainingPhaseCorrectionNanoseconds;

            // Discipline estimates.

            /// Long-term signed frequency correction applied to monotonic elapsed time.
            std::int32_t FrequencyCorrectionPartsPerBillion;

            /// Conservative uncertainty of the current frequency estimate in parts per billion.
            std::uint32_t FrequencyUncertaintyPartsPerBillion;

            /// Conservative synchronization uncertainty at the mapping anchor.
            std::uint32_t UncertaintyAtAnchorNanoseconds;

            // Lifecycle metadata.

            /// Last explicitly established lifecycle state for this mapping.
            SynchronizationState State;

            /// Indicates whether at least one observation has been accepted in this runtime.
            std::uint8_t HasObservation;

            /// Reserved zeroed storage keeps the 48-byte snapshot representation fully explicit.
            std::uint16_t Reserved;

        };


        static_assert(
            sizeof(SynchronizedClockState) == 48U,
            "SynchronizedClockState must remain a compact 48-byte discipline snapshot"
        );

        static_assert(
            std::is_trivially_copyable_v<SynchronizedClockState>,
            "SynchronizedClockState must remain trivially copyable for ConcurrentSnapshot"
        );


        /// Exact quotient/remainder of `(numerator * scale) / denominator` for a proper fraction.
        struct ScaledFractionResult final {

            // Scaled fraction result.

            /// Integer quotient of the scaled fraction.
            std::uint64_t Quotient{0U};

            /// Remainder after dividing the scaled numerator by the denominator.
            std::uint64_t Remainder{0U};

        };


        /// Projected synchronized coordinate and uncertainty derived from one immutable state snapshot.
        struct SynchronizedProjection final {

            // Projected reading values.

            /// Projected synchronized coordinate in nanoseconds.
            std::uint64_t TimestampNanoseconds{0U};

            /// Conservative projected uncertainty in nanoseconds.
            std::uint32_t UncertaintyNanoseconds{std::numeric_limits<std::uint32_t>::max()};

            /// Remaining phase correction after the projected elapsed interval.
            std::int64_t RemainingPhaseCorrectionNanoseconds;

        };


        /// Adds two unsigned values and saturates at the 64-bit boundary.
        constexpr std::uint64_t SaturatingAdd64(
            std::uint64_t left,
            std::uint64_t right
        ) noexcept {
            constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();

            if (right > (maximum - left)) return maximum;

            return left + right;
        }

        /// Adds two uncertainty magnitudes and saturates at the 32-bit uncertainty boundary.
        constexpr std::uint32_t SaturatingAddUncertainty(
            std::uint32_t left,
            std::uint64_t right
        ) noexcept {
            constexpr auto maximum = std::numeric_limits<std::uint32_t>::max();

            if (right >= static_cast<std::uint64_t>(maximum)) return maximum;
            if (left > (maximum - static_cast<std::uint32_t>(right))) return maximum;

            return static_cast<std::uint32_t>(
                left + static_cast<std::uint32_t>(right)
            );
        }

        /// Returns the unsigned magnitude of a signed 64-bit value without overflowing at INT64_MIN.
        constexpr std::uint64_t SignedMagnitude(
            std::int64_t value
        ) noexcept {
            if (value >= 0) return static_cast<std::uint64_t>(value);

            return static_cast<std::uint64_t>(-(value + 1)) + 1U;
        }

        /// Reconstructs a signed 64-bit value from a sign and an already-bounded magnitude.
        constexpr std::int64_t SignedFromMagnitude(
            bool negative,
            std::uint64_t magnitude
        ) noexcept {
            constexpr auto minimumMagnitude =
                static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) + 1U;

            if (!negative) {
                if (magnitude > static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()))
                    return std::numeric_limits<std::int64_t>::max();

                return static_cast<std::int64_t>(magnitude);
            }

            if (magnitude >= minimumMagnitude) return std::numeric_limits<std::int64_t>::min();

            return -static_cast<std::int64_t>(magnitude);
        }

        /// Calculates `to - from` as a signed nanosecond difference with deterministic saturation.
        constexpr std::int64_t SignedCoordinateDifference(
            std::uint64_t from,
            std::uint64_t to
        ) noexcept {
            constexpr auto maximumPositiveMagnitude =
                static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());
            constexpr auto maximumNegativeMagnitude = maximumPositiveMagnitude + 1U;

            if (to >= from) {
                const auto magnitude = to - from;

                if (magnitude > maximumPositiveMagnitude) return std::numeric_limits<std::int64_t>::max();

                return static_cast<std::int64_t>(magnitude);
            }

            const auto magnitude = from - to;

            if (magnitude >= maximumNegativeMagnitude) return std::numeric_limits<std::int64_t>::min();

            return -static_cast<std::int64_t>(magnitude);
        }

        /// Scales a proper fraction by a 32-bit integer without requiring a 128-bit intermediate.
        ///
        /// Preconditions: `numerator < denominator` and `denominator != 0`.
        inline ScaledFractionResult ScaleProperFraction(
            std::uint64_t numerator,
            std::uint32_t scale,
            std::uint64_t denominator
        ) noexcept {
            ScaledFractionResult result {};

            for (std::uint32_t bit = 32U; bit > 0U; --bit) {
                result.Quotient *= 2U;

                // Double the current remainder modulo the denominator without calculating an
                // overflowing `remainder * 2` intermediate.
                if (result.Remainder >= (denominator - result.Remainder)) {
                    result.Remainder -= denominator - result.Remainder;
                    ++result.Quotient;
                } else {
                    result.Remainder += result.Remainder;
                }

                const auto mask = static_cast<std::uint32_t>(1U << (bit - 1U));

                if ((scale & mask) == 0U) continue;

                // Add one numerator contribution for the current multiplier bit, again retaining
                // the result as quotient/remainder rather than forming a potentially huge product.
                if (result.Remainder >= (denominator - numerator)) {
                    result.Remainder -= denominator - numerator;
                    ++result.Quotient;
                } else {
                    result.Remainder += numerator;
                }
            }

            return result;
        }

        /// Scales elapsed nanoseconds by a bounded parts-per-billion magnitude.
        inline std::uint64_t ScaleElapsedByPartsPerBillion(
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
        inline std::uint32_t ScaleElapsedUncertainty(
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

        /// Returns the remaining signed phase correction after applying a bounded unsigned amount.
        constexpr std::int64_t RemainingPhaseAfterApplication(
            std::int64_t phaseCorrection,
            std::uint64_t appliedMagnitude
        ) noexcept {
            const auto magnitude = SignedMagnitude(phaseCorrection);
            const auto remainingMagnitude = magnitude > appliedMagnitude
                ? magnitude - appliedMagnitude
                : 0U;

            return SignedFromMagnitude(
                phaseCorrection < 0,
                remainingMagnitude
            );
        }

        /// Estimates the signed source-frequency correction from two accepted observations.
        ///
        /// @param localDeltaNanoseconds Monotonic elapsed time between observations.
        /// @param referenceDeltaNanoseconds Synchronized-reference elapsed time between observations.
        /// @param maximumCorrectionPartsPerBillion Maximum permitted absolute correction.
        /// @param correction Receives the rounded signed correction when the estimate is acceptable.
        /// @return true when the estimate is within the configured correction bound.
        inline bool EstimateFrequencyCorrection(
            std::uint64_t localDeltaNanoseconds,
            std::uint64_t referenceDeltaNanoseconds,
            std::uint32_t maximumCorrectionPartsPerBillion,
            std::int32_t& correction
        ) noexcept {
            if (referenceDeltaNanoseconds == localDeltaNanoseconds) {
                correction = 0;
                return true;
            }

            const auto referenceIsFaster = referenceDeltaNanoseconds > localDeltaNanoseconds;
            const auto difference = referenceIsFaster
                ? referenceDeltaNanoseconds - localDeltaNanoseconds
                : localDeltaNanoseconds - referenceDeltaNanoseconds;

            // Any difference at least as large as the local interval represents at least one
            // billion ppb and therefore exceeds every permitted configuration for this provider.
            if (difference >= localDeltaNanoseconds) return false;

            const auto scaled = ScaleProperFraction(
                difference,
                1000000000U,
                localDeltaNanoseconds
            );
            const auto ceiling = scaled.Quotient + (scaled.Remainder == 0U ? 0U : 1U);

            if (ceiling > maximumCorrectionPartsPerBillion) return false;

            const auto roundedMagnitude = scaled.Quotient +
                (
                    (scaled.Remainder >= (localDeltaNanoseconds - scaled.Remainder))
                        ? 1U
                        : 0U
                );
            const auto boundedMagnitude = std::min<std::uint64_t>(
                roundedMagnitude,
                maximumCorrectionPartsPerBillion
            );
            const auto signedMagnitude = static_cast<std::int32_t>(boundedMagnitude);

            correction = referenceIsFaster ? signedMagnitude : -signedMagnitude;
            return true;
        }

        /// Estimates frequency uncertainty from the uncertainty of two observation coordinates.
        inline std::uint32_t EstimateFrequencyUncertainty(
            std::uint32_t previousUncertaintyNanoseconds,
            std::uint32_t currentUncertaintyNanoseconds,
            std::uint64_t localDeltaNanoseconds
        ) noexcept {
            constexpr std::uint64_t billion = 1000000000ULL;
            constexpr auto maximum = std::numeric_limits<std::uint32_t>::max();

            if (localDeltaNanoseconds == 0U) return maximum;

            const auto combinedUncertainty =
                static_cast<std::uint64_t>(previousUncertaintyNanoseconds) +
                static_cast<std::uint64_t>(currentUncertaintyNanoseconds);

            if (combinedUncertainty == 0U) return 0U;

            const auto scaledUncertainty = combinedUncertainty * billion;
            const auto quotient = scaledUncertainty / localDeltaNanoseconds;
            const auto remainder = scaledUncertainty % localDeltaNanoseconds;
            const auto ceiling = quotient + (remainder == 0U ? 0U : 1U);

            return ceiling > static_cast<std::uint64_t>(maximum)
                ? maximum
                : static_cast<std::uint32_t>(ceiling);
        }

    } // ESPressio::Clock::Detail


    /// Generic synchronized clock disciplined from source-agnostic synchronization observations.
    ///
    /// The synchronized clock is derived entirely from the selected MonotonicClock. It owns no
    /// second physical counter. Mutable discipline state is published through EDP-Platform's
    /// fixed-storage ConcurrentSnapshot so one writer can update the mapping while arbitrary
    /// application threads call Now() without a mutex, heap allocation, worker task, or vtable.
    ///
    /// Exactly one execution context may call Observe() for a given provider. Source selection and
    /// serialization of observations therefore remain upstream responsibilities.
    ///
    /// @tparam TMonotonicClockProvider Concrete application-selected MonotonicClock provider.
    /// @tparam TAtomicWordProvider Concrete application-selected Platform AtomicWord32 provider.
    /// @tparam TMaximumFrequencyCorrectionPartsPerBillion Maximum long-term correction magnitude.
    /// @tparam TMaximumPhaseSlewPartsPerBillion Maximum temporary phase-slew magnitude.
    /// @tparam TSynchronizationUncertaintyLimitNanoseconds Exclusive synchronization-quality limit.
    template<
        class TMonotonicClockProvider,
        class TAtomicWordProvider,
        std::uint32_t TMaximumFrequencyCorrectionPartsPerBillion = 1000000U,
        std::uint32_t TMaximumPhaseSlewPartsPerBillion = 5000000U,
        std::uint32_t TSynchronizationUncertaintyLimitNanoseconds = 1000000U
    >
    class SynchronizedClockProvider final : public Framework::Provider<
        Domain,
        Framework::Provides<
            Framework::Offer<
                SynchronizedClock,
                Framework::PropertyValue<
                    SynchronizedClockUncertaintyLimitNanoseconds,
                    TSynchronizationUncertaintyLimitNanoseconds
                >,
                Framework::PropertyValue<
                    SynchronizedClockMaximumFrequencyCorrectionPartsPerBillion,
                    TMaximumFrequencyCorrectionPartsPerBillion
                >,
                Framework::PropertyValue<
                    SynchronizedClockMaximumPhaseSlewPartsPerBillion,
                    TMaximumPhaseSlewPartsPerBillion
                >
            >
        >,
        Framework::Requires<
            Framework::Need<
                MonotonicClock,
                Framework::LessThan<
                    ClockResolutionNanoseconds,
                    TSynchronizationUncertaintyLimitNanoseconds
                >
            >
        >
    > {
    private:

        /// Validated monotonic-clock metadata used by this provider.
        using MonotonicTraits = Detail::SynchronizedMonotonicClockTraits<TMonotonicClockProvider>;

        /// Fixed-storage concurrent discipline snapshot supplied by EDP-Platform.
        using StateSnapshot = ESPressio::Platform::Concurrency::ConcurrentSnapshot<
            Detail::SynchronizedClockState,
            TAtomicWordProvider
        >;

        static_assert(
            TMaximumFrequencyCorrectionPartsPerBillion <= static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max()),
            "Maximum synchronized-clock frequency correction must fit in signed 32-bit ppb"
        );

        static_assert(
            TMaximumFrequencyCorrectionPartsPerBillion < 1000000000U,
            "Maximum synchronized-clock frequency correction must remain below one billion ppb"
        );

        static_assert(
            TMaximumPhaseSlewPartsPerBillion > 0U,
            "Maximum synchronized-clock phase slew must be non-zero"
        );

        static_assert(
            TMaximumPhaseSlewPartsPerBillion < 1000000000U,
            "Maximum synchronized-clock phase slew must remain below one billion ppb"
        );

        static_assert(
            TMaximumFrequencyCorrectionPartsPerBillion <
                (1000000000U - TMaximumPhaseSlewPartsPerBillion),
            "Combined negative frequency correction and phase slew must never stop or reverse time"
        );

        static_assert(
            TSynchronizationUncertaintyLimitNanoseconds > 0U,
            "Synchronization uncertainty limit must be non-zero"
        );

        static_assert(
            MonotonicTraits::ResolutionNanoseconds < TSynchronizationUncertaintyLimitNanoseconds,
            "MonotonicClock resolution must be strictly better than the synchronization uncertainty limit"
        );

        static_assert(
            MonotonicTraits::ResolutionNanoseconds <= std::numeric_limits<std::uint32_t>::max(),
            "MonotonicClock resolution must fit in synchronized uncertainty storage"
        );

        // Clock dependencies and discipline state.

        /// Borrowed lifetime-stable monotonic clock selected by application Bootstrap.
        const TMonotonicClockProvider* _monotonicClock;

        /// Coherently published mutable synchronized-clock discipline state.
        StateSnapshot _state;


        // State initialization.

        /// Creates the initial never-synchronized discipline state for this runtime incarnation.
        static Detail::SynchronizedClockState InitialState() noexcept {
            Detail::SynchronizedClockState state {};

            state.FrequencyUncertaintyPartsPerBillion = TMaximumFrequencyCorrectionPartsPerBillion;
            state.UncertaintyAtAnchorNanoseconds = std::numeric_limits<std::uint32_t>::max();
            state.State = SynchronizationState::NeverSynchronized;
            state.HasObservation = 0U;
            return state;
        }


        // Quality arithmetic.

        /// Combines observation uncertainty with monotonic-clock resolution conservatively.
        static std::uint32_t ObservationUncertainty(
            const SynchronizationObservation& observation
        ) noexcept {
            return Detail::SaturatingAddUncertainty(
                observation.Uncertainty().Nanoseconds(),
                MonotonicTraits::ResolutionNanoseconds
            );
        }

        /// Derives the externally visible lifecycle state for one projected reading.
        static SynchronizationState EffectiveState(
            SynchronizationState storedState,
            std::uint32_t uncertaintyNanoseconds
        ) noexcept {
            const auto saturated = uncertaintyNanoseconds == std::numeric_limits<std::uint32_t>::max();
            const auto withinContract = uncertaintyNanoseconds < TSynchronizationUncertaintyLimitNanoseconds;

            switch (storedState) {
                case SynchronizationState::NeverSynchronized:
                    return SynchronizationState::NeverSynchronized;

                case SynchronizationState::Synchronized:
                    if (saturated) return SynchronizationState::LostSynchronization;
                    if (withinContract) return SynchronizationState::Synchronized;
                    return SynchronizationState::Holdover;

                case SynchronizationState::Holdover:
                    if (saturated) return SynchronizationState::LostSynchronization;
                    if (withinContract) return SynchronizationState::Synchronized;
                    return SynchronizationState::Holdover;

                case SynchronizationState::Reacquiring:
                    if (saturated) return SynchronizationState::LostSynchronization;
                    if (withinContract) return SynchronizationState::Synchronized;
                    return SynchronizationState::Reacquiring;

                case SynchronizationState::LostSynchronization:
                    return SynchronizationState::LostSynchronization;
            }

            return SynchronizationState::LostSynchronization;
        }


        // Mapping projection.

        /// Projects one immutable discipline snapshot to the supplied monotonic coordinate.
        static Detail::SynchronizedProjection Project(
            const Detail::SynchronizedClockState& state,
            std::uint64_t monotonicNanoseconds
        ) noexcept {
            if (state.HasObservation == 0U) return Detail::SynchronizedProjection {};

            const auto elapsedNanoseconds = monotonicNanoseconds >= state.MonotonicAnchorNanoseconds
                ? monotonicNanoseconds - state.MonotonicAnchorNanoseconds
                : 0U;
            auto adjustedElapsedNanoseconds = elapsedNanoseconds;

            const auto frequencyMagnitude = Detail::ScaleElapsedByPartsPerBillion(
                elapsedNanoseconds,
                static_cast<std::uint32_t>(
                    state.FrequencyCorrectionPartsPerBillion < 0
                        ? -static_cast<std::int64_t>(state.FrequencyCorrectionPartsPerBillion)
                        : state.FrequencyCorrectionPartsPerBillion
                )
            );

            if (state.FrequencyCorrectionPartsPerBillion >= 0) {
                adjustedElapsedNanoseconds = Detail::SaturatingAdd64(
                    adjustedElapsedNanoseconds,
                    frequencyMagnitude
                );
            } else {
                adjustedElapsedNanoseconds = frequencyMagnitude > adjustedElapsedNanoseconds
                    ? 0U
                    : adjustedElapsedNanoseconds - frequencyMagnitude;
            }

            const auto phaseMagnitude = Detail::SignedMagnitude(
                state.RemainingPhaseCorrectionNanoseconds
            );
            const auto maximumPhaseApplication = Detail::ScaleElapsedByPartsPerBillion(
                elapsedNanoseconds,
                TMaximumPhaseSlewPartsPerBillion
            );
            const auto phaseApplication = std::min(
                phaseMagnitude,
                maximumPhaseApplication
            );

            if (state.RemainingPhaseCorrectionNanoseconds >= 0) {
                adjustedElapsedNanoseconds = Detail::SaturatingAdd64(
                    adjustedElapsedNanoseconds,
                    phaseApplication
                );
            } else {
                adjustedElapsedNanoseconds = phaseApplication > adjustedElapsedNanoseconds
                    ? 0U
                    : adjustedElapsedNanoseconds - phaseApplication;
            }

            const auto remainingPhase = Detail::RemainingPhaseAfterApplication(
                state.RemainingPhaseCorrectionNanoseconds,
                phaseApplication
            );
            auto uncertainty = state.UncertaintyAtAnchorNanoseconds;
            const auto rateGrowth = Detail::ScaleElapsedUncertainty(
                elapsedNanoseconds,
                state.FrequencyUncertaintyPartsPerBillion
            );

            uncertainty = Detail::SaturatingAddUncertainty(
                uncertainty,
                rateGrowth
            );
            uncertainty = Detail::SaturatingAddUncertainty(
                uncertainty,
                Detail::SignedMagnitude(remainingPhase)
            );

            return Detail::SynchronizedProjection {
                Detail::SaturatingAdd64(
                    state.SynchronizedAnchorNanoseconds,
                    adjustedElapsedNanoseconds
                ),
                uncertainty,
                remainingPhase
            };
        }

        /// Projects one state snapshot to a complete public reading at a monotonic coordinate.
        static SynchronizedReading ReadingAt(
            const Detail::SynchronizedClockState& state,
            const MonotonicTimestamp& monotonicTimestamp
        ) noexcept {
            const auto projection = Project(
                state,
                monotonicTimestamp.Nanoseconds()
            );
            const auto effectiveState = EffectiveState(
                state.State,
                projection.UncertaintyNanoseconds
            );

            return SynchronizedReading(
                SynchronizedTimestamp::FromNanoseconds(
                    projection.TimestampNanoseconds
                ),
                SynchronizationUncertainty::FromNanoseconds(
                    projection.UncertaintyNanoseconds
                ),
                effectiveState
            );
        }


        // Rejected-observation handling.

        /// Degrades a previously synchronized mapping after an implausible frequency observation.
        void DegradeForRejectedFrequency(
            Detail::SynchronizedClockState state
        ) noexcept {
            if (state.State == SynchronizationState::NeverSynchronized) return;

            state.UncertaintyAtAnchorNanoseconds = std::max(
                state.UncertaintyAtAnchorNanoseconds,
                TSynchronizationUncertaintyLimitNanoseconds
            );

            if (state.State != SynchronizationState::LostSynchronization)
                state.State = SynchronizationState::Reacquiring;

            _state.Publish(state);
        }

    public:

        // Construction and lifetime.

        /// Creates a synchronized clock over one lifetime-stable MonotonicClock provider.
        ///
        /// @param monotonicClock Monotonic clock selected and initialized by application Bootstrap.
        explicit SynchronizedClockProvider(
            const TMonotonicClockProvider& monotonicClock
        ) noexcept :
            _monotonicClock(&monotonicClock),
            _state(InitialState()) {}

        /// Prevents duplicating live synchronized-clock discipline state.
        SynchronizedClockProvider(const SynchronizedClockProvider&) = delete;

        /// Prevents copy assignment of live synchronized-clock discipline state.
        SynchronizedClockProvider& operator =(const SynchronizedClockProvider&) = delete;

        /// Prevents relocating a synchronized clock after readers may reference it.
        SynchronizedClockProvider(SynchronizedClockProvider&&) = delete;

        /// Prevents move assignment of live synchronized-clock discipline state.
        SynchronizedClockProvider& operator =(SynchronizedClockProvider&&) = delete;


        // Policy metadata.

        /// Strict upper bound for uncertainty while a reading may report Synchronized.
        static constexpr std::uint32_t SynchronizationUncertaintyLimitNanoseconds =
            TSynchronizationUncertaintyLimitNanoseconds;

        /// Maximum permitted magnitude of long-term source-frequency correction.
        static constexpr std::uint32_t MaximumFrequencyCorrectionPartsPerBillion =
            TMaximumFrequencyCorrectionPartsPerBillion;

        /// Maximum permitted magnitude of temporary phase slew.
        static constexpr std::uint32_t MaximumPhaseSlewPartsPerBillion =
            TMaximumPhaseSlewPartsPerBillion;

        /// Consumer state bytes retained inside one ConcurrentSnapshot buffer.
        static constexpr std::size_t DisciplineStateBytes = sizeof(Detail::SynchronizedClockState);

        /// Total fixed atomic storage retained by the discipline ConcurrentSnapshot.
        static constexpr std::size_t ConcurrentStateStorageBytes = StateSnapshot::AtomicStorageBytes;


        // Clock correlation.

        /// Returns one immutable snapshot of the best accepted monotonic-to-reference mapping.
        ///
        /// Correlation uses the raw accepted reference anchor rather than the phase-slewed published
        /// SynchronizedClock anchor. The returned value is therefore suitable for retrospectively
        /// projecting hardware/event monotonic timestamps even while the public clock is Reacquiring.
        ClockCorrelation Correlation() const noexcept {
            const auto state = _state.Read();

            if (state.HasObservation == 0U) return ClockCorrelation();

            return Detail::ClockCorrelationFactory::Create(
                MonotonicTimestamp::FromNanoseconds(
                    state.MonotonicAnchorNanoseconds
                ),
                SynchronizedTimestamp::FromNanoseconds(
                    state.ReferenceAnchorNanoseconds
                ),
                state.FrequencyCorrectionPartsPerBillion,
                state.FrequencyUncertaintyPartsPerBillion,
                SynchronizationUncertainty::FromNanoseconds(
                    state.UncertaintyAtAnchorNanoseconds
                )
            );
        }


        // Clock reading.

        /// Returns the current synchronized timestamp and its quality information.
        ///
        /// The coherent discipline snapshot is captured before the monotonic sample. Therefore a
        /// concurrent writer may publish a newer mapping while this call executes, but the sampled
        /// monotonic coordinate can never precede the anchor contained by the state being projected.
        SynchronizedReading Now() const noexcept {
            const auto state = _state.Read();
            const auto monotonicTimestamp = _monotonicClock->Now();

            return ReadingAt(
                state,
                monotonicTimestamp
            );
        }


        // Clock discipline.

        /// Applies one source-agnostic synchronization observation to the clock discipline.
        ///
        /// Exactly one execution context may call Observe() for this provider. Observations must be
        /// serialized upstream before entering Clock. A first-ever mapping may step directly to the
        /// reference coordinate; after successful synchronization, corrections are applied through
        /// bounded slew so the published synchronized coordinate never moves backwards.
        ///
        /// @param observation Accepted upstream estimate relating monotonic and synchronized time.
        /// @return Status describing whether the observation was accepted by Clock discipline.
        SynchronizationObservationStatus Observe(
            const SynchronizationObservation& observation
        ) noexcept {
            const auto currentMonotonic = _monotonicClock->Now();

            if (observation.LocalTimestamp() > currentMonotonic)
                return SynchronizationObservationStatus::RejectedFutureObservation;

            auto currentState = _state.Read();
            const auto observationMonotonic = observation.LocalTimestamp().Nanoseconds();

            if (
                (currentState.HasObservation != 0U) &&
                (observationMonotonic < currentState.MonotonicAnchorNanoseconds)
            ) {
                return SynchronizationObservationStatus::RejectedOutOfOrderObservation;
            }

            const auto currentObservationUncertainty = ObservationUncertainty(observation);
            auto nextFrequencyCorrection = currentState.FrequencyCorrectionPartsPerBillion;
            auto nextFrequencyUncertainty = currentState.FrequencyUncertaintyPartsPerBillion;

            if (
                (currentState.HasObservation != 0U) &&
                (observationMonotonic > currentState.MonotonicAnchorNanoseconds)
            ) {
                const auto localDelta = observationMonotonic - currentState.MonotonicAnchorNanoseconds;
                const auto currentReference = observation.ReferenceTimestamp().Nanoseconds();

                if (currentReference < currentState.ReferenceAnchorNanoseconds) {
                    DegradeForRejectedFrequency(currentState);
                    return SynchronizationObservationStatus::RejectedFrequencyCorrection;
                }

                const auto referenceDelta = currentReference - currentState.ReferenceAnchorNanoseconds;

                if (!Detail::EstimateFrequencyCorrection(
                    localDelta,
                    referenceDelta,
                    TMaximumFrequencyCorrectionPartsPerBillion,
                    nextFrequencyCorrection
                )) {
                    DegradeForRejectedFrequency(currentState);
                    return SynchronizationObservationStatus::RejectedFrequencyCorrection;
                }

                nextFrequencyUncertainty = Detail::EstimateFrequencyUncertainty(
                    currentState.UncertaintyAtAnchorNanoseconds,
                    currentObservationUncertainty,
                    localDelta
                );
            }

            Detail::SynchronizedClockState nextState {};
            nextState.MonotonicAnchorNanoseconds = observationMonotonic;
            nextState.ReferenceAnchorNanoseconds = observation.ReferenceTimestamp().Nanoseconds();
            nextState.FrequencyCorrectionPartsPerBillion = nextFrequencyCorrection;
            nextState.FrequencyUncertaintyPartsPerBillion = currentState.HasObservation == 0U
                ? TMaximumFrequencyCorrectionPartsPerBillion
                : nextFrequencyUncertainty;
            nextState.UncertaintyAtAnchorNanoseconds = currentObservationUncertainty;
            nextState.HasObservation = 1U;

            const auto hasEverSynchronized = currentState.State != SynchronizationState::NeverSynchronized;

            if (!hasEverSynchronized) {
                // Before synchronized time has ever satisfied its contract, the mapping is not yet
                // authoritative to consumers and may therefore step directly to a better reference.
                nextState.SynchronizedAnchorNanoseconds = observation.ReferenceTimestamp().Nanoseconds();
                nextState.RemainingPhaseCorrectionNanoseconds = 0;
                nextState.State = currentObservationUncertainty < TSynchronizationUncertaintyLimitNanoseconds
                    ? SynchronizationState::Synchronized
                    : SynchronizationState::NeverSynchronized;

                _state.Publish(nextState);
                return SynchronizationObservationStatus::Accepted;
            }

            const auto currentProjection = Project(
                currentState,
                observationMonotonic
            );
            nextState.SynchronizedAnchorNanoseconds = currentProjection.TimestampNanoseconds;
            nextState.RemainingPhaseCorrectionNanoseconds = Detail::SignedCoordinateDifference(
                currentProjection.TimestampNanoseconds,
                observation.ReferenceTimestamp().Nanoseconds()
            );

            const auto anchorUncertainty = Detail::SaturatingAddUncertainty(
                currentObservationUncertainty,
                Detail::SignedMagnitude(nextState.RemainingPhaseCorrectionNanoseconds)
            );
            nextState.State = anchorUncertainty < TSynchronizationUncertaintyLimitNanoseconds
                ? SynchronizationState::Synchronized
                : SynchronizationState::Reacquiring;

            _state.Publish(nextState);
            return SynchronizationObservationStatus::Accepted;
        }

    };

} // ESPressio::Clock
