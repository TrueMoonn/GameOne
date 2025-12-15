/*
** EPITECH PROJECT, 2025
** GameOne
** File description:
** Game.cpp
** Copyright [2025] <DeepestDungeonGroup>
*/

#include <string>

#include <Game.hpp>

Game::Game(const std::string& dir) {
    loadPlugins(dir);
}

Game::~Game() {
    clearPlugins();
}

Game::GAME_STATE Game::getGameState() const {
    return _game_state;
}

void Game::setGameState(Game::GAME_STATE game_state) {
    _game_state = game_state;
}
