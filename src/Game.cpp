/*
** EPITECH PROJECT, 2025
** GameOne
** File description:
** Game.cpp
*/

#include <string>
#include <clock.hpp>
#include <ECS/Entity.hpp>
#include <physic/components/position.hpp>

#include <Game.hpp>
#include "waves.hpp"

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

void Game::createMobWave(std::size_t index) {
    ECS::Entity e = BEGIN_WAVE_ENTITY;
    if (index < NB_WAVES) {
        for (auto& entity : WAVES[index]) {
            createEntity(e++, entity.name, entity.pos);
        }
    }
}

void Game::createProjectile(ECS::Entity e) {
    // static te::Timestamp delay(0.2f);

    // const auto &position = getComponent<addon::physic::Position2>();
    // if (position[e].has_value()) {
    //     createEntity(entity_proj++, "shoot",
    //         {position[e].value().x + 10, position[e].value().y});
    // }
}
