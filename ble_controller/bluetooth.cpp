#include "bluetooth.hpp"

EventQueue queue;

// BatteryService::BatteryService(uint8_t initial_charge) :
//         _charge(initial_charge),
//         _charge_characteristic(
//             0xa001,
//             &initial_charge,
//             GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
//         )
// {
//     // Add this new service. This should only happen once.
//     BLE &ble = BLE::Instance();

//     GattCharacteristic *characteristics[] = { &_charge_characteristic };
//     GattService battery_service(
//         0xa000,
//         characteristics,
//         sizeof(characteristics) / sizeof(characteristics[0])
//     );

//     ble.gattServer().addService(battery_service);
// }

uint16_t customServiceUUID  = 0xA000;
uint16_t readCharUUID       = 0xA001;

static uint8_t readValue[10] = {0};
ReadOnlyArrayGattCharacteristic<uint8_t, sizeof(readValue)> readChar(readCharUUID, readValue);

GattCharacteristic *characteristics[] = {&readChar};
GattService customService(customServiceUUID, characteristics, sizeof(characteristics) / sizeof(GattCharacteristic *));

void advertise()
{
    BLE &ble = BLE::Instance();
    auto &_gap = ble.gap();

    ble_error_t error = _gap.setAdvertisingParameters(
        ble::LEGACY_ADVERTISING_HANDLE, advertising_params);
    if (error) {
        print_error(error, "Gap::setAdvertisingParameters() failed");
        return;
    }

    ble::AdvertisingDataSimpleBuilder<ble::LEGACY_ADVERTISING_MAX_SIZE> data_builder;
    data_builder.setFlags();

    data_builder.setName("GattServer");
    data_builder.setLocalService(0xa000); // Service UUID here

    error  = _gap.setAdvertisingPayload(ble::LEGACY_ADVERTISING_HANDLE,
                                            data_builder.getAdvertisingData());
    if (error) {
        print_error(error, "Could not set advertising payload");
        return;
    }

    error = _gap.startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
    if (error) {
        print_error(error, "Gap::startAdvertising() failed");
        return;
    }

}

void print_address(const ble::address_t &addr)
{
    printf("%02x:%02x:%02x:%02x:%02x:%02x\r\n",
           addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);
}

void on_init_complete(BLE::InitializationCompleteCallbackContext *event)
{
    if (event->error) {
        print_error(event->error, "Error during the initialisation");
        return;
    }

    BLE &ble = BLE::Instance();
    auto &gap = ble.gap();

    // Setup the default phy used in connection to 2M to reduce power consumption
    if (gap.isFeatureSupported(ble::controller_supported_features_t::LE_2M_PHY)) {
        ble::phy_set_t phys(false, true, false);

        ble_error_t error = gap.setPreferredPhys(&phys, &phys);
        if (error) {
            print_error(error, "GAP::setPreferedPhys failed");
        }
    }

    /* Print out device MAC address to the console*/
    ble::own_address_type_t addr_type;
    ble::address_t address;
    BLE::Instance().gap().getAddress(addr_type, address);
    printf("DEVICE MAC ADDRESS: ");
    print_address(address);

    queue.call(advertise);
}

void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context)
{
    queue.call(Callback<void()>(&context->ble, &BLE::processEvents));
}

void GapHandler::onAdvertisingStart(const ble::AdvertisingStartEvent &event)
{
    puts("Searching...");
    start_blinking(100ms);
}

void GapHandler::onConnectionComplete(const ble::ConnectionCompleteEvent &event)
{
    if (event.getStatus() != BLE_ERROR_NONE) {
        print_error(event.getStatus(), "Connection failed");
        return; // We should try again after
    }

    printf("Connected to %u.\n", event.getConnectionHandle());
    cancel_blinking();
}

void GapHandler::onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event)
{
    printf("Disconnected from %u because %u.\n", event.getConnectionHandle(), event.getReason());
    start_blinking(500ms);
    queue.call(advertise); // Try again
}

int ble_init() {
    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(schedule_ble_events);

    // Initialize BLE and then call our own function
    ble_error_t error = ble.init(&on_init_complete);
    if (error) {
        print_error(error, "Error returned by BLE::init");
        return 0;
    }

    GapHandler handler;
    auto &gap = ble.gap();
    gap.setEventHandler(&handler);
    
    BatteryService battery_service(100);

    queue.dispatch_forever();
    return 0;
}