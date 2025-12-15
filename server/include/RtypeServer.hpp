/*
** EPITECH PROJECT, 2025
** GameOne
** File description:
** RtypeServer.hpp
** Copyright [2025] <DeepestDungeonGroup>
*/

#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <utility>
#include <unordered_map>
#include <network/GameServer.hpp>
#include <GameTool.hpp>
#include <Game.hpp>
#include <Protocol.hpp>

class RtypeServer : public Game {
 public:
    RtypeServer(uint16_t port,
                const std::string& protocol = "UDP",
                size_t max_clients = 4);
    ~RtypeServer();

    void run();

    te::network::GameServer& getServer() { return _server; }
    size_t getClientCount() const { return _server.getClientCount(); }

    #define FPS 60
    #define TIME_ENNEMY_SPAWN 15

 private:
    te::network::GameServer _server;
    uint16_t _port;
    std::string _protocol;
    size_t _max_clients;

    // next entities
    size_t _nextMapE = EntityField::MAP_BEGIN;
    size_t _nextPlayerE = EntityField::PLAYER_BEGIN;
    size_t _nextEnnemyE = EntityField::ENEMIES_BEGIN;
    size_t _nextProjectileE = EntityField::PROJECTILES_BEGIN;

    std::unordered_map<std::string, size_t> _client_entities;
    std::unordered_map<size_t, te::event::Events> _entity_events;
    float _state_broadcast_timer;
    std::vector<std::pair<size_t, PLAYER_STATE>> _players;

    bool start();
    void stop();
    void update(float delta_time);

    void waitGame();  // Boucle d'attente (GAME_WAITING)
    void runGame();   // Boucle de jeu principale (IN_GAME)

    void registerProtocolHandlers();
    void generatePlayerHitbox();

    void sendConnectionAccepted(const net::Address& client, size_t entity_id);
    void sendErrorTooManyClients(const net::Address& client);
    void sendPong(const net::Address& client);
    void sendDisconnection(const net::Address& client);
    void sendEntityState();  // Broadcast all entity positions
    void sendPlayersStates();  // Broadcast all entity positions
    void sendGameStart();
    void sendEnnemySpawn(size_t waveNb);

    void handleConnectionRequest(const std::vector<uint8_t>& data,
                                  const net::Address& sender);
    void handleDisconnection(const std::vector<uint8_t>& data,
                            const net::Address& sender);
    void handlePing(const std::vector<uint8_t>& data,
      const net::Address& sender);
    void handlePong(const std::vector<uint8_t>& data,
      const net::Address& sender);

    void handleUserEvent(const std::vector<uint8_t>& data,
      const net::Address& sender);
    void handleWantStart(const std::vector<uint8_t>& data,
      const net::Address& sender);

    void handleShoot(const std::vector<uint8_t>& data,
      const net::Address& sender);

    size_t spawnPlayerEntity(const net::Address& client);
    void spawnEnnemyEntity(size_t waveNb);

    void processEntitiesEvents();  // Process events for each entity

    std::string addressToString(const net::Address& addr) const;
    void append(std::vector<uint8_t>& vec, uint32_t value) const;
    void append(std::vector<uint8_t>& vec, size_t value) const;
    void append(std::vector<uint8_t>& vec, int64_t value) const;
    void append(std::vector<uint8_t>& vec, float value) const;
};
