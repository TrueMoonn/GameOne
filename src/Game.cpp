/*
** EPITECH PROJECT, 2025
** GameOne
** File description:
** Game.cpp
*/

#include <string>
#include <ECS/Entity.hpp>

#include <Game.hpp>
#include "waves.hpp"

Game::Game(const std::string& dir) {
    loadPlugins(dir);
}

Game::~Game() {
    clearPlugins();
}

void Game::createMobWave(std::size_t index) {
    ECS::Entity e = BEGIN_WAVE_ENTITY;
    if (index < NB_WAVES) {
        for (auto& entity : WAVES[index]) {
            createEntity(e++, entity.name, entity.pos);
        }
    }
}
