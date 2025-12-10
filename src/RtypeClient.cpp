#include <algorithm>
#include <array>
#include <arpa/inet.h>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <RtypeClient.hpp>
#include <physic/components/position.hpp>

RtypeClient::RtypeClient(ECS::Registry& ecs, const std::string& protocol)
    : _client(ecs, protocol)
    , _ecs(ecs) {
    _ecs.registerComponent<addon::physic::Position2>();

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

    _client.registerPacketHandler(ENTITY_STATE,
        [this](const std::vector<uint8_t>& data) {
            handleEntityState(data);
        });
}

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

uint32_t RtypeClient::extractUint32(const std::vector<uint8_t>& data, size_t offset) const {
    if (offset + 4 > data.size())
        return 0;
    uint32_t value;
    std::memcpy(&value, data.data() + offset, 4);
    return value;
}

void RtypeClient::handleEntityState(const std::vector<uint8_t>& data) {
    if (data.size() < 4)
        return;

    // Extract entity count
    uint32_t entity_count = extractUint32(data, 0);

    size_t expected_size = 4 + (entity_count * 12);
    if (data.size() < expected_size) {
        std::cerr << "[Client] Invalid ENTITY_STATE packet size: got "
                  << data.size() << ", expected " << expected_size
                  << ", entity_count=" << entity_count << std::endl;
        return;
    }

    // Log every 10 packets (not every 10 entity updates)
    static int packet_counter = 0;
    bool should_log = (packet_counter++ % 10 == 0);

    // Process each entity
    size_t offset = 4;
    for (uint32_t i = 0; i < entity_count; ++i) {
        uint32_t id = extractUint32(data, offset);
        uint32_t x = extractUint32(data, offset + 4);
        uint32_t y = extractUint32(data, offset + 8);
        offset += 12;

        auto& positions = _ecs.getComponents<addon::physic::Position2>();

        if (id >= positions.size() || !positions[id].has_value()) {
            _ecs.addComponent(id, addon::physic::Position2(
                static_cast<float>(x), static_cast<float>(y)));
            std::cout << "[Client] Created entity " << id << " at position ("
                      << x << ", " << y << ")" << std::endl;
        } else {
            positions[id].value().x = static_cast<float>(x);
            positions[id].value().y = static_cast<float>(y);
            if (should_log) {
                std::cout << "[Client] Updated entity " << id << " to position ("
                          << x << ", " << y << ") from server" << std::endl;
            }
        }
    }
}
