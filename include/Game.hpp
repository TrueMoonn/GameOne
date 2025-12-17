/*
** EPITECH PROJECT, 2025
** GameOne
** File description:
** Game.hpp
*/

#pragma once

#include <unordered_map>
#include <iterator>
#include <string>
#include "ECS/Entity.hpp"
#include "maths/Vector.hpp"

#include <GameTool.hpp>

#define MENU_FIELD_SIZE 10
#define MAP_FIELD_SIZE 50
#define PLAYERS_FIELD_SIZE 50
#define ENNEMIES_FIELD_SIZE 100
#define PROJECTILES_FIELD_SIZE 1000

enum EntityField : ECS::Entity {
    SYSTEM = 0,
    MENU_BEGIN = SYSTEM + 1,
    MENU_END = MENU_BEGIN + MENU_FIELD_SIZE,
    MAP_BEGIN = MENU_END + 1,
    MAP_END = MAP_BEGIN + MAP_FIELD_SIZE,
    PLAYER_BEGIN = MAP_END + 1,
    PLAYER_END = PLAYER_BEGIN + PLAYERS_FIELD_SIZE,
    ENEMIES_BEGIN = PLAYER_END + 1,
    ENEMIES_END = ENEMIES_BEGIN + ENNEMIES_FIELD_SIZE,
    PROJECTILES_BEGIN = ENEMIES_END + 1,
    PROJECTILES_END = PROJECTILES_BEGIN + PROJECTILES_FIELD_SIZE,
};

class Game : public te::GameTool {
 public:
    enum Weapons {
        MINIGUN = 0,
        ROCKET,
        SHOTGUN,
        ENDWEAPON,
    };

    static const inline std::unordered_map<Weapons, std::string>
        WEAPONS_NAMES = {
            {MINIGUN, "minigun"},
            {ROCKET, "rocket"},
            {SHOTGUN, "shotgun"}
    };

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
    std::size_t createMobWave(
      std::size_t type, std::size_t begin = EntityField::ENEMIES_BEGIN,
      std::size_t end = EntityField::ENEMIES_END);

    std::size_t createBoundaries(std::size_t begin = EntityField::MAP_BEGIN,
        std::size_t end = EntityField::MAP_END);

    void createProjectile(ECS::Entity e);
};
