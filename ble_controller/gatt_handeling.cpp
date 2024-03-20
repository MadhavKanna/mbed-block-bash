#include "bluetooth.hpp"
#include "led_ctrl.hpp"
#include "controller.hpp"

void setPlayerNum(uint8_t num) {
    dispPlayer(num);
}

void handleConsoleReady() {
    puts("ready");
}

void handleConsolePaused() {
    puts("pause");
    BLE &ble = BLE::Instance();
    // ble.waitForEvent(); /* Save power */
    sleep(); // https://os.mbed.com/docs/mbed-os/v6.16/feature-i2c-doxy/class_b_l_e.html#a389bf72fbc5339a27ade7c065a507787
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

        // BLE::Instance(BLE::DEFAULT_INSTANCE).gattServer().write(readChar.getValueHandle(), params->data, params->len);
    }
}
