#include "TetrisManager.h"

namespace Tetris {

    void TetrisGameManager::playGame() {
        if (running) {
            return;
        }

        for (auto& game : games) {
            game->start();
        }
        renderGames();
    }

    int TetrisGameManager::addGame() {
        TetrisGame* game = new TetrisGame();
        games.push_back(game);
        renderer.setGames(games.size());
        renderGames();
        return games.size() - 1;
    }

    void TetrisGameManager::pushAction(int gameIndex, TetrisAction action) {
        if (gameIndex < 0 || gameIndex >= games.size()) {
            return;
        }

        games[gameIndex]->applyAction(action);
        renderer.renderGames(games);
    }

    void TetrisGameManager::renderGames() {
        renderer.renderGames(games);
    }

    void TetrisGameManager::runTick() {
        for (auto& game : games) {
            game->tick();
        }
        renderGames();
    }
};
