/**
 * @file main.cpp
 * @author Mario Badr, Lucas Wilkinson
 * @version 20221
 * @copyright Copyright (c) 2022 Mario Badr
 *
 * @brief BLE and different services.
 */

#include <iostream>
#include <unordered_map>
#include "controller.h"

#include "TetrisManager.h"
#include "TetrisRenderer.h"
#include "TetrisAction.h"

/**
 * @brief The main event queue used in our program.
 */
EventQueue queue;

InterruptIn button(BUTTON1);

static bool should_start_game = false;

class BlockBashGame {
public:
    BlockBashGame(BLE &ble, EventQueue &queue)
        : num_games(0),
          ble(ble),
          event_queue(queue),
          renderer(),
          game_manager(renderer),
          controller_set(),
          connection_manager(event_queue, ble, controller_set)
    {
        connection_manager.start();
        queue.call([this]{
            // printf(" called setup \r\n");
            setup_controllers();
        });
    }

    void setup_controllers() {
        // printf(" %d controllers connected \r\n", num_games);
        auto conn_ids = this->controller_set.get_valid_controllers();
        if (num_games == 3 || should_start_game) return;
        for (auto id: conn_ids) {
            // printf(" controller id %d \r\n", id);
            auto it = this->conn_to_game.find(id);
            if (it != this->conn_to_game.end()) continue;

            this->game_to_conn.emplace(num_games, id);
            this->conn_to_game.emplace(id, num_games);
            num_games++;
            this->game_manager.addGame();
            if (num_games == 3) return;
        }
        queue.call_in(2000ms, [this]{
            setup_controllers();
        });
    }

    void run_game_frame() {
        using namespace Tetris;

        for (int game = 0; game < num_games; game++) {
            auto controller_id = this->game_to_conn.at(game);
            auto pair = this->controller_set.dequeue_from_controller(controller_id);
            auto action = pair.first;
            auto magnitude = pair.second;

            // printf(" action is: %d\r\n", action);

            switch (action) {
            case Action::NoOp:
                continue;
            
            case Action::Left:
                game_manager.pushAction(game, TetrisAction::MoveLeft);
                break;
            
            case Action::Right:
                game_manager.pushAction(game, TetrisAction::MoveRight);
                break;
            
            case Action::Down:
                game_manager.pushAction(game, TetrisAction::Drop);
                break;
            
            case Action::FlipRight:
                game_manager.pushAction(game, TetrisAction::Rotate);
                break;

            case Action::Save:
                game_manager.pushAction(game, TetrisAction::Store);
                break; 

            default:
                // other operations not supported yet
                continue;
            }
        }

        // run tick for all games
        game_manager.runTick();
    }

    void start_game() {
        this->connection_manager.ready_controllers();
        this->game_manager.playGame();
    }

private:
    int num_games;
    unordered_map<int, int> game_to_conn;
    unordered_map<int, int> conn_to_game;
    EventQueue &event_queue;
    BLE &ble;
    TetrisRenderer renderer;
    TetrisGameManager game_manager;
    ControllerSet controller_set;
    ControllerConnectionHandler connection_manager;
};

BlockBashGame *game;

void button1_push_handler()
{
    if (game == nullptr) return;
    if (should_start_game) return;
    should_start_game = true;
    queue.call([] {
        if (game == nullptr) return;
        game->start_game();
    });
    queue.call_in(5000ms, [] {
        queue.call_every(1000ms, [] {
            if (game == nullptr) return;
            game->run_game_frame();
        });
    });
    
}

int main()
{
    // printf("\r\n starting \r\n");
    BLE &ble = BLE::Instance();

    if (ble.hasInitialized()) {
        ble.shutdown();
    }

    game = new BlockBashGame(ble, queue);
    button.fall(&button1_push_handler);

    // printf(" this runs \r\n");

    queue.dispatch_forever();
    return 0;
}
