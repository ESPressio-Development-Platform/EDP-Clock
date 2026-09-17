#pragma once

#include <type_traits>
#include <utility>

#include "SynchronizedReading.hpp"

namespace ESPressio::Clock {

    namespace Detail {

        /// Process-lifetime binding used by the SynchronizedNow convenience function.
        struct SynchronizedClockBinding final {

            // Bound clock state.

            /// Borrowed address of the application-selected synchronized clock.
            const void* Context{nullptr};

            /// Type-erased read operation for the application-selected synchronized clock.
            SynchronizedReading (*Read)(const void*) noexcept{nullptr};

        };


        /// Single application-wide synchronized clock binding established during Bootstrap.
        inline SynchronizedClockBinding ApplicationSynchronizedClock;


        /// Invokes the concrete bound synchronized clock without runtime inheritance or a vtable.
        ///
        /// @tparam TClock Concrete synchronized clock type bound by Bootstrap.
        /// @param context Borrowed concrete clock address.
        /// @return Current synchronized reading and quality information.
        template<class TClock>
        SynchronizedReading ReadBoundSynchronizedClock(
            const void* context
        ) noexcept {
            return static_cast<const TClock*>(context)->Now();
        }

    } // ESPressio::Clock::Detail


    /// Permanently binds the application-wide SynchronizedNow convenience call to one clock instance.
    ///
    /// Binding is a Bootstrap operation and must complete before concurrently executing code can
    /// call SynchronizedNow(). The binding is intentionally irreversible for the running lifetime.
    ///
    /// @tparam TClock Concrete synchronized clock type.
    /// @param clock Lifetime-stable synchronized clock selected by Bootstrap.
    /// @return true when the clock was bound; false when a clock was already bound.
    template<class TClock>
    bool BindSynchronizedClock(
        const TClock& clock
    ) noexcept {
        using NowResult = decltype(
            std::declval<const TClock&>().Now()
        );

        static_assert(
            std::is_same_v<NowResult, SynchronizedReading>,
            "Bound synchronized clock Now() must return SynchronizedReading"
        );

        static_assert(
            noexcept(
                std::declval<const TClock&>().Now()
            ),
            "Bound synchronized clock Now() must be noexcept"
        );

        if (Detail::ApplicationSynchronizedClock.Read != nullptr) return false;

        Detail::ApplicationSynchronizedClock.Context = &clock;
        Detail::ApplicationSynchronizedClock.Read = &Detail::ReadBoundSynchronizedClock<TClock>;
        return true;
    }

    /// Indicates whether application Bootstrap has permanently bound SynchronizedNow().
    inline bool IsSynchronizedClockBound() noexcept {
        return Detail::ApplicationSynchronizedClock.Read != nullptr;
    }

    /// Returns the current application-wide synchronized reading and quality information.
    ///
    /// Application Bootstrap must bind a lifetime-stable clock through BindSynchronizedClock()
    /// before this function is called. Calling it before successful binding is a lifecycle error.
    inline SynchronizedReading SynchronizedNow() noexcept {
        return Detail::ApplicationSynchronizedClock.Read(
            Detail::ApplicationSynchronizedClock.Context
        );
    }

} // ESPressio::Clock
