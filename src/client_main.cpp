/*
** EPITECH PROJECT, 2025
** GameOne
** File description:
** client_main.cpp
** Copyright [2025] <DeepestDungeonGroup>
*/

#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>
#include <string>
#include <ECS/Registry.hpp>
#include <GameTool.hpp>
#include <RtypeClient.hpp>

int main(int argc, char** argv) {
    std::string server_ip = "127.0.0.1";
    uint16_t port = 8080;
    std::string protocol = "UDP";

    if (argc > 1) {
        server_ip = argv[1];
    }
    if (argc > 2) {
        port = static_cast<uint16_t>(std::stoi(argv[2]));
    }
    if (argc > 3) {
        protocol = argv[3];
    }

    RtypeClient client = RtypeClient(protocol, port, server_ip);

    client.run();

    return 0;
}
