#include "bluetooth.hpp"
#include "led_ctrl.hpp"

void GapHandler::onAdvertisingStart(const ble::AdvertisingStartEvent &event) {
    puts("Searching...");
    startBlinking(100ms);
}

void GapHandler::onConnectionComplete(const ble::ConnectionCompleteEvent &event) {
    if (event.getStatus() != BLE_ERROR_NONE) {
        print_error(event.getStatus(), "Connection failed");
        return;
    }

    printf("Connected to %u.\n", event.getConnectionHandle());
    cancelBlinking();
}

void GapHandler::onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event) {
    printf("Disconnected from %u because %u.\n", event.getConnectionHandle(), event.getReason());
    startBlinking(500ms);
    queue.call(advertise);
}
