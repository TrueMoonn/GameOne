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

class RtypeClient : public Game {
 public:
    explicit RtypeClient(const std::string& protocol = "UDP",
         uint16_t port = 5000, const std::string& server_ip = "127.0.0.1");
    ~RtypeClient();

    void run();

    void sendPing();

    std::chrono::_V2::steady_clock::time_point getPing();
    void setPing(std::chrono::_V2::steady_clock::time_point);

    #define FPS 60

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

    bool connect(const std::string& ip, uint16_t port);
    void disconnect();
    void update(float delta_time);
    void sendEvent(te::event::Events events);
    bool isConnected() const { return _client.isConnected(); }
    te::network::GameClient& getClient() { return _client; }

    void registerProtocolHandlers();

    void sendConnectionRequest();
    void sendDisconnection();
    void sendPong();

    void handleConnectionAccepted(const std::vector<uint8_t>& data);
    void handleDisconnection(const std::vector<uint8_t>& data);
    void handleServerFull(const std::vector<uint8_t>& data);
    void handlePing(const std::vector<uint8_t>& data);
    void handlePong(const std::vector<uint8_t>& data);
    void handleEntityState(const std::vector<uint8_t>& data);
    void handlePlayersStates(const std::vector<uint8_t>& data);

    void append(std::vector<uint8_t>& vec, uint32_t value) const;
    uint32_t extractUint32(const std::vector<uint8_t>& data, size_t off) const;
    int64_t extractInt64(const std::vector<uint8_t>& data, size_t off) const;
    float extractFloat(const std::vector<uint8_t>& data, size_t off) const;
};
