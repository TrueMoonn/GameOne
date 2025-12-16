/*
** EPITECH PROJECT, 2025
** GameOne
** File description:
** Game.hpp
*/

#pragma once

#include <string>
#include "ECS/Entity.hpp"
#include "maths/Vector.hpp"

#include <GameTool.hpp>

enum EntityField : ECS::Entity {
    SYSTEM = 0,
    MENU_BEGIN = SYSTEM + 1,
    MENU_END = MENU_BEGIN + 10,
    MAP_BEGIN = MENU_END + 1,
    MAP_END = MAP_BEGIN + 50,
    PLAYER_BEGIN = MAP_END + 1,
    PLAYER_END = PLAYER_BEGIN + 50,
    ENEMIES_BEGIN = PLAYER_END + 1,
    ENEMIES_END = ENEMIES_BEGIN + 100,
    PROJECTILES_BEGIN = ENEMIES_END + 1,
    PROJECTILES_END = PROJECTILES_BEGIN + 1000,
};

class Game : public te::GameTool {
 public:
    enum GAME_STATE : uint8_t {
      GAME_WAITING = 1,
      IN_GAME = 2,
      GAME_ENDED = 3
    };
 public:
    explicit Game(const std::string& dir);
    ~Game();

    GAME_STATE getGameState() const;
    void setGameState(GAME_STATE game_state);

 private:
    GAME_STATE _game_state = GAME_WAITING;
 protected:
    void createMobWave(std::size_t type);
    void createProjectile(ECS::Entity e);
};
