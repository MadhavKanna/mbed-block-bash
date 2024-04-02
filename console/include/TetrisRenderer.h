#pragma once

#include "TetrisGame.h"


namespace Tetris {


    class TetrisRenderer {
    private:
        void renderGame(const TetrisGame* game);

        static std::ostream& get_render_stream() {
            std::cout << "RENDER ";
            return std::cout;
        }

    public:
        TetrisRenderer(int width=WIDTH, int height=HEIGHT) {
            get_render_stream() << "0 " + std::to_string(width) + " " + std::to_string(height) << std::endl;
        }

        void renderGames(const std::vector<TetrisGame*> games);

        void setGames(int numgames);
    };
}
