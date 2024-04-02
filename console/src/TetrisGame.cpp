#include <vector>
#include <set>
#include <utility> // for std::pair
#include <algorithm> // for std::min_element
#include <cstdlib>
#include <ctime>

#include "TetrisGame.h"

namespace Tetris {
    FallingPiece TetrisGame::rotatePiece(const FallingPiece& piece) {
        FallingPiece rotatedPiece;

        if (piece.empty()) {
            return rotatedPiece;
        }

        // Find the minimum x and y values to determine the pivot point
        // int minX = std::min_element(piece.begin(), piece.end(), 
        //                             [](const std::pair<int, int>& a, const std::pair<int, int>& b) { return a.first < b.first; })->x;
        // int minY = std::min_element(piece.begin(), piece.end(), 
        //                             [](const std::pair<int, int>& a, const std::pair<int, int>& b) { return a.second < b.second; })->y;
                                
        int minX = std::min_element(piece.begin(), piece.end(), 
                                    [](const Square& a, const Square& b) { return a.x < b.x; })->x;
        int minY = std::min_element(piece.begin(), piece.end(), 
                                    [](const Square& a, const Square& b) { return a.y < b.y; })->y;

        for (const auto& block : piece) {
            int x = block.x;
            int y = block.y;

            // Rotate the block around the pivot (minX, minY)
            int newX = minX - (y - minY) + 1;
            int newY = minY + (x - minX);

            rotatedPiece.push_back({newY, newX});
        }

        return rotatedPiece;
    }

    FallingPiece TetrisGame::shiftDownOne() {
        FallingPiece newPiece {};
        for (auto& square : currentPiece) {
            newPiece.push_back({square.y + 1, square.x});
        }

        return newPiece;
    }

    bool TetrisGame::moveDownOneChecked() {
        // Move the current piece down
        FallingPiece newPiece = shiftDownOne();

        // Check if overlaps with any existing blocks
        if (checkCollision(newPiece)) {
            // Overlaps with existing block, so spawn a new piece
            placePiece();
            spawnPiece();
            //std::cout << "Placed piece" << std::endl;
            return false;
        }
        currentPiece = newPiece;
        return true;
    }

    bool TetrisGame::checkCollision(const FallingPiece& piece) {
        for (auto& block : piece) {
            if (block.y >= HEIGHT || block.x < 0 || block.x >= WIDTH || board[block.y][block.x] != 0) {
                return true;
            }
        }

        return false;
    }


    void TetrisGame::spawnPiece(TetrisPiece pieceType) {
        //std::cout << "Spawning piece" << std::endl;
        TetrisPiece selectedPieceType = TetrisPiece(pieceType == PIECE_None ? rand() % 6 : pieceType);
        currentPieceType = selectedPieceType;

        currentPiece.clear();
        currentPiece = pieces[selectedPieceType];

        for (auto& square : currentPiece) {
            square.x += WIDTH / 2 - 1;
        }

        // Game is over when the newly spawned piece collides with existing blocks
        if (checkCollision(currentPiece)) {
            state = TetrisGameState::GameOver;
        }
    }


    void TetrisGame::placePiece() {
        //std::cout << "Placing piece" << std::endl;
        for (auto& block : currentPiece) {
            board[block.y][block.x] = 1;
        }

        // Check for completed rows
        std::set<int> rowsToRemove {};
        for (int y = 0; y < HEIGHT; y++) {
            bool rowComplete = true;
            for (int x = 0; x < WIDTH; x++) {
                if (board[y][x] == 0) {
                    rowComplete = false;
                    break;
                }
            }
            if (rowComplete) {
                score += 1;
                rowsToRemove.insert(y);
            }
        }
        removeRows(rowsToRemove);
        swapped = false;
    }

    void TetrisGame::removeRows(const std::set<int>& rowNums) {
        std::vector<int> keepRows = {};
        for (int y = 0; y < HEIGHT; y++) {
            if (rowNums.find(y) == rowNums.end()) {
                keepRows.push_back(y);
            }
        }

        for (int y = HEIGHT-1; y >= 0; y--) {
            if (keepRows.empty()) {
                for (int x = 0; x < WIDTH; x++) {
                    board[y][x] = 0;
                }
            } else {
                int nextRow = keepRows.back();
                for (int x = 0; x < WIDTH; x++) {
                    board[y][x] = board[nextRow][x];
                }
                keepRows.pop_back();
            }
        }
    }

    bool TetrisGame::moveLeft() {
        FallingPiece newPiece = currentPiece;

        // Move the piece left
        for (auto& block : newPiece) {
            block.x -= 1;
        }

        // Check if the piece can move left
        if (checkCollision(newPiece)) {
            return false;
        }

        currentPiece = newPiece;
        return true;
    }

    bool TetrisGame::moveRight() {
        FallingPiece newPiece = currentPiece;

        // Move the piece right
        for (auto& block : newPiece) {
            block.x += 1;
        }

        // Check if the piece can move right
        if (checkCollision(newPiece)) {
            return false;
        }

        currentPiece = newPiece;
        return true;
    }

    bool TetrisGame::moveRotate() {
        // Rotate the piece
        FallingPiece newPiece = rotatePiece(currentPiece);

        if (checkCollision(newPiece)) {
            return false;
        }

        // Apply the rotation
        currentPiece = newPiece;

        return true;
    }

    bool TetrisGame::moveStore() {
        if (swapped) {
            return false;
        }
        swapped = true;

        // Swap the current piece with the stored piece
        TetrisPiece newSpawn = currentPieceType;

        spawnPiece(stored);
        stored = newSpawn;

        return true;
    }

    void TetrisGame::start() {
        state = TetrisGameState::Playing;

        // Seed the random number generator
        std::srand(std::time(0));

        // Initialize the game board with zeros
        for (auto& row : board) {
            row.fill(0);
        }

        // Spawn the first piece
        spawnPiece();
    }
    
    TetrisBoard TetrisGame::getViewBoard() const {
        TetrisBoard viewBoard = board;

        for (auto& block : currentPiece) {
            viewBoard[block.y][block.x] = 2;
        }

        return viewBoard;
    }

    const TetrisBoard& TetrisGame::getBoard() const {
        return board;
    }

    TetrisPiece TetrisGame::getStoredPiece() const {
        return stored;
    }


    void TetrisGame::tick() {
        if (state != TetrisGameState::Playing) {
            return;
        }

        moveDownOneChecked();
    }


    void TetrisGame::applyAction(TetrisAction action) {
        if (state != TetrisGameState::Playing) {
            return;
        }

        // Apply the correct action
        bool drop = false;
        switch (action) {
            case TetrisAction::MoveLeft:
                moveLeft();
                break;
            case TetrisAction::MoveRight:
                moveRight();
                break;
            case TetrisAction::Rotate:
                moveRotate();
                break;
            case TetrisAction::Drop:
                drop = true;
                break;
            case TetrisAction::Store:
                moveStore();
                break;
        }

        while (drop) {
            drop = moveDownOneChecked();
        }
    }

    int TetrisGame::getScore() const {
        return score;
    }

    TetrisGame::TetrisGameState TetrisGame::getState() const {
        return state;
    }
};
