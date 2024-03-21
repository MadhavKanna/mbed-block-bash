#include "controller.hpp"
#include "bluetooth.hpp"
#include "mbed.h"

// THIS CODE IS FOR TESTING PURPOSES AND WILL BE REPLACED ONCE GESTURE DETECTION IS DONE

InterruptIn button(BUTTON1);
bool enable = false;

void button_handler()
{
    static int action = 1;
    if (enable) {
        send_input((TetrisAction) action);
        queue.call(printf, "sent to read characterisitc: %d\n", (int) action);
        action++;
    }
}

void controller_init() {
    button.fall(&button_handler);
}

void start_tracking_controller() {
    enable = true;
    puts("Tracking on");
}

void stop_tracking_controller() {
    enable = false;
    puts("Tracking off");
}
