#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

typedef unsigned char uint8_t;

enum controller_data{  // data stored in highest order byte
    player = 0x15,  // Player number stored second highest order byte
    ready = 0x20,
    paused = 0x40
};

void setPlayerNum(uint8_t num);

void handleConsoleReady();

void handleConsolePaused();

#endif