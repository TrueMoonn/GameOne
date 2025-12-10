#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <network/GameClient.hpp>
#include "Protocol.hpp"

class RtypeClient {
 public:
    explicit RtypeClient(ECS::Registry& ecs, const std::string& protocol = "UDP");
    ~RtypeClient();

    bool connect(const std::string& ip, uint16_t port);
    void disconnect();

    void update(float delta_time);

    bool isConnected() const { return _client.isConnected(); }

    te::network::GameClient& getClient() { return _client; }

    // Public API for testing/debugging
    void sendPing();

 private:
    te::network::GameClient _client;

    void registerProtocolHandlers();

    void sendConnectionRequest();
    void sendDisconnection();
    void sendPong();

    void handleDisconnection(const std::vector<uint8_t>& data);
    void handleServerFull(const std::vector<uint8_t>& data);
    void handlePing(const std::vector<uint8_t>& data);
    void handlePong(const std::vector<uint8_t>& data);

    void append(std::vector<uint8_t>& vec, uint32_t value) const;
};
