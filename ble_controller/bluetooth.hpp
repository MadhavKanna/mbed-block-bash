#ifndef BLUE_HPP
#define BLUE_HPP
#include "ble/BLE.h"
#include "ble/Gap.h"

#include "errors.hpp"
#include "led_ctrl.hpp"

static const ble::AdvertisingParameters advertising_params(
    // CTRL+Click on the advertising type below to see other types.
    ble::advertising_type_t::CONNECTABLE_UNDIRECTED,
    ble::adv_interval_t(ble::millisecond_t(25)),
    ble::adv_interval_t(ble::millisecond_t(50))
);

void advertise();

void on_init_complete(BLE::InitializationCompleteCallbackContext *event);

void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context);

class GapHandler : private mbed::NonCopyable<GapHandler>, public ble::Gap::EventHandler
{
public:
    /**
     * @brief Called when the device starts advertising itself to others.
     */
    void onAdvertisingStart(const ble::AdvertisingStartEvent &event) override;

    /**
     * @brief Called when another device connects to ours.
     */
    void onConnectionComplete(const ble::ConnectionCompleteEvent &event) override;

    /**
     * @brief Called when another connected evice disconnects from ours.
     */
    void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event) override;
};

int ble_init();

/**
 * @brief A simplified implementation of a BLE Battery Service.
 *
 * The amount of battery in the device is "fake", and can be controlled through
 * events.
 */
class BatteryService
{
public:
    /**
     * @brief Construct a new battery service object.
     *
     * @param initial_charge The initial charge of the battery.
     */
    BatteryService(uint8_t initial_charge);

private:
    /**
     * @brief A value between 0 and 100, inclusive.
     *
     * Represents the percent charge of the battery.
     */
    uint8_t _charge;

    /**
     * @brief The GATT Characteristic that communicates the battery level.
     */
    ReadOnlyGattCharacteristic<uint8_t> _charge_characteristic;
};

#endif