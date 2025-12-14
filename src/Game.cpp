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
