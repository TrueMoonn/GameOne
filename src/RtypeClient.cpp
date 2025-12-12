#include <algorithm>
#include <array>
#include <arpa/inet.h>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <csignal>
#include <array>
#include <vector>
#include <RtypeClient.hpp>
#include <GameTool.hpp>
#include <physic/components/position.hpp>
#include <event/events.hpp>

RtypeClient::RtypeClient(const std::string& protocol, uint16_t port, const std::string& server_ip)
    : Game("./plugins_client")
    , _client(protocol)
    , _server_port(port)
    , _server_ip(server_ip) {
    addConfig("./assets/config/player.toml");
    createComponent("window", 0);
    registerProtocolHandlers();
    createSystem("movement2");
    createSystem("draw");
    createSystem("display");
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

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\n[Client] Disconnecting..." << std::endl;
        return;  // Throw or exit
    }
}

void RtypeClient::run() {
    std::cout << "=== R-Type Client ===" << std::endl;
    std::cout << "Server: " << _server_ip << ":" << _server_port << std::endl;
    std::cout << "=====================" << std::endl;

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    try {
        std::cout << "[Client] Connecting to server..." << std::endl;
        if (!connect(_server_ip, _server_port)) {
            std::cerr << "[Client] Failed to connect to server" << std::endl;
            return;  // (devrait throw peut etre)
        }

        std::cout << "[Client] Connected! Starting game loop..." << std::endl;
        std::cout << "[Client] Press Ctrl+C to disconnect" << std::endl;

        const float deltaTime = 1.0f / 60.0f;  // 60 FPS  (ca sert à rien car update n'utilise pas delta_time mdr)
        auto lastUpdate = std::chrono::steady_clock::now();
        auto lastPing = std::chrono::steady_clock::now();

        while (!isEvent(te::event::System::Closed) && isConnected()) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - lastUpdate).count();

            if (elapsed >= 16) {  // ~60 FPS
                update(deltaTime);
                lastUpdate = now;
            }

           // Send ping every 5 seconds
            auto pingElapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - lastPing).count();
            if (pingElapsed >= 5) {
                std::cout << "[Client] Sending PING..." << std::endl;
                sendPing();
                lastPing = now;
            }

            pollEvent();
            auto events = getEvents();
            if ((events.keys.keys[te::event::Z] || events.keys.keys[te::event::Q] ||
                 events.keys.keys[te::event::S] || events.keys.keys[te::event::D])) {
                sendEvent(events);
            }

            if (_my_client_entity_id.has_value()) {
                emit(_my_client_entity_id);
            }

            runSystems();

            // Sleep a bit bro
            // std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        if (isConnected()) {
            std::cout << "[Client] Disconnecting..." << std::endl;
            disconnect();
        }
        std::cout << "[Client] Goodbye!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[Client] Fatal error: " << e.what() << std::endl;
        return;  // devrait throw
    }
}

std::chrono::_V2::steady_clock::time_point RtypeClient::getPing() {
    return _pingTime;
}

void RtypeClient::setPing(std::chrono::_V2::steady_clock::time_point time) {
    _pingTime = time;
}

void RtypeClient::sendEvent(te::event::Events events) {
    if (!isConnected()) {
        return;
    }

    std::vector<uint8_t> packet;

    packet.push_back(CLIENT_EVENT);
    std::vector<uint8_t> temp = net::PacketSerializer::serialize(events);
    std::copy(temp.begin(), temp.end(), back_inserter(packet));
    _client.send(packet);
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
    _client.registerPacketHandler(CONNECTION_ACCEPTED,
        [this](const std::vector<uint8_t>& data) {
            handleConnectionAccepted(data);
        });

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

    setPing(std::chrono::steady_clock::now());
    packet.push_back(PING);
    _client.send(packet);
}

void RtypeClient::sendPong() {
    std::vector<uint8_t> packet;

    packet.push_back(PONG);
    _client.send(packet);
}

void RtypeClient::handleConnectionAccepted(const std::vector<uint8_t>& data) {
    if (data.size() < 4) {
        std::cerr << "[Client] Invalid CONNECTION_ACCEPTED packet size" << std::endl;
        return;
    }

    uint32_t entity_id = extractUint32(data, 0);
    _my_server_entity_id = entity_id;

    std::cout << "[Client] Connection accepted! Our server entity ID: " << entity_id << std::endl;

    // Create our player entity locally
    uint32_t client_id = next_entity_id++;
    _my_client_entity_id = client_id;
    createEntity(client_id, "player", {0, 0});
    _serverToClientEntityMap[entity_id] = client_id;

    std::cout << "[Client] Created local player entity: server_id=" << entity_id
              << " -> client_id=" << client_id << std::endl;
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
    auto now = std::chrono::steady_clock::now();
    auto pingElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - getPing()).count();
    std::cout << "Latency between sendPing and receive Pong (in ms): "
        << pingElapsed << "\n";
}

void RtypeClient::append(std::vector<uint8_t>& vec, uint32_t value) const {
    std::array<uint8_t, 4> bytes;
    std::memcpy(bytes.data(), &value, 4);
    vec.insert(vec.end(), bytes.begin(), bytes.end());
}

float RtypeClient::extractFloat(const std::vector<uint8_t>& data,
    size_t offset) const {
    if (offset + 4 > data.size())
        return 0;
    float value;
    std::memcpy(&value, data.data() + offset, 4);
    return value;
}

uint32_t RtypeClient::extractUint32(const std::vector<uint8_t>& data,
    size_t offset) const {
    if (offset + 4 > data.size())
        return 0;
    uint32_t value;
    std::memcpy(&value, data.data() + offset, 4);
    return value;
}

void RtypeClient::handleEntityState(const std::vector<uint8_t>& data) {
    if (data.size() < 4)
        return;

    uint32_t entity_count = extractUint32(data, 0);

    size_t expected_size = 4 + (entity_count * 12);
    if (data.size() < expected_size) {
        std::cerr << "[Client] Invalid ENTITY_STATE packet size: got "
                  << data.size() << ", expected " << expected_size
                  << ", entity_count=" << entity_count << std::endl;
        return;
    }

    size_t offset = 4;
    for (uint32_t i = 0; i < entity_count; ++i) {
        uint32_t server_id = extractUint32(data, offset);
        float x = extractFloat(data, offset + 4);
        float y = extractFloat(data, offset + 8);
        offset += 12;

        //  to remove when movments are fixed
        if (_my_server_entity_id.has_value() && server_id == _my_server_entity_id.value()) {
            std::cout << "[Client] Skipping position update for OUR entity (server_id="
                      << server_id << "), using client-side prediction" << std::endl;
            continue;
        }

        auto& positions = getComponent<addon::physic::Position2>();

        uint32_t client_id;
        if (_serverToClientEntityMap.find(server_id) == _serverToClientEntityMap.end()) {
            client_id = next_entity_id++;
            createEntity(client_id, "player", {0, 0});
            _serverToClientEntityMap[server_id] = client_id;
            std::cout << "[Client] Created OTHER player entity: server_id=" << server_id
                      << " -> client_id=" << client_id << " at position ("
                      << x << ", " << y << ")" << std::endl;
        } else {
            client_id = _serverToClientEntityMap[server_id];
        }

        // Update the entity's position
        if (client_id < positions.size() && positions[client_id].has_value()) {
            positions[client_id].value().x = x;
            positions[client_id].value().y = y;
            std::cout << "[Client] Updated OTHER player entity server_id=" << server_id
                      << " (client_id=" << client_id << ") to position ("
                      << x << ", " << y << ")" << std::endl;
        } else {
            std::cout << "[Client] ERROR: Cannot update position for client_id=" << client_id
                      << " (positions.size()=" << positions.size() << ")" << std::endl;
        }
    }
}
