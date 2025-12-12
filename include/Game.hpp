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
    explicit Game(const std::string& dir);
    ~Game();

 private:
};
