#pragma once

#include <type_traits>
#include <utility>

#include "MonotonicTimestamp.hpp"

namespace ESPressio::Clock {

    namespace Detail {

        /// Process-lifetime binding used by the MonotonicNow convenience function.
        struct MonotonicClockBinding final {

            // Bound clock state.

            /// Borrowed address of the application-selected monotonic clock.
            const void* Context{nullptr};

            /// Type-erased read operation for the application-selected monotonic clock.
            MonotonicTimestamp (*Read)(const void*) noexcept{nullptr};

        };


        /// Single application-wide monotonic clock binding established during Bootstrap.
        inline MonotonicClockBinding ApplicationMonotonicClock;


        /// Invokes the concrete bound clock without requiring runtime inheritance or a vtable.
        ///
        /// @tparam TClock Concrete clock type bound by Bootstrap.
        /// @param context Borrowed concrete clock address.
        /// @return Current monotonic timestamp.
        template<class TClock>
        MonotonicTimestamp ReadBoundMonotonicClock(
            const void* context
        ) noexcept {
            return static_cast<const TClock*>(context)->Now();
        }

    } // ESPressio::Clock::Detail


    /// Permanently binds the application-wide MonotonicNow convenience call to one clock instance.
    ///
    /// Binding is a Bootstrap operation and must complete before any concurrently executing code
    /// can call MonotonicNow(). The binding is intentionally irreversible for the running lifetime.
    ///
    /// @tparam TClock Concrete monotonic clock type.
    /// @param clock Lifetime-stable monotonic clock selected by Bootstrap.
    /// @return true when the clock was bound; false when a clock was already bound.
    template<class TClock>
    bool BindMonotonicClock(
        const TClock& clock
    ) noexcept {
        using NowResult = decltype(
            std::declval<const TClock&>().Now()
        );

        static_assert(
            std::is_same_v<NowResult, MonotonicTimestamp>,
            "Bound monotonic clock Now() must return MonotonicTimestamp"
        );

        static_assert(
            noexcept(
                std::declval<const TClock&>().Now()
            ),
            "Bound monotonic clock Now() must be noexcept"
        );

        if (Detail::ApplicationMonotonicClock.Read != nullptr) return false;

        Detail::ApplicationMonotonicClock.Context = &clock;
        Detail::ApplicationMonotonicClock.Read = &Detail::ReadBoundMonotonicClock<TClock>;
        return true;
    }

    /// Indicates whether application Bootstrap has permanently bound MonotonicNow().
    inline bool IsMonotonicClockBound() noexcept {
        return Detail::ApplicationMonotonicClock.Read != nullptr;
    }

    /// Returns the current application-wide monotonic timestamp.
    ///
    /// Application Bootstrap must bind a lifetime-stable clock through BindMonotonicClock() before
    /// this function is called. No availability branch is placed on this high-frequency read path;
    /// calling MonotonicNow() before successful Bootstrap binding is therefore a lifecycle error.
    inline MonotonicTimestamp MonotonicNow() noexcept {
        return Detail::ApplicationMonotonicClock.Read(
            Detail::ApplicationMonotonicClock.Context
        );
    }

} // ESPressio::Clock
