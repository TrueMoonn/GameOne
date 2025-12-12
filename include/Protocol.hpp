#pragma once

#include <cstdint>

// Protocol codes - using enum instead of enum class to avoid casting
enum ProtocolCode : uint8_t {
    CONNECTION_REQUEST = 1,
    DISCONNECTION = 2,
    ERROR_TOO_MANY_CLIENTS = 3,
    CONNECTION_ACCEPTED = 4,  // Server → Client: [uint32_t entity_id]
    PING = 6,
    PONG = 7,
    CLIENT_EVENT = 50,
    ENTITY_STATE = 51  // Broadcast entity positions [uint32_t id, uint32_t x, uint32_t y, ...]
};
