#pragma once

#include "TetrisAction.h"
#include "TetrisGame.h"
#include "TetrisRenderer.h"

#include <iostream>
#include <array>
#include <vector>
#include <utility> // for std::pair
#include <algorithm> // for std::min_element

// #define NUM_GAMES 3
#define TICKS_PER_SECOND 3

namespace Tetris {
    class TetrisGameManager {
    private:
        std::vector<TetrisGame*> games {};

        TetrisRenderer renderer;

        bool running {false};

    public:
        TetrisGameManager(const TetrisRenderer& renderer) : renderer(renderer) {}

        ~TetrisGameManager() {
            for (auto& game : games) {
                delete game;
            }
        }

        void playGame();

        int addGame();

        void pushAction(int gameIndex, TetrisAction action);

        void renderGames();

        void runTick();
    };

}