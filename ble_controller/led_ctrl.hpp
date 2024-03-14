#ifndef LED_HPP
#define LED_HPP
#include "mbed.h"

static void blink();

void start_blinking(std::chrono::milliseconds rate);

void cancel_blinking();

// display player number on leds (0-3)
void disp_player(int num);

#endif