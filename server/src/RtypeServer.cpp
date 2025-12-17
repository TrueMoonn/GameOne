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
#include <utility>
#include <sstream>
#include <csignal>
#include <atomic>
#include <Network/PacketSerializer.hpp>
#include <physic/components/position.hpp>
#include <physic/components/velocity.hpp>
#include <physic/systems/movement.hpp>
#include <entity_spec/components/health.hpp>
#include <interaction/components/player.hpp>
#include <event/events.hpp>
#include <ECS/Zipper.hpp>

#include <waves.hpp>
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

    addConfig("config/entities/player.toml");
    addConfig("config/entities/enemy1.toml");
    addConfig("config/entities/enemy2.toml");
    addConfig("config/entities/enemy3.toml");
    addConfig("config/entities/boundaries.toml");

    createSystem("apply_pattern");
    createSystem("movement2");
    createSystem("bound_hitbox");
    createSystem("deal_damage");
    createSystem("apply_fragile");
    createSystem("kill_entity");

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
        std::cout << "[Server] Waiting for players to connect and ready up..."
            << std::endl;
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
    te::Timestamp updateTimer(UPDATES_TIME / 1000.0f);

    while (g_running && getGameState() == GAME_WAITING) {
        if (updateTimer.checkDelay()) {
            update(0.0f);
        }
    }
}

void RtypeServer::runGame() {
    te::Timestamp updateTimer(UPDATES_TIME / 1000.0f);
    te::Timestamp ennemyWaveTimer(static_cast<float>(TIME_ENNEMY_SPAWN));
    te::Timestamp playersUpdateTimer(REFRESH_PLAYERS_TIME / 1000.0f);
    te::Timestamp ennemiesUpdateTimer(REFRESH_ENNEMIES_TIME / 1000.0f);
    te::Timestamp projectileUpdateTimer(REFRESH_PROJECTILE_TIME / 1000.0f);
    uint waveNb = 0;

    std::cout << "[Server] Game started! Running game loop..." << std::endl;

    _nextMapE = createBoundaries(_nextMapE);
    spawnEnnemyEntity(waveNb);

    while (g_running && getGameState() == IN_GAME) {
        if (getGameState() != IN_GAME)
            break;

        if (updateTimer.checkDelay()) {
            update(0.0f);
            processEntitiesEvents();
            runSystems();
        }

        if (playersUpdateTimer.checkDelay()) {
            sendPlayersData();
        }

        if (ennemiesUpdateTimer.checkDelay()) {
            sendEnnemiesData();
        }

        if (projectileUpdateTimer.checkDelay()) {
            sendProjectilesData();
        }

        if (ennemyWaveTimer.checkDelay()) {
            spawnEnnemyEntity(waveNb);
            waveNb++;
            if (waveNb >= NB_WAVES)
                waveNb = 0;
        }
    }
    std::cout << "[Server] Game loop ended." << std::endl;
}

bool RtypeServer::start() {
    return _server.start();
}

void RtypeServer::stop() {
    _server.stop();
}

void RtypeServer::update(float delta_time) {
    _server.update(delta_time);
    if (getGameState() != IN_GAME)
        return;
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
    _server.registerPacketHandler(PLAYER_SHOT,
        [this](const std::vector<uint8_t>& data, const net::Address& sender) {
            handleShoot(data, sender);
        });
}

void RtypeServer::sendErrorTooManyClients(const net::Address& client) {
    std::vector<uint8_t> packet;

    packet.push_back(ERROR_TOO_MANY_CLIENTS);
    _server.queuePacket(client, packet);
}

void RtypeServer::sendConnectionAccepted(const net::Address& client,
    size_t entity_id) {
    std::vector<uint8_t> packet;

    packet.push_back(CONNECTION_ACCEPTED);
    append(packet, entity_id);
    _server.queuePacket(client, packet);
}

void RtypeServer::sendPong(const net::Address& client) {
    std::vector<uint8_t> packet;

    packet.push_back(PONG);
    _server.queuePacket(client, packet);
}

void RtypeServer::sendDisconnection(const net::Address& client) {
    std::vector<uint8_t> packet;

    packet.push_back(DISCONNECTION);
    _server.queuePacket(client, packet);
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
            std::cout << "[Server] Removing player "
                << entity_id << " from players list\n";
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

void RtypeServer::append(std::vector<uint8_t>& vec, size_t value) const {
    std::array<uint8_t, sizeof(size_t)> bytes;
    std::memcpy(bytes.data(), &value, sizeof(size_t));
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

void RtypeServer::spawnEnnemyEntity(size_t waveNb) {
    _nextEnnemyE = createMobWave(waveNb,
        _nextEnnemyE, EntityField::ENEMIES_END);
    sendEnnemySpawn(waveNb);
}

size_t RtypeServer::spawnPlayerEntity(const net::Address& client) {
    size_t entity = _nextPlayerE++;

    createEntity(entity, "player");

    std::string addr_key = addressToString(client);
    _client_entities[addr_key] = entity;
    _players.push_back({entity, WAIT_GAME});
    return entity;
}

void RtypeServer::sendEnnemySpawn(size_t waveNb) {
    if (_server.getClientCount() == 0)
        return;

    std::vector<uint8_t> packet;

    packet.push_back(NEW_WAVE);

    append(packet, waveNb);

    std::cout << "[Server] Sending spawn wave : WAVE " << waveNb << "\n";
    _server.queueBroadcast(packet);
}

void RtypeServer::sendEnnemiesData() {
    if (_server.getClientCount() == 0)
        return;

    std::vector<uint8_t> packet;
    packet.push_back(ProtocolCode::ENNEMIES_DATA);

    auto& positions = getComponent<addon::physic::Position2>();

    for (auto &&[entity, pos] : ECS::IndexedZipper(positions)) {
        if (entity < EntityField::ENEMIES_BEGIN ||
            entity >= EntityField::ENEMIES_END)
            continue;

        append(packet, entity);
        append(packet, pos.x);
        append(packet, pos.y);
    }
    _server.queueBroadcast(packet);
}

void RtypeServer::sendProjectilesData() {
    if (_server.getClientCount() == 0)
        return;

    std::vector<uint8_t> packet;
    packet.push_back(ProtocolCode::PROJECTILES_DATA);

    auto& positions = getComponent<addon::physic::Position2>();

    for (auto &&[entity, pos] : ECS::IndexedZipper(positions)) {
        if (entity < EntityField::PROJECTILES_BEGIN ||
            entity >= EntityField::PROJECTILES_END)
            continue;

        append(packet, entity);
        append(packet, pos.x);
        append(packet, pos.y);
    }
    _server.queueBroadcast(packet);
}

void RtypeServer::sendPlayersData() {
    if (_server.getClientCount() == 0)
        return;

    std::vector<uint8_t> packet;
    packet.push_back(ProtocolCode::PLAYERS_DATA);

    auto& positions = getComponent<addon::physic::Position2>();
    auto& velocities = getComponent<addon::physic::Velocity2>();
    auto& healths = getComponent<addon::eSpec::Health>();

    for (auto &&[entity, pos, vel, hp] :
        ECS::IndexedZipper(positions, velocities, healths)) {
        if (entity < EntityField::PLAYER_BEGIN ||
            entity > EntityField::PLAYER_END)
            continue;

        append(packet, entity);
        append(packet, pos.x);
        append(packet, pos.y);
        append(packet, vel.x);
        append(packet, vel.y);
        append(packet, hp.amount);
    }
    _server.queueBroadcast(packet);
}

void RtypeServer::sendGameStart() {
    std::vector<uint8_t> packet;
    packet.push_back(GAME_START);

    std::cout << "[Server] Broadcasting GAME_START to all clients\n";
    _server.queueBroadcast(packet);
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
            std::cout << "[Server] Player " << entity_id
                << " is ready to start ("
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
                size_t ready_count = std::count_if(_players.begin(),
                    _players.end(),
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

void RtypeServer::handleShoot(const std::vector<uint8_t>& data,
    const net::Address& sender) {

    std::string addr_key = addressToString(sender);
    auto it = _client_entities.find(addr_key);
    if (it == _client_entities.end())
        return;

    const auto &player = getComponent<addon::intact::Player>();
    const auto &position = getComponent<addon::physic::Position2>();

    if (_nextProjectileE > EntityField::PROJECTILES_END)
        _nextProjectileE = EntityField::PROJECTILES_BEGIN;
    for (ECS::Entity e = 0; e < player.size() && e < position.size(); ++e) {
        if (e == it->second && player[e].has_value() && position[e].has_value())
            createEntity(_nextProjectileE++, "rocket",
                {position[e].value().x + 60, position[e].value().y + 10});
    }
}
