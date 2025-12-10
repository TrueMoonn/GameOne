#include <array>
#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <ECS/Registry.hpp>
#include <RtypeServer.hpp>

RtypeServer::RtypeServer(ECS::Registry& ecs, uint16_t port,
                         const std::string& protocol,
                         size_t max_clients)
    : _server(ecs, port, protocol)
    , _max_clients(max_clients) {
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

    std::cout << "[Server] Client connected: " << sender.getIP() << ":"
              << sender.getPort() << " (" << (_server.getClientCount())
              << "/" << _max_clients << ")" << std::endl;
}

void RtypeServer::handleDisconnection(const std::vector<uint8_t>& data,
                                       const net::Address& sender) {
    std::cout << "[Server] Client disconnected: " << sender.getIP()
              << ":" << sender.getPort() << std::endl;
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
    std::array<uint8_t, 4> bytes;
    std::memcpy(bytes.data(), &value, 4);
    vec.insert(vec.end(), bytes.begin(), bytes.end());
}
