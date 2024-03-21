#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

typedef unsigned char uint8_t;

enum controller_data{  // data stored in highest order byte
    player  = 0x15,  // Player number stored second highest order byte
    ready   = 0x20,
    paused  = 0x40
};

enum class TetrisAction{
    MoveLeft, // move the block one column to right
    MoveRight, // move the block one column to left
    Drop, // Drop the block
    Store,  // Store the block
    Rotate  // Rotate the block clockwise
};

/** 
 * @brief Initialize the gyroscope, button and time of flight sensor
 */ 
void controller_init();

/** 
 *  @brief Send the input. This os only called when a user move has occurred
 *  
 */ 
void send_input(enum TetrisAction action);

/**
 * @brief start polling the sensors and processing data for any user moves
 * call send_input if a user move is detected
 */
void start_tracking_controller();

void stop_tracking_controller();

void setPlayerNum(uint8_t num);

void handleConsoleReady();

void handleConsolePaused();



#endif