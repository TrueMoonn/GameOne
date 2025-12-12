/*
** EPITECH PROJECT, 2025
** GameOne
** File description:
** server_main.cpp
** Copyright [2025] <DeepestDungeonGroup>
*/

#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>
#include <string>
#include <ECS/Registry.hpp>
#include <RtypeServer.hpp>

int main(int argc, char** argv) {
    uint16_t port = 8080;
    std::string protocol = "UDP";
    size_t max_clients = 4;

    if (argc > 1) {
        port = static_cast<uint16_t>(std::stoi(argv[1]));
    }
    if (argc > 2) {
        protocol = argv[2];
    }
    if (argc > 3) {
        max_clients = static_cast<size_t>(std::stoi(argv[3]));
    }

    RtypeServer server(port, protocol, max_clients);

    server.run();
    return 0;
}
