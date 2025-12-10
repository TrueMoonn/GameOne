#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>
#include <string>
#include <ECS/Registry.hpp>
#include <RtypeClient.hpp>

std::atomic<bool> running{true};

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\n[Client] Disconnecting..." << std::endl;
        running = false;
    }
}

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

    std::cout << "=== R-Type Client ===" << std::endl;
    std::cout << "Server: " << server_ip << ":" << port << std::endl;
    std::cout << "Protocol: " << protocol << std::endl;
    std::cout << "=====================" << std::endl;

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    try {
        // Create ECS registry
        ECS::Registry ecs;

        // Create client
        RtypeClient client(ecs, protocol);

        // Connect to server
        std::cout << "[Client] Connecting to server..." << std::endl;
        if (!client.connect(server_ip, port)) {
            std::cerr << "[Client] Failed to connect to server" << std::endl;
            return 1;
        }

        std::cout << "[Client] Connected! Starting game loop..." << std::endl;
        std::cout << "[Client] Press Ctrl+C to disconnect" << std::endl;

        // Main client loop
        const float deltaTime = 1.0f / 60.0f;  // 60 FPS
        auto lastUpdate = std::chrono::steady_clock::now();
        auto lastPing = std::chrono::steady_clock::now();

        while (running && client.isConnected()) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - lastUpdate).count();

            if (elapsed >= 16) {  // ~60 FPS
                client.update(deltaTime);
                lastUpdate = now;
            }

            // Send ping every 5 seconds
            auto pingElapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - lastPing).count();
            if (pingElapsed >= 5) {
                std::cout << "[Client] Sending PING..." << std::endl;
                client.sendPing();
                lastPing = now;
            }

            // Sleep a bit to avoid busy-waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        if (client.isConnected()) {
            std::cout << "[Client] Disconnecting..." << std::endl;
            client.disconnect();
        }
        std::cout << "[Client] Goodbye!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[Client] Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
