#pragma once

#include <cstdint>

// Protocol codes - using enum instead of enum class to avoid casting
enum ProtocolCode : uint8_t {
    CONNECTION_REQUEST = 1,
    DISCONNECTION = 2,
    ERROR_TOO_MANY_CLIENTS = 3,
    PING = 6,
    PONG = 7
};
