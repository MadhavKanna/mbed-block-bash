#ifndef LED_HPP
#define LED_HPP
#include "mbed.h"

static void blink();

void startBlinking(std::chrono::milliseconds rate);

void cancelBlinking();

// display player number on leds (0-3)
void dispPlayer(int num);

#endif