#pragma once

#include <string>

#include <GameTool.hpp>

class Game : public te::GameTool {
 public:
    explicit Game(const std::string& dir);
    ~Game();

 private:
};
