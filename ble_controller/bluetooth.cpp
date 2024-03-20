#include "bluetooth.hpp"

EventQueue queue;

uint16_t customServiceUUID  = 0xA000;
uint16_t readCharUUID       = 0xA001;  // Controller write here
uint16_t writeCharUUID      = 0xA002;  // Console write to here

ReadOnlyArrayGattCharacteristic<uint8_t, sizeof(readValue)> readChar(readCharUUID, readValue);
WriteOnlyArrayGattCharacteristic<uint8_t, sizeof(writeValue)> writeChar(writeCharUUID, writeValue);

GattCharacteristic *characteristics[] = {&readChar, &writeChar};
GattService customService(customServiceUUID, characteristics, sizeof(characteristics) / sizeof(GattCharacteristic *));

void advertise() {
    BLE &ble = BLE::Instance();
    auto &gap = ble.gap();

    ble_error_t error = gap.startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
    if (error) {
        print_error(error, "Gap::startAdvertising() failed");
        return;
    }
}


int bleInit() {
    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(scheduleBleEvents);

    // Initialize BLE and then call our own function
    ble_error_t error = ble.init(&onInitComplete);
    if (error) {
        print_error(error, "Error returned by BLE::init");
        return 1;
    }

    GapHandler handler;
    auto &gap = ble.gap();
    gap.setEventHandler(&handler);

    ble.gattServer().addService(customService);
    ble.gattServer().onDataWritten(writeCharCallback);

    queue.dispatch_forever();
    return 0;
}