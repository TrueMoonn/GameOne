#include <array>
#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <ECS/Registry.hpp>
#include <RtypeClient.hpp>

RtypeClient::RtypeClient(ECS::Registry& ecs, const std::string& protocol)
    : _client(ecs, protocol) {
    registerProtocolHandlers();

    _client.setConnectCallback([this]() {
        std::cout << "[Client] Network connection established, sending CONNECTION_REQUEST..."
                  << std::endl;
        sendConnectionRequest();
    });

    _client.setDisconnectCallback([this]() {
        std::cout << "[Client] Disconnected from server" << std::endl;
        // TODO(Pierre): Cleanup local entities, return to menu, etc.
    });
}

RtypeClient::~RtypeClient() {
    if (_client.isConnected()) {
        disconnect();
    }
}

bool RtypeClient::connect(const std::string& ip, uint16_t port) {
    // Connection is established, and setConnectCallback will send CONNECTION_REQUEST
    return _client.connect(ip, port);
}

void RtypeClient::disconnect() {
    if (_client.isConnected()) {
        sendDisconnection();
    }
    _client.disconnect();
}

void RtypeClient::update(float delta_time) {
    _client.receive(0, 10);
    _client.update(delta_time);
}

void RtypeClient::registerProtocolHandlers() {
    _client.registerPacketHandler(DISCONNECTION,
        [this](const std::vector<uint8_t>& data) {
            handleDisconnection(data);
        });

    _client.registerPacketHandler(ERROR_TOO_MANY_CLIENTS,
        [this](const std::vector<uint8_t>& data) {
            handleServerFull(data);
        });

    _client.registerPacketHandler(PING,
        [this](const std::vector<uint8_t>& data) {
            handlePing(data);
        });

    _client.registerPacketHandler(PONG,
        [this](const std::vector<uint8_t>& data) {
            handlePong(data);
        });
}

// === SEND FUNCTIONS ===

void RtypeClient::sendConnectionRequest() {
    std::vector<uint8_t> packet;

    packet.push_back(CONNECTION_REQUEST);
    _client.send(packet);
}

void RtypeClient::sendDisconnection() {
    std::vector<uint8_t> packet;

    packet.push_back(DISCONNECTION);
    _client.send(packet);
}

void RtypeClient::sendPing() {
    std::vector<uint8_t> packet;

    packet.push_back(PING);
    _client.send(packet);
}

void RtypeClient::sendPong() {
    std::vector<uint8_t> packet;

    packet.push_back(PONG);
    _client.send(packet);
}

void RtypeClient::handleDisconnection(const std::vector<uint8_t>& data) {
    std::cout << "[Client] Server disconnected us" << std::endl;
}

void RtypeClient::handleServerFull(const std::vector<uint8_t>& data) {
    std::cout << "[Client] Server full!" << std::endl;
    disconnect();  //  On pourrait le laisser attendre mais pour le moment
}

void RtypeClient::handlePing(const std::vector<uint8_t>& data) {
    std::cout << "[Client] Ping received, sending pong..." << std::endl;
    sendPong();
}

void RtypeClient::handlePong(const std::vector<uint8_t>& data) {
    std::cout << "[Client] Pong received" << std::endl;
}

void RtypeClient::append(std::vector<uint8_t>& vec, uint32_t value) const {
    std::array<uint8_t, 4> bytes;
    std::memcpy(bytes.data(), &value, 4);
    vec.insert(vec.end(), bytes.begin(), bytes.end());
}
