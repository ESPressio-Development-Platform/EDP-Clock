#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

#include "ClockComposition.hpp"
#include "Duration.hpp"
#include "MonotonicTimestamp.hpp"

namespace ESPressio::Clock {

    namespace Detail {

        /// Validates the MonotonicClock provider consumed by Stopwatch.
        template<class TMonotonicClockProvider>
        struct StopwatchMonotonicClockTraits {

            static_assert(
                TMonotonicClockProvider::CompositionCapabilities::template Contains<MonotonicClock>,
                "Stopwatch requires a provider that supplies MonotonicClock"
            );

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

            // Validation result.

            /// Indicates that all compile-time Stopwatch provider requirements are satisfied.
            static constexpr bool IsValid = true;

        };


        /// Largest non-negative duration representable through Duration.
        inline constexpr std::uint64_t MaximumStopwatchNanoseconds =
            static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());


        /// Adds elapsed nanoseconds and saturates at Duration::Maximum().
        constexpr std::uint64_t SaturatingStopwatchAdd(
            std::uint64_t accumulatedNanoseconds,
            std::uint64_t additionalNanoseconds
        ) noexcept {
            if (accumulatedNanoseconds >= MaximumStopwatchNanoseconds) return MaximumStopwatchNanoseconds;

            if (additionalNanoseconds > (MaximumStopwatchNanoseconds - accumulatedNanoseconds)) return MaximumStopwatchNanoseconds;

            return accumulatedNanoseconds + additionalNanoseconds;
        }

        /// Returns a non-negative monotonic interval without permitting unsigned underflow.
        constexpr std::uint64_t StopwatchIntervalNanoseconds(
            const MonotonicTimestamp& started,
            const MonotonicTimestamp& finished
        ) noexcept {
            if (finished < started) return 0U;

            return finished.Nanoseconds() - started.Nanoseconds();
        }

    } // ESPressio::Clock::Detail


    /// Small stateful elapsed-time utility driven exclusively by one MonotonicClock provider.
    ///
    /// Stopwatch owns no clock, task, timer, synchronization primitive, heap allocation, callback,
    /// or history. It retains only a borrowed provider pointer and fixed local measurement state.
    /// Synchronization discipline, Clock Correlation, RTCs, and civil time cannot affect it.
    ///
    /// @tparam TMonotonicClockProvider Concrete MonotonicClock provider selected by application Bootstrap.
    template<class TMonotonicClockProvider>
    class Stopwatch final {
    private:

        // Provider contract.

        /// Compile-time validation of the supplied MonotonicClock provider.
        using ProviderTraits = Detail::StopwatchMonotonicClockTraits<TMonotonicClockProvider>;

        static_assert(
            ProviderTraits::IsValid,
            "Stopwatch MonotonicClock provider validation failed"
        );


        // Measurement state.

        /// Monotonic coordinate captured by the most recent successful Start() or Restart().
        MonotonicTimestamp _runningSince{};

        /// Elapsed nanoseconds accumulated across completed running intervals.
        std::uint64_t _accumulatedNanoseconds{0U};

        /// Borrowed lifetime-stable MonotonicClock provider.
        const TMonotonicClockProvider* _clock;

        /// Indicates whether the stopwatch currently accumulates elapsed time.
        std::uint8_t _running{0U};


        // Internal elapsed-time calculation.

        /// Calculates elapsed nanoseconds against one already-captured current timestamp.
        std::uint64_t ElapsedNanosecondsAt(
            const MonotonicTimestamp& now
        ) const noexcept {
            if (_running == 0U) return _accumulatedNanoseconds;

            return Detail::SaturatingStopwatchAdd(
                _accumulatedNanoseconds,
                Detail::StopwatchIntervalNanoseconds(
                    _runningSince,
                    now
                )
            );
        }

        /// Resets one moved-from Stopwatch to a valid stopped state while retaining its provider binding.
        void ResetMovedFromState() noexcept {
            _runningSince = MonotonicTimestamp{};
            _accumulatedNanoseconds = 0U;
            _running = 0U;
        }

    public:

        // Construction and lifetime.

        /// Creates a stopped zero-duration Stopwatch over the supplied MonotonicClock provider.
        ///
        /// @param clock Lifetime-stable MonotonicClock provider used for every measurement.
        explicit Stopwatch(
            const TMonotonicClockProvider& clock
        ) noexcept :
            _clock(&clock) {}

        /// Prevents accidental duplication of one mutable measurement session.
        Stopwatch(const Stopwatch&) = delete;

        /// Prevents accidental copy replacement of one mutable measurement session.
        Stopwatch& operator =(const Stopwatch&) = delete;

        /// Transfers one measurement session and resets the moved-from Stopwatch to stopped zero state.
        Stopwatch(
            Stopwatch&& other
        ) noexcept :
            _runningSince(other._runningSince),
            _accumulatedNanoseconds(other._accumulatedNanoseconds),
            _clock(other._clock),
            _running(other._running) {
            other.ResetMovedFromState();
        }

        /// Transfers one measurement session and resets the moved-from Stopwatch to stopped zero state.
        Stopwatch& operator =(
            Stopwatch&& other
        ) noexcept {
            if (this == &other) return *this;

            _runningSince = other._runningSince;
            _accumulatedNanoseconds = other._accumulatedNanoseconds;
            _clock = other._clock;
            _running = other._running;

            other.ResetMovedFromState();

            return *this;
        }


        // Measurement control.

        /// Starts or resumes elapsed-time accumulation without clearing already accumulated time.
        ///
        /// Calling Start() while already running is a no-op and does not read the Clock.
        void Start() noexcept {
            if (_running != 0U) return;

            _runningSince = _clock->Now();
            _running = 1U;
        }

        /// Stops elapsed-time accumulation and freezes the measured duration.
        ///
        /// Calling Stop() while already stopped is a no-op and does not read the Clock.
        void Stop() noexcept {
            if (_running == 0U) return;

            const auto now = _clock->Now();

            _accumulatedNanoseconds = ElapsedNanosecondsAt(now);
            _runningSince = MonotonicTimestamp{};
            _running = 0U;
        }

        /// Clears all elapsed time and leaves the Stopwatch stopped without reading the Clock.
        void Reset() noexcept {
            _runningSince = MonotonicTimestamp{};
            _accumulatedNanoseconds = 0U;
            _running = 0U;
        }

        /// Clears all elapsed time and immediately starts a fresh measurement interval.
        void Restart() noexcept {
            const auto now = _clock->Now();

            _runningSince = now;
            _accumulatedNanoseconds = 0U;
            _running = 1U;
        }


        // Measurement access.

        /// Returns the current measured elapsed duration.
        ///
        /// A running Stopwatch reads the MonotonicClock exactly once. A stopped Stopwatch performs
        /// no Clock read. Elapsed time saturates at Duration::Maximum() rather than wrapping.
        Duration Elapsed() const noexcept {
            if (_running == 0U) {
                return Duration::FromNanoseconds(
                    static_cast<std::int64_t>(_accumulatedNanoseconds)
                );
            }

            const auto now = _clock->Now();
            const auto elapsedNanoseconds = ElapsedNanosecondsAt(now);

            return Duration::FromNanoseconds(
                static_cast<std::int64_t>(elapsedNanoseconds)
            );
        }

        /// Indicates whether elapsed-time accumulation is currently active.
        bool IsRunning() const noexcept {
            return _running != 0U;
        }

    };

} // ESPressio::Clock
