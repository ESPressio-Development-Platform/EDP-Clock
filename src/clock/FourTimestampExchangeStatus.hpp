#pragma once

#include <cstdint>

namespace ESPressio::Clock {

    /// Result status produced while reducing a four-timestamp time-transfer exchange.
    enum class FourTimestampExchangeStatus : std::uint8_t {
        Accepted = 0,
        RejectedLocalTimestampOrder = 1,
        RejectedRemoteTimestampOrder = 2,
        RejectedImpossiblePathDelay = 3,
        RejectedReferenceOverflow = 4
    };

} // ESPressio::Clock
