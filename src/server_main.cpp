#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>
#include <string>
#include <ECS/Registry.hpp>
#include <RtypeServer.hpp>

std::atomic<bool> running{true};

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\n[Server] Shutting down gracefully..." << std::endl;
        running = false;
    }
}

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

    std::cout << "=== R-Type Server ===" << std::endl;
    std::cout << "Port: " << port << std::endl;
    std::cout << "Protocol: " << protocol << std::endl;
    std::cout << "Max clients: " << max_clients << std::endl;
    std::cout << "=====================" << std::endl;

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    try {
        // Create ECS registry
        ECS::Registry ecs;

        // Create and start server
        RtypeServer server(ecs, port, protocol, max_clients);

        if (!server.start()) {
            std::cerr << "[Server] Failed to start server on port " << port << std::endl;
            return 1;
        }

        std::cout << "[Server] Server started successfully!" << std::endl;
        std::cout << "[Server] Press Ctrl+C to stop" << std::endl;

        // Main server loop
        const float deltaTime = 1.0f / 60.0f;  // 60 FPS  (actuellement ca sert à rien mdr)
        auto lastUpdate = std::chrono::steady_clock::now();

        while (running) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - lastUpdate).count();

            if (elapsed >= 16) {  // ~60 FPS
                server.update(deltaTime);
                lastUpdate = now;
            }

            // Sleep a bit bro
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        std::cout << "[Server] Stopping server..." << std::endl;
        server.stop();
        std::cout << "[Server] Server stopped. Goodbye!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[Server] Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
