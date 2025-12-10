#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <RtypeServer.hpp>
#include <physic/components/position.hpp>
#include <physic/components/velocity.hpp>
#include <physic/systems/movement.hpp>

RtypeServer::RtypeServer(ECS::Registry& ecs, uint16_t port,
                         const std::string& protocol,
                         size_t max_clients)
    : _server(ecs, port, protocol)
    , _ecs(ecs)
    , _max_clients(max_clients)
    , _state_broadcast_timer(0.0f) {
    // Register ECS components from TrueEngine
    _ecs.registerComponent<addon::physic::Position2>();
    _ecs.registerComponent<addon::physic::Velocity2>();

    // Register movement system
    _ecs.addSystem(addon::physic::movement2_sys);

    registerProtocolHandlers();

    _server.setClientConnectCallback([this](const net::Address& client) {
        std::cout << "[Server] Network connection from: "
                  << client.getIP() << ":" << client.getPort() << std::endl;
    });

    _server.setClientDisconnectCallback([this](const net::Address& client) {
        std::cout << "[Server] Client disconnected: "
                  << client.getIP() << ":" << client.getPort()
                  << " (clients left: " << _server.getClientCount() << ")"
                  << std::endl;

        // TODO(Pierre): Cleanup player entities in ECS
        // TODO(Pierre): Broadcast to other clients that this player left
    });
}

RtypeServer::~RtypeServer() {
    stop();
}

bool RtypeServer::start() {
    return _server.start();
}

void RtypeServer::stop() {
    _server.stop();
}

void RtypeServer::update(float delta_time) {
    _server.receive(0, 100);
    _server.update(delta_time);

    // Update entity movement every frame
    updateMovement(delta_time);

    // Broadcast entity state every 100ms
    _state_broadcast_timer += delta_time;
    if (_state_broadcast_timer >= 0.1f) {
        sendEntityState();
        _state_broadcast_timer = 0.0f;
    }
}

void RtypeServer::registerProtocolHandlers() {
    _server.registerPacketHandler(CONNECTION_REQUEST,
        [this](const std::vector<uint8_t>& data, const net::Address& sender) {
            handleConnectionRequest(data, sender);
        });

    _server.registerPacketHandler(DISCONNECTION,
        [this](const std::vector<uint8_t>& data, const net::Address& sender) {
            handleDisconnection(data, sender);
        });

    _server.registerPacketHandler(PING,
        [this](const std::vector<uint8_t>& data, const net::Address& sender) {
            handlePing(data, sender);
        });

    _server.registerPacketHandler(PONG,
        [this](const std::vector<uint8_t>& data, const net::Address& sender) {
            handlePong(data, sender);
        });
}

void RtypeServer::sendErrorTooManyClients(const net::Address& client) {
    std::vector<uint8_t> packet;

    packet.push_back(ERROR_TOO_MANY_CLIENTS);
    _server.sendTo(client, packet);
}

void RtypeServer::sendPong(const net::Address& client) {
    std::vector<uint8_t> packet;

    packet.push_back(PONG);
    _server.sendTo(client, packet);
}

void RtypeServer::sendDisconnection(const net::Address& client) {
    std::vector<uint8_t> packet;

    packet.push_back(DISCONNECTION);
    _server.sendTo(client, packet);
}

void RtypeServer::handleConnectionRequest(const std::vector<uint8_t>& data,
                                           const net::Address& sender) {
    if (_server.getClientCount() > _max_clients) {
        std::cout << "[Server] Too many clients! Rejecting "
                  << sender.getIP() << ":" << sender.getPort() << std::endl;
        sendErrorTooManyClients(sender);
        return;
    }

    size_t entity_id = spawnPlayerEntity(sender);
    std::cout << "[Server] Client connected: " << sender.getIP() << ":"
              << sender.getPort() << " (" << (_server.getClientCount())
              << "/" << _max_clients << ") - Entity ID: " << entity_id << std::endl;
}

void RtypeServer::handleDisconnection(const std::vector<uint8_t>& data,
                                       const net::Address& sender) {
    std::cout << "[Server] Client disconnected: " << sender.getIP()
              << ":" << sender.getPort() << std::endl;

    // Remove entity from tracking
    std::string addr_key = addressToString(sender);
    auto it = _client_entities.find(addr_key);
    if (it != _client_entities.end()) {
        size_t entity_id = it->second;
        std::cout << "[Server] Removing entity " << entity_id << std::endl;
        _ecs.killEntity(entity_id);
        _client_entities.erase(it);
    }
}

void RtypeServer::handlePing(const std::vector<uint8_t>& data, const net::Address& sender) {
    std::cout << "[Server] Ping from " << sender.getIP() << ":"
              << sender.getPort() << " - sending pong" << std::endl;
    sendPong(sender);
}

void RtypeServer::handlePong(const std::vector<uint8_t>& data, const net::Address& sender) {
    std::cout << "[Server] Pong from " << sender.getIP() << ":"
              << sender.getPort() << std::endl;
}

void RtypeServer::append(std::vector<uint8_t>& vec, uint32_t value) const {
    // Protocol already handles endianness conversion, just copy bytes
    std::array<uint8_t, 4> bytes;
    std::memcpy(bytes.data(), &value, 4);
    vec.insert(vec.end(), bytes.begin(), bytes.end());
}


std::string RtypeServer::addressToString(const net::Address& addr) const {
    std::ostringstream oss;
    oss << addr.getIP() << ":" << addr.getPort();
    return oss.str();
}

size_t RtypeServer::spawnPlayerEntity(const net::Address& client) {
    // Create entity at position (0, 0) with a velocity for automatic movement
    static size_t next_entity_id = 0;
    size_t entity = next_entity_id++;

    _ecs.addComponent(entity, addon::physic::Position2(0.0f, 0.0f));
    _ecs.addComponent(entity, addon::physic::Velocity2(10.0f, 5.0f));

    std::string addr_key = addressToString(client);
    _client_entities[addr_key] = entity;
    return entity;
}

void RtypeServer::updateMovement(float delta_time) {
    // Use TrueEngine movement system
    addon::physic::movement2_sys(_ecs);

    // Apply screen wrapping (800x600)
    auto& positions = _ecs.getComponents<addon::physic::Position2>();
    for (size_t i = 0; i < positions.size(); ++i) {
        if (positions[i].has_value()) {
            auto& pos = positions[i].value();
            if (pos.x > 800.0f)
                pos.x = 0.0f;
            if (pos.y > 600.0f)
                pos.y = 0.0f;
        }
    }
}

void RtypeServer::sendEntityState() {
    if (_server.getClientCount() == 0) return;

    std::vector<uint8_t> packet;
    packet.push_back(ENTITY_STATE);

    // Get all entities with Position2
    auto& positions = _ecs.getComponents<addon::physic::Position2>();
    uint32_t entity_count = 0;
    for (size_t i = 0; i < positions.size(); ++i) {
        if (positions[i].has_value()) {
            entity_count++;
        }
    }

    append(packet, entity_count);

    // Add each entity's data: [id, x, y]
    static int broadcast_counter = 0;
    bool should_log = (broadcast_counter++ % 10 == 0);  // Log every 10 broadcasts (1 second)

    if (should_log) {
        std::cout << "[Server] Broadcasting ENTITY_STATE with " << entity_count
                  << " entities:" << std::endl;
    }

    for (size_t i = 0; i < positions.size(); ++i) {
        if (positions[i].has_value()) {
            const auto& pos = positions[i].value();

            // Convert float positions to uint32_t for network protocol
            append(packet, static_cast<uint32_t>(i));
            append(packet, static_cast<uint32_t>(pos.x));
            append(packet, static_cast<uint32_t>(pos.y));

            if (should_log) {
                std::cout << "  - Entity " << i << " at (" << pos.x << ", "
                          << pos.y << ")" << std::endl;
            }
        }
    }

    _server.broadcastToAll(packet);
}
