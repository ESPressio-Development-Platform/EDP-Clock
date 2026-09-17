#include "ESPressio_Clock.hpp"

namespace ESPressio::Clock::Tests::Monotonic {

    /// Reads the application-wide monotonic clock from a separate translation unit.
    MonotonicTimestamp ReadFromAnotherTranslationUnit() noexcept {
        return MonotonicNow();
    }

} // ESPressio::Clock::Tests::Monotonic
