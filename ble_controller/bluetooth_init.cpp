#include "bluetooth.hpp"

void onInitComplete(BLE::InitializationCompleteCallbackContext *event) {
    if (event->error) {
        print_error(event->error, "Error during the initialization");
        return;
    }

    BLE &ble = BLE::Instance();
    auto &gap = ble.gap();

    // Setup the default PHY used in connection to 2M to reduce power consumption
    if (gap.isFeatureSupported(ble::controller_supported_features_t::LE_2M_PHY)) {
        ble::phy_set_t phys(false, true, false);

        ble_error_t error = gap.setPreferredPhys(&phys, &phys);
        if (error) {
            print_error(error, "GAP::setPreferedPhys failed");
        }
    }

    /* Print out device MAC address to the console */
    ble::own_address_type_t addr_type;
    ble::address_t address;
    BLE::Instance().gap().getAddress(addr_type, address);
    printf("MAC ADDRESS: ");
    printf("%02x:%02x:%02x:%02x:%02x:%02x\r\n",
           address[5], address[4], address[3], address[2], address[1], address[0]);

    ble_error_t error = gap.setAdvertisingParameters(
        ble::LEGACY_ADVERTISING_HANDLE, advertising_params);
    if (error) {
        print_error(error, "Gap::setAdvertisingParameters() failed");
        return;
    }

    ble::AdvertisingDataSimpleBuilder<ble::LEGACY_ADVERTISING_MAX_SIZE> data_builder;
    data_builder.setFlags();
    data_builder.setName(CONTROLLER_NAME);
    data_builder.setLocalService(customServiceUUID);

    error  = gap.setAdvertisingPayload(ble::LEGACY_ADVERTISING_HANDLE,
                                            data_builder.getAdvertisingData());
    if (error) {
        print_error(error, "Could not set advertising payload");
        return;
    }

    queue.call(advertise);
}

void scheduleBleEvents(BLE::OnEventsToProcessCallbackContext *context) {
    queue.call(Callback<void()>(&context->ble, &BLE::processEvents));
}
