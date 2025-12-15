/*
** EPITECH PROJECT, 2025
** GameOne
** File description:
** Protocol.hpp
** Copyright [2025] <DeepestDungeonGroup>
*/

#pragma once

#include <cstdint>

enum PLAYER_STATE : uint8_t {
    WAIT_GAME = 0,
    READY_TO_START = 1,
    PLAYER_ALIVE = 2,
    PLAYER_DEAD = 3
};

// Protocol codes - using enum instead of enum class to avoid casting
enum ProtocolCode : uint8_t {
    CONNECTION_REQUEST = 1,
    DISCONNECTION = 2,
    ERROR_TOO_MANY_CLIENTS = 3,
    CONNECTION_ACCEPTED = 4,  // Server → Client: [uint32_t entity_id]
    PING = 6,
    PONG = 7,
    WANT_START = 35,  // client send
    GAME_START = 36,  // server send
    GAME_ENDED = 49,  // Server send
    CLIENT_EVENT = 50,
    PLAYERS_STATES = 51,  // Broadcast players positions
    ENTITIES_STATES = 54   // Broadcast entities positions (float)
};
