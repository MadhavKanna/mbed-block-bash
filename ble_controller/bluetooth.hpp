#ifndef BLUE_HPP
#define BLUE_HPP
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "mbed.h"

#include "errors.hpp"

#define CONTROLLER_NAME "BlockBashController"

extern EventQueue queue;

extern uint16_t customServiceUUID;
extern uint16_t readCharUUID;
extern uint16_t writeCharUUID;

static uint8_t readValue[10] = {0};
extern ReadOnlyArrayGattCharacteristic<uint8_t, sizeof(readValue)> readChar;

static uint8_t writeValue[10] = {0};
extern WriteOnlyArrayGattCharacteristic<uint8_t, sizeof(writeValue)> writeChar;

extern GattService customService;

static const ble::AdvertisingParameters advertising_params(
    ble::advertising_type_t::CONNECTABLE_UNDIRECTED,
    ble::adv_interval_t(ble::millisecond_t(25)),
    ble::adv_interval_t(ble::millisecond_t(50))
);

void writeCharCallback(const GattWriteCallbackParams *params);

void advertise();

void onInitComplete(BLE::InitializationCompleteCallbackContext *event);

void scheduleBleEvents(BLE::OnEventsToProcessCallbackContext *context);

class GapHandler : public ble::Gap::EventHandler {
public:
    virtual void onAdvertisingStart(const ble::AdvertisingStartEvent &event);
    virtual void onConnectionComplete(const ble::ConnectionCompleteEvent &event);
    virtual void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event);
};

int bleInit();

#endif