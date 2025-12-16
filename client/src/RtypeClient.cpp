/*
** EPITECH PROJECT, 2025
** GameOne
** File description:
** RtypeClient.cpp
** Copyright [2025] <DeepestDungeonGroup>
*/

#include <arpa/inet.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <csignal>
#include <vector>
#include <unordered_map>
#include <physic/components/position.hpp>
#include <entity_spec/components/health.hpp>
#include <event/events.hpp>
#include <RtypeClient.hpp>
#include <GameTool.hpp>

RtypeClient::RtypeClient(const std::string& protocol, uint16_t port,
    const std::string& server_ip)
    : Game("./client/plugins")
    , _client(protocol)
    , _server_port(port)
    , _server_ip(server_ip) {
    createComponent("window", 0);

    createSystem("apply_pattern");
    createSystem("bound_hitbox");
    createSystem("deal_damage");
    createSystem("movement2");
    createSystem("animate");
    createSystem("draw");
    createSystem("parallax_sys");
    createSystem("display");
    addConfig("./client/assets/enemies/basic/player.toml");

    // BACKGROUND
    addConfig("./client/assets/background/config.toml");

    // MOBS
    addConfig("./config/entities/enemy1.toml");
    addConfig("./client/assets/enemies/basic/enemy1.toml");
    addConfig("./config/entities/enemy2.toml");
    addConfig("./client/assets/enemies/basic/enemy2.toml");

    registerProtocolHandlers();
    _client.setConnectCallback([this]() {
        std::cout
        << "[Client] Network connection established, "
        << "sending CONNECTION_REQUEST...\n";
        sendConnectionRequest();
    });

    _client.setDisconnectCallback([this]() {
        std::cout << "[Client] Disconnected from server\n";
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
        std::cout << "\n[Client] Disconnecting...\n";
        return;  // Throw or exit
    }
}

void RtypeClient::run() {
    std::cout << "=== R-Type Client ===\n";
    std::cout << "Server: " << _server_ip << ":" << _server_port << "\n";
    std::cout << "=====================\n";

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    try {
        std::cout << "[Client] Connecting to server...\n";
        if (!connect(_server_ip, _server_port)) {
            std::cerr << "[Client] Failed to connect to server\n";
            return;
        }

        std::cout << "[Client] Connected! Loading entities...\n";

        createEntity(10, "bg1");
        createEntity(11, "bg2");
        createEntity(12, "bg3");
        createEntity(13, "bg4");
        createEntity(14, "bg5");
        createEntity(15, "bg6");

        std::cout << "[Client] Connected! Waiting for game to start...\n";
        std::cout << "[Client] Press Ctrl+C to disconnect\n";

        while (!isEvent(te::event::System::Closed) && isConnected()) {
            if (getGameState() == GAME_WAITING || getGameState() == IN_GAME) {
                if (getGameState() == GAME_WAITING) {
                    waitGame();
                } else {
                    runGame();
                }
            } else if (getGameState() == GAME_ENDED) {
                std::cout << "[Client] Game ended!\n";
                break;
            }
        }

        if (isConnected()) {
            std::cout << "[Client] Disconnecting...\n";
            disconnect();
        }
        std::cout << "[Client] Goodbye!\n";
    } catch (const std::exception& e) {
        std::cerr << "[Client] Fatal error: " << e.what() << "\n";
        return;
    }
}

void RtypeClient::waitGame() {
    const float deltaTime = 1.0f / FPS;
    auto lastUpdate = std::chrono::steady_clock::now();
    auto lastPing = std::chrono::steady_clock::now();

    while (!isEvent(te::event::System::Closed) && isConnected()
           && getGameState() == GAME_WAITING) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastUpdate).count();

        if (elapsed >= (1000.0f / FPS)) {
            update(deltaTime);
            lastUpdate = now;
        }

        // Send ping every 5 seconds
        auto pingElapsed =
            std::chrono::duration_cast<std::chrono::seconds>(
            now - lastPing).count();
        if (pingElapsed >= 5) {
            std::cout << "[Client] Sending PING...\n";
            sendPing();
            lastPing = now;
        }

        pollEvent();
        auto events = getEvents();

        // TEMPORAIRE : Appuyer sur P pour envoyer WANT_START (test)
        if (events.keys.UniversalKey[te::event::P]) {
            std::cout
                << "[Client] P pressed - Sending WANT_START (test mode)\n";
            sendWantStart();
        }

        // TODO(ETHAN):
        // Ajouter ici la logique pour afficher et gérer le bouton "Ready"
        // et appeler sendWantStart quand il clique sur le bouton

        runSystems();
    }
}

void RtypeClient::runGame() {
    const float deltaTime = 1.0f / FPS;
    auto lastUpdate = std::chrono::steady_clock::now();
    auto lastPing = std::chrono::steady_clock::now();

    while (!isEvent(te::event::System::Closed) && isConnected()
           && getGameState() == IN_GAME) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastUpdate).count();

        if (elapsed >= (1000.0f / FPS)) {
            update(deltaTime);
            lastUpdate = now;
        }

        // Send ping every 5 seconds
        auto pingElapsed =
            std::chrono::duration_cast<std::chrono::seconds>(
            now - lastPing).count();
        if (pingElapsed >= 5) {
            std::cout << "[Client] Sending PING...\n";
            sendPing();
            lastPing = now;
        }

        pollEvent();
        auto events = getEvents();

        if ((events.keys.UniversalKey[te::event::Z]
            || events.keys.UniversalKey[te::event::Q]
            || events.keys.UniversalKey[te::event::S]
            || events.keys.UniversalKey[te::event::D])) {
            sendEvent(events);
        }

        if (_my_client_entity_id.has_value()) {
            emit(_my_client_entity_id);
        }

        if (events.keys.UniversalKey[te::event::Space])
            sendShoot();

        runSystems();
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

void RtypeClient::sendShoot() {
     static te::Timestamp delay(0.2f);
    if (!isConnected() && !delay.checkDelay()) {
        return;
    }

    // TODO(PIERRE): delay
    std::vector<uint8_t> packet;

    packet.push_back(PLAYER_SHOT);
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

    _client.registerPacketHandler(PLAYERS_DATA,
        [this](const std::vector<uint8_t>& data) {
            handlePlayersData(data);
        });

    _client.registerPacketHandler(PROJECTILES_DATA,
        [this](const std::vector<uint8_t>& data) {
            handleProjectilesData(data);
        });

    _client.registerPacketHandler(ENNEMIES_DATA,
        [this](const std::vector<uint8_t>& data) {
            handleEnnemiesData(data);
        });

    _client.registerPacketHandler(GAME_START,
        [this](const std::vector<uint8_t>& data) {
            handleGameStarted(data);
        });

    _client.registerPacketHandler(GAME_ENDED,
        [this](const std::vector<uint8_t>& data) {
            handleGameEnded(data);
        });

    _client.registerPacketHandler(NEW_WAVE,
        [this](const std::vector<uint8_t>& data) {
            handleWaveSpawned(data);
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

void RtypeClient::sendWantStart() {
    if (!isConnected()) {
        std::cerr << "[Client] Cannot send WANT_START: not connected\n";
        return;
    }

    std::vector<uint8_t> packet;
    packet.push_back(WANT_START);

    std::cout << "[Client] Sending WANT_START to server\n";
    _client.send(packet);
}

void RtypeClient::handleConnectionAccepted(const std::vector<uint8_t>& data) {
    if (data.size() < 4) {
        std::cerr << "[Client] Invalid CONNECTION_ACCEPTED packet size\n";
        return;
    }

    uint32_t entity_id = extractUint32(data, 0);
    _my_server_entity_id = entity_id;

    std::cout << "[Client] Connection accepted! Our server entity ID: "
        << entity_id << "\n";

    // Create our player entity locally
    uint32_t client_id = next_entity_id++;
    _my_client_entity_id = client_id;

    std::cout << "[Client] Created local player entity: server_id="
        << entity_id << " -> client_id=" << client_id << "\n";

    // Mettre le jeu en mode attente
    setGameState(GAME_WAITING);
    std::cout << "[Client] Waiting for players to be ready...\n";
}

void RtypeClient::handleDisconnection(const std::vector<uint8_t>& data) {
    std::cout << "[Client] Server disconnected us\n";
}

void RtypeClient::handleServerFull(const std::vector<uint8_t>& data) {
    std::cout << "[Client] Server full!\n";
    disconnect();  // TODO(PIERRE): On pourrait le laisser attendre
    // ouais why not, on le deco si il fait rien trop longtemps
}

void RtypeClient::handlePing(const std::vector<uint8_t>& data) {
    std::cout << "[Client] Ping received, sending pong...\n";
    sendPong();
}

void RtypeClient::handlePong(const std::vector<uint8_t>& data) {
    std::cout << "[Client] Pong received\n";
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
    if (offset + sizeof(float) > data.size())
        throw TypeExtractError("Could not extract float at pos " + offset);
    float value;
    std::memcpy(&value, data.data() + offset, sizeof(float));
    return value;
}

uint32_t RtypeClient::extractUint32(const std::vector<uint8_t>& data,
    size_t offset) const {
    if (offset + sizeof(uint32_t) > data.size())
        throw TypeExtractError("Could not extract uint32_t at pos " + offset);
    uint32_t value;
    std::memcpy(&value, data.data() + offset, sizeof(uint32_t));
    return value;
}

size_t RtypeClient::extractSizeT(const std::vector<uint8_t>& data,
    size_t offset) const {
    if (offset + sizeof(size_t) > data.size())
        throw TypeExtractError("Could not extract size_t at pos " + offset);
    size_t value;
    std::memcpy(&value, data.data() + offset, sizeof(size_t));
    return value;
}

int64_t RtypeClient::extractInt64(const std::vector<uint8_t>& data,
    size_t offset) const {
    if (offset + sizeof(int64_t) > data.size())
        throw TypeExtractError("Could not extract int64_t at pos " + offset);
    int64_t value;
    std::memcpy(&value, data.data() + offset, sizeof(int64_t));
    return value;
}

void RtypeClient::handleEnnemiesData(const std::vector<uint8_t>& data) {
    size_t size = data.size();
    size_t follow = 0;

    std::vector<bool> present(
        (EntityField::ENEMIES_END - EntityField::ENEMIES_BEGIN), false);

    while (follow < size) {
        size_t entity = extractSizeT(data, follow);
        follow += sizeof(size_t);
        float x = extractFloat(data, follow);
        follow += sizeof(float);
        float y = extractFloat(data, follow);
        follow += sizeof(float);

        if (entity < EntityField::ENEMIES_BEGIN ||
            entity > EntityField::ENEMIES_END)
            continue;

        present[entity - EntityField::ENEMIES_BEGIN] = true;

        auto& positions = getComponent<addon::physic::Position2>();

        if (!positions[entity].has_value()) {
            _nextEnnemy++;
            createEntity(entity, "ennemy1", {x, y});

            std::cout << "[Client] Created projectile entity: entity="
                << entity << " -> entity=" << entity
                << " at position (" << x << ", " << y << ")\n";
        } else {
            positions[entity].value().x = x;
            positions[entity].value().y = y;
        }
    }

    // A mettre avant la boucle + enlever celui dedans pour l'opti ?
    auto& positions = getComponent<addon::physic::Position2>();

    // Delete absent ennemies
    for (size_t idx = EntityField::ENEMIES_BEGIN;
        idx < EntityField::ENEMIES_END; idx++) {
        if (!positions[idx].has_value())
            continue;
        if (!present[idx - EntityField::ENEMIES_BEGIN]) {
            removeEntity(idx);
            std::cout << "deleted ennemy\n";
        }
    }
}

void RtypeClient::handleProjectilesData(const std::vector<uint8_t>& data) {
    size_t size = data.size();
    size_t follow = 0;

    std::vector<bool> present(
        (EntityField::PROJECTILES_END - EntityField::PROJECTILES_BEGIN), false);

    while (follow < size) {
        size_t entity = extractSizeT(data, follow);
        follow += sizeof(size_t);
        float x = extractFloat(data, follow);
        follow += sizeof(float);
        float y = extractFloat(data, follow);
        follow += sizeof(float);

        if (entity < EntityField::PROJECTILES_BEGIN ||
            entity > EntityField::PROJECTILES_END)
            continue;

        present[entity - EntityField::PROJECTILES_BEGIN] = true;

        auto& positions = getComponent<addon::physic::Position2>();

        if (!positions[entity].has_value()) {
            _nextProjectile++;
            createEntity(entity, "projectile", {x, y});

            std::cout << "[Client] Created projectile entity: entity="
                << entity << " -> entity=" << entity
                << " at position (" << x << ", " << y << ")\n";
        } else {
            positions[entity].value().x = x;
            positions[entity].value().y = y;
        }
    }

    // A mettre avant la boucle + enlever celui dedans pour l'opti ?
    auto& positions = getComponent<addon::physic::Position2>();

    // Delete absent projectiles
    for (size_t idx = EntityField::PROJECTILES_BEGIN;
        idx < EntityField::PROJECTILES_END; idx++) {
        if (!positions[idx].has_value())
            continue;
        if (!present[idx - EntityField::PROJECTILES_BEGIN]) {
            removeEntity(idx);
            std::cout << "deleted projectile\n";
        }
    }
}

void RtypeClient::handlePlayersData(const std::vector<uint8_t>& data) {
    size_t size = data.size();
    size_t follow = 0;

    std::vector<bool> present(
        (EntityField::PLAYER_END - EntityField::PLAYER_BEGIN), false);


    while (follow < size) {
        size_t entity = extractSizeT(data, follow);
        follow += sizeof(size_t);
        float x = extractFloat(data, follow);
        follow += sizeof(float);
        float y = extractFloat(data, follow);
        follow += sizeof(float);
        int64_t hp = extractInt64(data, follow);
        follow += sizeof(int64_t);

        if (entity < EntityField::PLAYER_BEGIN ||
            entity > EntityField::PLAYER_END)
            continue;

        present[entity - EntityField::PLAYER_BEGIN] = true;

        // if (_my_server_entity_id.has_value()
        //     && entity == _my_server_entity_id.value()) {
        //     // recaler myplayer
        // }

        auto& positions = getComponent<addon::physic::Position2>();
        auto& healths = getComponent<addon::eSpec::Health>();

        if (!positions[entity].has_value()) {
            _nextPlayer++;
            createEntity(entity, "player", {0, 0});

            std::cout <<
            "[Client] Created OTHER player entity: entity=" <<
                entity << " -> entity=" << entity
                << " at position (" << x << ", " << y << ")\n";
        } else {
            positions[entity].value().x = x;
            positions[entity].value().y = y;

            std::cout <<
                "[Client] Updated OTHER player entity entity=" << entity
                << " (entity=" << entity
                << ") to position (" << x << ", " << y << ")\n";
        }

        if (healths[entity].has_value()) {
            healths[entity].value().amount = hp;

            std::cout <<
                "[Client] Updated OTHER player entity entity="
                << entity
                << " (entity=" << entity << ") to health ("
                << hp << ")\n";
        } else {
            std::cout <<
                "[Client] ERROR: Cannot update health for entity="
                << entity
                << " (healths.size()=" << healths.size() << ")\n";
        }
    }

    // A mettre avant la boucle + enlever celui dedans pour l'opti ?
    auto& positions = getComponent<addon::physic::Position2>();

    // Delete absent players
    for (size_t idx = EntityField::PLAYER_BEGIN;
        idx < EntityField::PLAYER_END; idx++) {
        if (!positions[idx].has_value())
            continue;
        if (!present[idx - EntityField::PLAYER_BEGIN]) {
            removeEntity(idx);
            std::cout << "deleted player\n";
        }
    }
}

void RtypeClient::handleGameStarted(const std::vector<uint8_t>& data) {
    Game::setGameState(Game::IN_GAME);
    if (_my_client_entity_id.has_value())
        createEntity(_my_client_entity_id.value(), "player", {0, 0});
}

void RtypeClient::handleGameEnded(const std::vector<uint8_t>& data) {
    Game::setGameState(Game::GAME_ENDED);
}

void RtypeClient::handleWaveSpawned(const std::vector<uint8_t>& data) {
    size_t waveNb = extractSizeT(data, 0);

    _nextEnnemy = createMobWave(waveNb, _nextEnnemy, EntityField::ENEMIES_END);
}
