/*
** EPITECH PROJECT, 2025
** GameOne
** File description:
** waves.hpp
*/

#pragma once

    #include <string>
    #include <vector>
    #include <maths/Vector.hpp>

    #define BEGIN_WAVE_ENTITY 20

struct Entity {
    std::string name;
    mat::Vector2f pos;
};

typedef std::vector<Entity> wave;

static const std::vector<wave> WAVES {
    {
        {"enemy2", {1280, 360}}
    },
    {
        // === PART ONE ===
        {"enemy1", {1550, 10}},
        {"enemy1", {1550, 500}},

        {"enemy2", {1500, 200}},
        {"enemy2", {1460, 250}},
        {"enemy2", {1420, 300}},
        {"enemy2", {1380, 350}},
        {"enemy2", {1420, 400}},
        {"enemy2", {1460, 450}},
        {"enemy2", {1500, 500}},

        // === PART TWO ===
        {"enemy1", {1800, 100}},
        {"enemy1", {1800, 150}},
        {"enemy1", {1800, 200}},
        {"enemy1", {1800, 250}},
        {"enemy1", {1800, 300}},
        {"enemy1", {1800, 350}},
        {"enemy1", {1800, 400}}
    },
    {
        {"enemy3", {1500, 200}},
        {"enemy3", {1460, 250}},
        {"enemy3", {1420, 300}},
        {"enemy3", {1380, 350}},
        {"enemy3", {1420, 400}},
        {"enemy3", {1460, 450}},
        {"enemy3", {1500, 500}},
    }
};

#define NB_WAVES WAVES.size()
