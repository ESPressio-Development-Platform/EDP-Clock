#include <ESPressio_Clock.hpp>

ESPressio::Clock::SynchronizedReading ReadSynchronizedFromAnotherTranslationUnit() noexcept {
    return ESPressio::Clock::SynchronizedNow();
}
