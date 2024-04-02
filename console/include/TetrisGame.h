#pragma once

#include "TetrisAction.h"

#include <iostream>
#include <array>
#include <set>
#include <vector>
#include <utility> // for std::pair
#include <algorithm> // for std::min_element
#include <random>
#include <cstdlib>
#include <ctime>

// Define the size of the game board
constexpr int WIDTH = 10;
constexpr int HEIGHT = 20;

using namespace Tetris;

namespace Tetris {
    struct Square {
        int y;
        int x;
    };
    
    using FallingPiece = std::vector<Square>;

    // A board starts at 0,0 in top left corner: X is horizontal and Y is vertical
    using TetrisBoard = std::array<std::array<int, WIDTH>, HEIGHT>;

    const std::vector<FallingPiece> pieces {
        {{0, 0}, {1, 0}, {0, 1}, {1, 1}}, // Square
        {{0, 0}, {1, 0}, {2, 0}, {3, 0}}, // Line
        {{0, 0}, {1, 0}, {2, 0}, {1, 1}}, // T
        {{0, 0}, {1, 0}, {1, 1}, {2, 1}}, // S
        {{0, 0}, {1, 0}, {2, 0}, {2, 1}}, // L
        {{0, 1}, {1, 1}, {2, 1}, {2, 0}}  // J
    };

    enum TetrisPiece {
        PIECE_None = -1,
        PIECE_Square = 0,
        PIECE_Line = 1,
        PIECE_T = 2,
        PIECE_S = 3,
        PIECE_L = 4,
        PIECE_J = 5,
    };

    class TetrisGame {
    public:
        enum class TetrisGameState {
            Ready,
            Playing,
            GameOver
        };

        static int gameStateToInt(TetrisGameState state) {
            switch (state) {
                case TetrisGameState::Ready:
                    return 0;
                case TetrisGameState::Playing:
                    return 1;
                case TetrisGameState::GameOver:
                    return 2;
            }
        }
    private:
        // Game board
        TetrisBoard board {};

        TetrisPiece currentPieceType {PIECE_None};
        FallingPiece currentPiece{};


        TetrisPiece stored {PIECE_None};
        bool swapped {false};
        int score {0};

        // std::mt19937* gen;
        // std::uniform_int_distribution<std::mt19937::result_type>* dist6;

        TetrisGameState state {TetrisGameState::Ready};


        static FallingPiece rotatePiece(const FallingPiece& piece);

        /**
         * Move the current piece down by one block
         * 
         * @return The new position of the piece
        */
        FallingPiece shiftDownOne();

        /**
         * Move the current piece down by one block.
         * If the piece cannot move down, place it and spawn next.
         * 
         * @return true if the piece was moved, false if placed
        */
        bool moveDownOneChecked();

        /**
         * Spawns a new piece at the top of the board
        */
        void spawnPiece(TetrisPiece pieceType = PIECE_None);

        /**
         * Places the current piece on the board
        */
        void placePiece();

        /**
         * remove the given row from the board
        */
        void removeRows(const std::set<int>& rowNums);

        /**
         * Move the current piece left by one block
         * 
         * @return True if the piece was moved, false otherwise
        */
        bool moveLeft();

        /**
         * Move the current piece right by one block
         * 
         * @return True if the piece was moved, false otherwise
        */
        bool moveRight();

        /**
         * Rotate the current piece
         * 
         * @return True if the piece was rotated, false otherwise
        */
        bool moveRotate();

        /**
         * Store the current piece, swaps for an existing one is stored
         * 
         * @return True if the piece was stored, false otherwise
        */
        bool moveStore();

        /**
         * Check if the current piece is colliding with the board
         * 
         * @return True if the piece is colliding, false otherwise
        */
        bool checkCollision(const FallingPiece& piece);


    public:
        TetrisGame() {
            // Initialize the game board with zeros
            for (auto& row : board) {
                row.fill(0);
            }
            // std::random_device* rd = new std::random_device();
            // gen = new std::mt19937(*rd);
            // dist6 = new std::uniform_int_distribution<std::mt19937::result_type>(0, 5);
            srand(time(0));
        }

        void start();

        void applyAction(TetrisAction action);

        /**
         * Run a single tick of the game
        */
        void tick();


        /**
         * Add the moving piece to a copy of game board
        */
        TetrisBoard getViewBoard() const;

        /**
         * Returns the current solid board state (ignoring falling pieces)
        */
        const TetrisBoard& getBoard() const;

        /**
         * Returns the current stored piece
        */
        TetrisPiece getStoredPiece() const;

        /**
         * Returns the current score
        */
        int getScore() const;

        /**
         * Returns the current game state
        */
        TetrisGameState getState() const;
    };

};
