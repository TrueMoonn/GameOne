#include <string>

#include "Game.hpp"

Game::Game(const std::string& dir) {
    loadPlugins(dir);
}

Game::~Game() {
    clearPlugins();
}
