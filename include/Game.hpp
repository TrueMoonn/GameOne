/*
** EPITECH PROJECT, 2025
** GameOne
** File description:
** Game.hpp
** Copyright [2025] <DeepestDungeonGroup>
*/

#pragma once

#include <string>

#include <GameTool.hpp>

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
};
