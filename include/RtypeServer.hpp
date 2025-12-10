#pragma once

#include <string>
#include <cstdint>
#include <vector>
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
    size_t _max_clients;

    void registerProtocolHandlers();

    void sendErrorTooManyClients(const net::Address& client);
    void sendPong(const net::Address& client);
    void sendDisconnection(const net::Address& client);

    void handleConnectionRequest(const std::vector<uint8_t>& data,
                                  const net::Address& sender);
    void handleDisconnection(const std::vector<uint8_t>& data,
                            const net::Address& sender);
    void handlePing(const std::vector<uint8_t>& data, const net::Address& sender);
    void handlePong(const std::vector<uint8_t>& data, const net::Address& sender);

    void append(std::vector<uint8_t>& vec, uint32_t value) const;
};
