#include "bluetooth.hpp"
#include "led_ctrl.hpp"
#include "controller.hpp"

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
    controller_init();
    cancelBlinking();
}

void GapHandler::onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event) {
    printf("Disconnected from %u because %u.\n", event.getConnectionHandle(), event.getReason());
    stop_tracking_controller();
    startBlinking(500ms);
    queue.call(advertise);
}
