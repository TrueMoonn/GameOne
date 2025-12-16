/*
** EPITECH PROJECT, 2025
** GameOne
** File description:
** RtypeClient.hpp
** Copyright [2025] <DeepestDungeonGroup>
*/

#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <chrono>
#include <unordered_map>
#include <GameTool.hpp>
#include <network/GameClient.hpp>
#include <event/events.hpp>
#include <Game.hpp>
#include <Protocol.hpp>
// #include <GameException.hpp>

class RtypeClient : public Game {
 public:
    explicit RtypeClient(const std::string& protocol = "UDP",
         uint16_t port = 5000, const std::string& server_ip = "127.0.0.1");
    ~RtypeClient();

    void run();

    void sendPing();
    void sendWantStart();  // Envoyer WANT_START au serveur
    void sendShoot();

    std::chrono::_V2::steady_clock::time_point getPing();
    void setPing(std::chrono::_V2::steady_clock::time_point);

    #define FPS 60

    class TypeExtractError : public std::exception {
     public:
        explicit TypeExtractError(std::string msg = "Type Extraction error")
            : _msg(msg) {}
        const char* what() const noexcept override {return _msg.c_str();}
     private:
        std::string _msg;
    };

    // TE_EXCEPTION("RTypeClient", TypeExtractError)  // ca marche po

 private:
    te::network::GameClient _client;
    uint16_t _server_port;
    std::string _server_ip;
    std::chrono::_V2::steady_clock::time_point _pingTime;

    uint32_t next_entity_id = 1;
    std::optional<uint32_t> _my_server_entity_id;
    std::optional<uint32_t> _my_client_entity_id;
    std::unordered_map<uint32_t, uint32_t> _serverToClientEntityMap;
    std::vector<int> _players;

    // next entities
    size_t _nextMap = MAP_BEGIN;
    size_t _nextPlayer = PLAYER_BEGIN;
    size_t _nextEnnemy = ENEMIES_BEGIN;
    size_t _nextProjectile = PROJECTILES_BEGIN;

    bool connect(const std::string& ip, uint16_t port);
    void disconnect();
    void update(float delta_time);
    void sendEvent(te::event::Events events);
    bool isConnected() const { return _client.isConnected(); }
    te::network::GameClient& getClient() { return _client; }

    void waitGame();  // Boucle d'attente (WAIT_GAME et READY_TO_START)
    void runGame();   // Boucle de jeu principale (IN_GAME)

    void playersAnimation(void);

    void registerProtocolHandlers();

    void sendConnectionRequest();
    void sendDisconnection();
    void sendPong();

    void handleConnectionAccepted(const std::vector<uint8_t>& data);
    void handleDisconnection(const std::vector<uint8_t>& data);
    void handleServerFull(const std::vector<uint8_t>& data);
    void handlePing(const std::vector<uint8_t>& data);
    void handlePong(const std::vector<uint8_t>& data);
    void handleEnnemiesData(const std::vector<uint8_t>& data);
    void handlePlayersData(const std::vector<uint8_t>& data);
    void handleProjectilesData(const std::vector<uint8_t>& data);
    void handleGameStarted(const std::vector<uint8_t>& data);
    void handleGameEnded(const std::vector<uint8_t>& data);
    void handleWaveSpawned(const std::vector<uint8_t>& data);

    void append(std::vector<uint8_t>& vec, uint32_t value) const;
    uint32_t extractUint32(const std::vector<uint8_t>& data, size_t off) const;
    size_t extractSizeT(const std::vector<uint8_t>& data, size_t off) const;
    int64_t extractInt64(const std::vector<uint8_t>& data, size_t off) const;
    float extractFloat(const std::vector<uint8_t>& data, size_t off) const;
};
