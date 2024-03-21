#include "bluetooth.hpp"
#include "led_ctrl.hpp"
#include "controller.hpp"

void setPlayerNum(uint8_t num) {
    dispPlayer(num);
}

void handleConsoleReady() {
    puts("ready");
    start_tracking_controller();
}

void handleConsolePaused() {
    puts("pause");
    stop_tracking_controller();
}

void send_input(TetrisAction action) {
    uint8_t data = (uint8_t) action;
    BLE::Instance().gattServer().write(readChar.getValueHandle(), &data, sizeof(uint8_t));
}

void writeCharCallback(const GattWriteCallbackParams *params) {
    if (params->handle == writeChar.getValueHandle()) {
        printf("Data received: length = %d, data = 0x", params->len);
        for (int x = 0; x < params->len; x++) {
            printf("%x", params->data[x]);
        }
        printf("\n\r");

        switch(params->data[0]) {
            case player:
                setPlayerNum(params->data[1]);
                break;
            case ready:
                handleConsoleReady();
                break;
            case paused:
                handleConsolePaused();
                break;
            default:
                break; // Skip unknown data type
        }

        BLE::Instance(BLE::DEFAULT_INSTANCE).gattServer().write(readChar.getValueHandle(), params->data, params->len);
    }
}
