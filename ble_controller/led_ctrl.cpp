#include "led_ctrl.hpp"

Ticker ticker;
DigitalOut led1(LED1);
DigitalOut led2(LED2);

static void blink() {
    led1 = !led1;
}

void start_blinking(std::chrono::milliseconds rate) {
    ticker.attach(blink, rate);
}

void cancel_blinking() {
    ticker.detach();
    led1 = 1;
}

// display player number on leds (0-3)
void disp_player(int num) {
    cancel_blinking();
    switch (num) {
    case 0:
        led1 = 0;
        led2 = 0;
    case 1:
        led1 = 0;
        led2 = 1;
    case 2:
        led1 = 1;
        led2 = 0;
    case 3:
        led1 = 1;
        led2 = 1;
    }

    printf("Connected as player %d/n", num + 1);
}