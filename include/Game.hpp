/*
** EPITECH PROJECT, 2025
** GameOne
** File description:
** Game.hpp
*/

#pragma once

#include <string>

#include <GameTool.hpp>

class Game : public te::GameTool {
 public:
    explicit Game(const std::string& dir);
    ~Game();

 protected:
    void createMobWave(std::size_t type);
};
