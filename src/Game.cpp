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

std::size_t Game::createMobWave(std::size_t index,
    std::size_t begin, std::size_t end) {
    ECS::Entity e = begin;
    if (index < NB_WAVES) {
        for (auto& entity : WAVES[index]) {
            if (e >= end)
                break;
            createEntity(e++, entity.name, entity.pos);
        }
    }
    return e;
}

std::size_t Game::createBoundaries(std::size_t begin, std::size_t end) {
    createEntity(begin++, "boundaries_left");
    createEntity(begin++, "boundaries_right");
    createEntity(begin++, "boundaries_top");
    createEntity(begin++, "boundaries_bottom");
    createEntity(begin++, "kz_mob");
    createEntity(begin++, "kz_projectile_right");
    return begin;
}

void Game::createProjectile(ECS::Entity e) {
    // static te::Timestamp delay(0.2f);

    // const auto &position = getComponent<addon::physic::Position2>();
    // if (position[e].has_value()) {
    //     createEntity(entity_proj++, "shoot",
    //         {position[e].value().x + 10, position[e].value().y});
    // }
}
