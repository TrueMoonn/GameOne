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

    #define FPS 5

 private:
    te::network::GameServer _server;
    uint16_t _port;
    std::string _protocol;
    size_t _max_clients;

    std::unordered_map<std::string, size_t> _client_entities;
    std::unordered_map<size_t, te::event::Events> _entity_events;
    float _state_broadcast_timer;

    bool start();
    void stop();
    void update(float delta_time);

    void registerProtocolHandlers();

    void sendConnectionAccepted(const net::Address& client, size_t entity_id);
    void sendErrorTooManyClients(const net::Address& client);
    void sendPong(const net::Address& client);
    void sendDisconnection(const net::Address& client);
    void sendEntityState();  // Broadcast all entity positions

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

    size_t spawnPlayerEntity(const net::Address& client);
    void processEntitiesEvents();  // Process events for each entity

    std::string addressToString(const net::Address& addr) const;
    void append(std::vector<uint8_t>& vec, uint32_t value) const;
    void append(std::vector<uint8_t>& vec, float value) const;
};
