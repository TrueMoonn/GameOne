#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <network/GameServer.hpp>
#include "Protocol.hpp"

class RtypeServer {
 public:
    RtypeServer(ECS::Registry& ecs, uint16_t port,
                const std::string& protocol = "UDP",
                size_t max_clients = 4);
    ~RtypeServer();

    bool start();
    void stop();
    void update(float delta_time);

    te::network::GameServer& getServer() { return _server; }
    size_t getClientCount() const { return _server.getClientCount(); }

 private:
    te::network::GameServer _server;
    ECS::Registry& _ecs;
    size_t _max_clients;
    std::unordered_map<std::string, size_t> _client_entities;  // address -> entity index
    float _state_broadcast_timer;

    void registerProtocolHandlers();

    void sendErrorTooManyClients(const net::Address& client);
    void sendPong(const net::Address& client);
    void sendDisconnection(const net::Address& client);
    void sendEntityState();  // Broadcast all entity positions

    void handleConnectionRequest(const std::vector<uint8_t>& data,
                                  const net::Address& sender);
    void handleDisconnection(const std::vector<uint8_t>& data,
                            const net::Address& sender);
    void handlePing(const std::vector<uint8_t>& data, const net::Address& sender);
    void handlePong(const std::vector<uint8_t>& data, const net::Address& sender);

    size_t spawnPlayerEntity(const net::Address& client);
    void updateMovement(float delta_time);

    std::string addressToString(const net::Address& addr) const;
    void append(std::vector<uint8_t>& vec, uint32_t value) const;
};
