/*
** EPITECH PROJECT, 2025
** GameOne
** File description:
** RtypeServer.cpp
** Copyright [2025] <DeepestDungeonGroup>
*/

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <csignal>
#include <atomic>
#include <Network/PacketSerializer.hpp>
#include <physic/components/position.hpp>
#include <physic/components/velocity.hpp>
#include <physic/systems/movement.hpp>
#include <entity_spec/components/health.hpp>
#include <event/events.hpp>
#include <ECS/Zipper.hpp>

#include <RtypeServer.hpp>

// Global flag for signal handling
static std::atomic<bool> g_running(true);

RtypeServer::RtypeServer(uint16_t port,
                         const std::string& protocol,
                         size_t max_clients)
    : Game("./server/plugins")
    , _server(port, protocol)
    , _port(port)
    , _protocol(protocol)
    , _max_clients(max_clients)
    , _state_broadcast_timer(0.0f) {
    registerProtocolHandlers();

    addConfig("config/entities/server_player.toml");
    addConfig("config/entities/boundaries.toml");

    createSystem("movement2");
    createSystem("bound_hitbox");

    generatePlayerHitbox();
    _server.setClientConnectCallback([this](const net::Address& client) {
        std::cout << "[Server] Network connection from: "
                  << client.getIP() << ":" << client.getPort() << "\n";
    });

    _server.setClientDisconnectCallback([this](const net::Address& client) {
        std::cout << "[Server] Client disconnected: "
                  << client.getIP() << ":" << client.getPort()
                  << " (clients left: " << _server.getClientCount() << ")"
                  << "\n";

        std::string addr_key = addressToString(client);
        auto it = _client_entities.find(addr_key);
        if (it != _client_entities.end()) {
            size_t entity_id = it->second;
            std::cout << "[Server] Removing disconnected client's entity "
                << entity_id << "\n";

            auto player_it = std::find_if(_players.begin(), _players.end(),
                [entity_id](const std::pair<size_t, PLAYER_STATE>& p) {
                    return p.first == entity_id;
                });
            if (player_it != _players.end()) {
                std::cout << "[Server] Removing player " << entity_id <<
                " from players list\n";
                _players.erase(player_it);
            }
            removeEntity(entity_id);
            _client_entities.erase(it);
        }
    });
}

RtypeServer::~RtypeServer() {
    stop();
}

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\n[Server] Shutting down gracefully..." << std::endl;
        g_running = false;
    }
}

void RtypeServer::run() {
    std::cout << "=== R-Type Server ===" << std::endl;
    std::cout << "Port: " << _port << std::endl;
    std::cout << "Protocol: " << _protocol << std::endl;
    std::cout << "Max clients: " << _max_clients << std::endl;
    std::cout << "=====================" << std::endl;

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    try {
        if (!start()) {
            std::cerr << "[Server] Failed to start server on port "
                << _port << std::endl;
            return;
        }

        std::cout << "[Server] Server started successfully!" << std::endl;
        std::cout << "[Server] Waiting for players to connect and ready up..." << std::endl;
        std::cout << "[Server] Press Ctrl+C to stop" << std::endl;

        while (g_running) {
            if (getGameState() == GAME_WAITING) {
                waitGame();
            } else if (getGameState() == IN_GAME) {
                runGame();
            } else if (getGameState() == GAME_ENDED) {
                std::cout << "[Server] Game ended!" << std::endl;
                break;
            }
        }

        std::cout << "[Server] Stopping server..." << std::endl;
        stop();
        std::cout << "[Server] Server stopped. Goodbye!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[Server] Fatal error: " << e.what() << std::endl;
        return;
    }
}

void RtypeServer::waitGame() {
    const float deltaTime = 1.0f / FPS;
    auto lastUpdate = std::chrono::steady_clock::now();

    while (g_running && getGameState() == GAME_WAITING) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastUpdate).count();

        if (elapsed >= (1000.0f / FPS)) {
            update(deltaTime);
            lastUpdate = now;
        }
    }
}

void RtypeServer::runGame() {
    const float deltaTime = 1.0f / FPS;
    auto lastUpdate = std::chrono::steady_clock::now();

    std::cout << "[Server] Game started! Running game loop..." << std::endl;

    while (g_running && getGameState() == IN_GAME) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastUpdate).count();

        if (elapsed >= (1000.0f / FPS)) {
            update(deltaTime);
            lastUpdate = now;
        }

        // Traiter les événements des joueurs et exécuter les systèmes de jeu
        processEntitiesEvents();
        runSystems();
    }

    std::cout << "[Server] Game loop ended." << std::endl;
}

bool RtypeServer::start() {
    return _server.start();
}

void RtypeServer::stop() {
    _server.stop();
}

void RtypeServer::generatePlayerHitbox() {
    size_t left_pannel_e = _next_entity_id++;
    size_t top_pannel_e = _next_entity_id++;
    size_t right_pannel_e = _next_entity_id++;
    size_t bottom_pannel_e = _next_entity_id++;

    createEntity(left_pannel_e, "boundary_left");
    createEntity(top_pannel_e, "boundary_top");
    createEntity(right_pannel_e, "boundary_right");
    createEntity(bottom_pannel_e, "boundary_bottom");
}

void RtypeServer::update(float delta_time) {
    _server.update(delta_time);
    // Broadcast entity state every X ms
    if (getGameState() != IN_GAME)
        return;
    _state_broadcast_timer += delta_time;
    if (_state_broadcast_timer >= 0.01f) {
        sendPlayersStates();
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

    _server.registerPacketHandler(CLIENT_EVENT,
        [this](const std::vector<uint8_t>& data, const net::Address& sender) {
            handleUserEvent(data, sender);
        });

    _server.registerPacketHandler(WANT_START,
        [this](const std::vector<uint8_t>& data, const net::Address& sender) {
            handleWantStart(data, sender);
        });
}

void RtypeServer::sendErrorTooManyClients(const net::Address& client) {
    std::vector<uint8_t> packet;

    packet.push_back(ERROR_TOO_MANY_CLIENTS);
    _server.sendTo(client, packet);
}

void RtypeServer::sendConnectionAccepted(const net::Address& client,
    size_t entity_id) {
    std::vector<uint8_t> packet;

    packet.push_back(CONNECTION_ACCEPTED);
    append(packet, static_cast<uint32_t>(entity_id));
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

    sendConnectionAccepted(sender, entity_id);
}

void RtypeServer::handleDisconnection(const std::vector<uint8_t>& data,
                                       const net::Address& sender) {
    std::cout << "[Server] Client disconnected: " << sender.getIP()
              << ":" << sender.getPort() << std::endl;

    std::string addr_key = addressToString(sender);
    auto it = _client_entities.find(addr_key);
    if (it != _client_entities.end()) {
        size_t entity_id = it->second;
        std::cout << "[Server] Removing entity " << entity_id << "\n";

        auto player_it = std::find_if(_players.begin(), _players.end(),
            [entity_id](const std::pair<size_t, PLAYER_STATE>& p) {
                return p.first == entity_id;
            });
        if (player_it != _players.end()) {
            std::cout << "[Server] Removing player " << entity_id << " from players list\n";
            _players.erase(player_it);
        }
        removeEntity(entity_id);
        _client_entities.erase(it);
    }
}

void RtypeServer::handlePing(const std::vector<uint8_t>& data,
    const net::Address& sender) {
    std::cout << "[Server] Ping from " << sender.getIP() << ":"
              << sender.getPort() << " - sending pong" << "\n";
    sendPong(sender);
}

void RtypeServer::handlePong(const std::vector<uint8_t>& data,
    const net::Address& sender) {
    std::cout << "[Server] Pong from " << sender.getIP() << ":"
              << sender.getPort() << "\n";
}

void RtypeServer::handleUserEvent(const std::vector<uint8_t>& data,
    const net::Address& sender) {
    te::event::Events events;
    if (!net::PacketSerializer::deserialize(data, events)) {
        std::cerr << "[Server] Failed to deserialize events (size: "
            << data.size() << ", expected: "
            << sizeof(te::event::Events) << ")" << "\n";
        return;
    }

    std::string addr_key = addressToString(sender);
    auto it = _client_entities.find(addr_key);
    if (it == _client_entities.end()) {
        std::cerr << "[Server] Received event from unknown client: "
                  << sender.getIP() << ":" << sender.getPort() << "\n";
        return;
    }

    size_t entity_id = it->second;
    _entity_events[entity_id] = events;
}

void RtypeServer::processEntitiesEvents() {
    auto& velocities = getComponent<addon::physic::Velocity2>();
    for (const auto& [addr, entity_id] : _client_entities) {
        if (entity_id < velocities.size()
            && velocities[entity_id].has_value()) {
            auto& vel = velocities[entity_id].value();
            vel.x = 0.0;
            vel.y = 0.0;
        }
    }

    for (auto& [entity_id, events] : _entity_events) {
        setEvents(events);
        emit(entity_id);
    }
    _entity_events.clear();
}

void RtypeServer::append(std::vector<uint8_t>& vec, uint32_t value) const {
    std::array<uint8_t, sizeof(uint32_t)> bytes;
    std::memcpy(bytes.data(), &value, sizeof(uint32_t));
    vec.insert(vec.end(), bytes.begin(), bytes.end());
}

void RtypeServer::append(std::vector<uint8_t>& vec, int64_t value) const {
    std::array<uint8_t, sizeof(int64_t)> bytes;
    std::memcpy(bytes.data(), &value, sizeof(int64_t));
    vec.insert(vec.end(), bytes.begin(), bytes.end());
}

void RtypeServer::append(std::vector<uint8_t>& vec, float value) const {
    std::array<uint8_t, sizeof(float)> bytes;
    std::memcpy(bytes.data(), &value, sizeof(float));
    vec.insert(vec.end(), bytes.begin(), bytes.end());
}

std::string RtypeServer::addressToString(const net::Address& addr) const {
    std::ostringstream oss;
    oss << addr.getIP() << ":" << addr.getPort();
    return oss.str();
}

size_t RtypeServer::spawnEnnemyEntity(const net::Address& client) {
    size_t entity = _next_entity_id++;

    createComponent("position2", entity);
    createComponent("velocity2", entity);
    createComponent("health", entity);

    std::string addr_key = addressToString(client);
    _client_entities[addr_key] = entity;
    return entity;
}

size_t RtypeServer::spawnPlayerEntity(const net::Address& client) {
    size_t entity = _next_entity_id++;

    createEntity(entity, "player");

    std::string addr_key = addressToString(client);
    _client_entities[addr_key] = entity;
    _players.push_back({entity, WAIT_GAME});  // Le joueur est en attente
    return entity;
}

void RtypeServer::sendEntityState() {
    if (_server.getClientCount() == 0)
        return;

    std::vector<uint8_t> packet;
    packet.push_back(ENTITIES_STATES);

    auto& positions = getComponent<addon::physic::Position2>();

    for (auto &&[entity, pos] : ECS::IndexedZipper(positions)) {
        auto isPlayer = std::find_if(_players.begin(), _players.end(),
            [entity](const std::pair<size_t, PLAYER_STATE>& p) {
                return p.first == entity;
            });
        if (isPlayer != _players.end())
            continue;

        append(packet, static_cast<uint32_t>(entity));
        append(packet, pos.x);
        append(packet, pos.y);

        std::cout << "  - Entity " << entity << " at (" << pos.x << ", "
            << pos.y << ")" << "\n";
    }
    _server.broadcastToAll(packet);
}

void RtypeServer::sendPlayersStates() {
    if (_server.getClientCount() == 0)
        return;

    std::vector<uint8_t> packet;
    packet.push_back(PLAYERS_STATES);

    auto& positions = getComponent<addon::physic::Position2>();
    auto& healths = getComponent<addon::eSpec::Health>();

    for (auto &&[entity, pos, hp] : ECS::IndexedZipper(positions, healths)) {
        auto isPlayer = std::find_if(_players.begin(), _players.end(),
            [entity](const std::pair<size_t, PLAYER_STATE>& p) {
                return p.first == entity;
            });
        if (isPlayer == _players.end())
            continue;

        append(packet, static_cast<uint32_t>(entity));
        append(packet, pos.x);
        append(packet, pos.y);
        append(packet, hp.amount);

        std::cout << "  - Player " << entity << " at (" << pos.x << ", "
            << pos.y << ", " << hp.amount << ")" << "\n";
    }

    _server.broadcastToAll(packet);
}

void RtypeServer::sendGameStart() {
    std::vector<uint8_t> packet;
    packet.push_back(GAME_START);

    std::cout << "[Server] Broadcasting GAME_START to all clients\n";
    _server.broadcastToAll(packet);
}

void RtypeServer::handleWantStart(const std::vector<uint8_t>& data,
    const net::Address& sender) {
    if (getGameState() != GAME_WAITING) {
        std::cout << "[Server] Ignoring WANT_START from " << sender.getIP()
                  << ":" << sender.getPort() << " - game already started\n";
        return;
    }

    std::string addr_key = addressToString(sender);
    auto it = _client_entities.find(addr_key);
    if (it == _client_entities.end()) {
        std::cerr << "[Server] Received WANT_START from unknown client: "
                  << sender.getIP() << ":" << sender.getPort() << "\n";
        return;
    }

    size_t entity_id = it->second;

    auto player_it = std::find_if(_players.begin(), _players.end(),
        [entity_id](const std::pair<size_t, PLAYER_STATE>& p) {
            return p.first == entity_id;
        });

    if (player_it != _players.end()) {
        if (player_it->second == WAIT_GAME) {
            player_it->second = READY_TO_START;
            std::cout << "[Server] Player " << entity_id << " is ready to start ("
                      << sender.getIP() << ":" << sender.getPort() << ")\n";

            bool all_ready = std::all_of(_players.begin(), _players.end(),
                [](const std::pair<size_t, PLAYER_STATE>& p) {
                    return p.second == READY_TO_START;
                });

            if (all_ready && !_players.empty()) {
                std::cout << "[Server] All " << _players.size()
                          << " players are ready! Starting game...\n";

                for (auto& player : _players) {
                    player.second = PLAYER_ALIVE;
                }

                setGameState(IN_GAME);
                sendGameStart();
            } else {
                size_t ready_count = std::count_if(_players.begin(), _players.end(),
                    [](const std::pair<size_t, PLAYER_STATE>& p) {
                        return p.second == READY_TO_START;
                    });
                std::cout << "[Server] Waiting for players... (" << ready_count
                          << "/" << _players.size() << " ready)\n";
            }
        } else {
            std::cout << "[Server] Player " << entity_id
                      << " already marked as ready or in different state\n";
        }
    }
}

