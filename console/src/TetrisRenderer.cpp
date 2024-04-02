#include "TetrisRenderer.h"
#include "TetrisGame.h"
#include <iostream>

namespace Tetris {


    void TetrisRenderer::renderGame(const TetrisGame* game) {
        TetrisGame::TetrisGameState state = game->getState();
        get_render_stream() << TetrisGame::gameStateToInt(state) << std::endl;
        if (state != TetrisGame::TetrisGameState::Playing) {
            return;
        }

        TetrisBoard viewBoard = game->getViewBoard();
        TetrisPiece piece = game->getStoredPiece();
        for (int y = 0; y < HEIGHT; y++) {
            std::ostream& ostream = get_render_stream();
            for (int x = 0; x < WIDTH; x++) {
                ostream << viewBoard[y][x];
            }
            ostream << std::endl;
        }
        get_render_stream() << piece << std::endl; // Stored Piece
        get_render_stream() << game->getScore() << std::endl; // Score
    }

    void TetrisRenderer::renderGames(const std::vector<TetrisGame*> games) {
        get_render_stream() << "FRAME" << std::endl;
        for (const TetrisGame* game : games) {
            renderGame(game);
        }
    }

    void TetrisRenderer::setGames(int numgames) {
        get_render_stream() << "SETGAMES" << std::endl;
        get_render_stream() << std::to_string(numgames) << std::endl;
    }
}
