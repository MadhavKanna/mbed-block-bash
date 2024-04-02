#include <cstdint>
#include <unordered_map>
#include <map>
#include <utility>
#include <iostream>
#include <deque>
#include <vector>

#include "mbed.h"
#include "ble/BLE.h"

#include <events/mbed_events.h>

#include "platform/Callback.h"
#include "platform/NonCopyable.h"

#include "Gap.h"
#include "gap/AdvertisingDataParser.h"
#include "ble/common/FunctionPointerWithContext.h"

#include "pretty_printer.h"

static DiscoveredCharacteristic gesture_characteristic;
static bool gesture_characteristic_found = false;

const static uint16_t ControllerServiceUUID = 0xA000;
const static uint16_t GestureCharacteristicUUID = 0xA001;
const static uint16_t SignalCharacteristicUUID = 0xA002;

static const uint16_t MaxAdvPayloadSize = 50;

// Enum representing actions
enum class Action { 
    Down, 
    Left, 
    Right, 
    Save, 
    FlipRight, 
    FlipLeft, 
    Pause, 
    NoOp 
};

enum class ControllerSignal : uint8_t {
    PlayerId  = 0x15,
    ReadyState   = 0x20,
    PausedState  = 0x40
};

Action parse_action(uint8_t num)
{
    switch (num) {
    case 0x01:
        return Action::Left;
    
    case 0x02:
        return Action::Right;
    
    case 0x03:
        return Action::Down;
    
    case 0x04:
        return Action::Save;
    
    case 0x05:
        return Action::FlipRight;

    default:
        return Action::NoOp;
    }
}

class Controller {
private:
    bool connected;
    bool valid;
    int id;
    std::deque<std::pair<Action, uint8_t>> action_queue;

public:

    Controller(int id) 
        : connected(true), id(id), valid(false)
    {}

    bool is_connected() const 
    {
        return connected;
    }

    bool is_valid() const
    {
        return valid;
    }

    int get_id() const 
    {
        return id;
    }

    void set_connection_status(bool connected)
    {
        this->connected = connected;
    }

    void set_as_valid()
    {
        this->valid = true;
    }

    void enqueue_action(Action action, uint8_t magnitude)
    {
        this->action_queue.emplace_front(action, magnitude);
    }

    std::pair<Action, uint8_t> dequeue_action()
    {
        if (!action_queue.empty()) {
            auto first_action = action_queue.front();
            action_queue.pop_front();
            return first_action;
        } else {
            // std::cout << "Action queue is empty!" << std::endl;
            return std::make_pair(Action::NoOp, 0); // Default action when the queue is empty
        }
    }
};

class ControllerSet {
private:
    std::unordered_map<int, Controller> controllers;

    Controller* find_controller(int id)
    {
        auto it = controllers.find(id);
        if (it != controllers.end()) {
            return &it->second;
        } else {
            return nullptr;
        }
    }
public:
    void make_controller(int id)
    {
        Controller new_controller(id);
        controllers.emplace(id, new_controller);
    }

    void queue_to_controller(int id, Action action, uint8_t magnitude)
    {
        Controller* controller = find_controller(id);
        if (controller) {
            controller->enqueue_action(action, magnitude);
        } else {
            // std::cout << "Player with ID " << id << " not found!" << std::endl;
        }
    }

    std::pair<Action, uint8_t> dequeue_from_controller(int controller_id)
    {
        Controller* controller = find_controller(controller_id);
        return controller->dequeue_action();
    }

    void disconnect_controller(int controller_id)
    {
        Controller* controller = find_controller(controller_id);
        controller->set_connection_status(false);
    }

    void reconnect_controller(int controller_id)
    {
        Controller* controller = find_controller(controller_id);
        controller->set_connection_status(true);
    }

    bool is_controller_connected(int controller_id)
    {
        Controller* controller = find_controller(controller_id);
        return controller->is_connected();
    }

    void validate_controller(int controller_id)
    {
        Controller* controller = find_controller(controller_id);
        controller->set_as_valid();
    }

    std::vector<int> get_valid_controllers()
    {
        std::vector<int> valid_controllers;

        for (auto pair: controllers) {
            if (pair.second.is_valid()) {
                valid_controllers.push_back(pair.first);
            }
        }

        return valid_controllers;
    }
};


struct CompareMacAddress {
    bool operator()(const ble::address_t &a, const ble::address_t &b) const
    {
        // printf("Size of data is: %d\n", a.size());
        // printf("Address of A: ");
        // print_address(a);
        // printf("Address of B: ");
        // print_address(b);
        auto diff = std::memcmp(a.data(), b.data(), a.size());
        // printf("The difference is %d\n", diff);
        return (diff < 0);
    }
};




class ControllerConnectionHandler 
    : private mbed::NonCopyable<ControllerConnectionHandler>, 
      public ble::Gap::EventHandler
{
    using AdvertisingHandle = ble::advertising_handle_t;
    using ConnectionHandle = ble::connection_handle_t;
    using MacAddress = ble::address_t;

    template<typename Value>
    using MacAddressTable = std::map<MacAddress, Value, CompareMacAddress>;

protected:
    events::EventQueue &queue;
    BLE &ble;
    ble::Gap &gap;
    ble::GattClient &gatt;
    ControllerSet &controller_set;
    MacAddressTable<int> mac_to_id;
    std::map<ConnectionHandle, MacAddress> conn_to_mac;
    MacAddressTable<DiscoveredCharacteristic> mac_to_read_char;
    MacAddressTable<DiscoveredCharacteristic> mac_to_write_char;

    int num_connections;

    uint8_t advertising_buff[MaxAdvPayloadSize];
    ble::AdvertisingDataBuilder data_builder;

    AdvertisingHandle advertising_handle = ble::LEGACY_ADVERTISING_HANDLE;

    bool is_connecting = false;
public:
    /**
     * Construct a BLEProcess from an event queue and a ble interface.
     * Call start() to initiate ble processing.
     */
    ControllerConnectionHandler(
            events::EventQueue &event_queue,
            BLE &ble_interface,
            ControllerSet &controller_set) :
        queue(event_queue),
        ble(ble_interface),
        gap(ble_interface.gap()),
        gatt(ble_interface.gattClient()),
        controller_set(controller_set),
        data_builder(advertising_buff),
        num_connections(0)
    {
    }

    ~ControllerConnectionHandler()
    {
        stop();
    }

    void halt_controllers()
    {
        for (auto pair: this->conn_to_mac) {
            auto it = this->mac_to_write_char.find(pair.second);
            if (it != this->mac_to_write_char.end()) {
                auto characteristic = it->second;
                uint8_t value = (uint8_t) ControllerSignal::PausedState;
                characteristic.write(1, &value);
            }
        }
    }

    void ready_controllers()
    {
        for (auto pair: this->conn_to_mac) {
            auto it = this->mac_to_write_char.find(pair.second);
            if (it != this->mac_to_write_char.end()) {
                auto characteristic = it->second;
                uint8_t value = (uint8_t) ControllerSignal::ReadyState;
                characteristic.write(1, &value);
                // printf(" ready signal sent \r\n");
            }
        }
    }

    void start()
    {
        // printf("Controller Handler started.\r\n");

        if (ble.hasInitialized()) {
            printf("Error: the ble instance has already been initialized.\r\n");
            return;
        }

        /* handle gap events */
        gap.setEventHandler(this);

        /* This will inform us off all events so we can schedule their handling
         * using our event queue */
        ble.onEventsToProcess(
            makeFunctionPointer(this, 
                &ControllerConnectionHandler::schedule_ble_events)
        );

        ble_error_t error = ble.init(
            this, &ControllerConnectionHandler::on_init_complete
        );

        if (error) {
            print_error(error, "Error returned by BLE::init.\r\n");
            return;
        }

        return;
    }

    void stop()
    {
        if (ble.hasInitialized()) {
            ble.shutdown();
            // printf("Controller Handler has stopped.\r\n");
        }
    }

    const char* name()
    {
        static const char name[] = "BlockBashConsole";
        return name;
    }

    const char* controller_name()
    {
        static const char name[] = "BlockBashController";
        return name;
    }

protected:
    void on_init_complete(BLE::InitializationCompleteCallbackContext *event)
    {
        if (event->error) {
            print_error(event->error, "Error during the initialisation\r\n");
            return;
        }

        // printf("Ble instance initialized\r\n");

        /* TODO: THIS RUNS ONCE THE BLE INITIALIZATION IS COMPLETED */

        /* All calls are serialised on the user thread through the event queue */
        start_activity();

        this->start_gatt();
    }

    void onScanTimeout(const ble::ScanTimeoutEvent &event) override {
        start_activity();
    }

    void onConnectionComplete(const ble::ConnectionCompleteEvent &event) override
    {
        /* TODO: ONCE A CONNECTION IS SETUP, THIS RUNS */
        if (event.getStatus() == BLE_ERROR_NONE) {
            // printf("\r\nConnected to: \r\n");

            auto addr = event.getPeerAddress();
            // print_address(addr);

            // If the mac address is not already present, create a new
            // connection
            if (this->mac_to_id.find(addr) != this->mac_to_id.end()) {
                // printf("Mac Address found!\n");
                this->conn_to_mac.emplace(event.getConnectionHandle(), addr);
            } // else store the new connection handle 
            else {
                // printf("Mac Address not found!\n");
                num_connections++;
                this->mac_to_id.emplace(addr, num_connections);
                this->controller_set.make_controller(num_connections);
                // printf("Connection handle %d\n", event.getConnectionHandle());
                this->conn_to_mac.emplace(event.getConnectionHandle(), addr);

                // create the callback that reads the data
                queue.call_every(1000ms, [this, addr] {
                    auto it = this->mac_to_read_char.find(addr);
                    if (it != this->mac_to_read_char.end()) {
                        auto characteristic = it->second;
                        characteristic.read();
                        // printf("This is still getting called...\r\n");
                    }
                });
                queue.call_in(5000ms, [this, addr] {
                    auto it = this->mac_to_write_char.find(addr);
                    if (it != this->mac_to_write_char.end()) {
                        auto characteristic = it->second;
                        uint8_t value[2];
                        value[0] = (uint8_t) ControllerSignal::PlayerId;
                        value[1] = (uint8_t) this->num_connections - 1;
                        characteristic.write(2, (uint8_t *) &value);
                        // printf("Signal sent...\r\n");
                    }
                });
            }

            // printf("We are now connected to %d players\r\n", num_connections);

            this->queue.call([this, event] { this->start_discovery(event); } );
            this->start_activity();
        } else {
            // printf("Failed to connect\r\n");
            start_activity();
        }
    }

    void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event) override
    {
        /* THIS RUNS ONCE WE DISCONNECT */
        const auto &handle = event.getConnectionHandle();
        // printf("Connection handle %d\n", handle);
        auto it = this->conn_to_mac.find(handle);
        auto addr = it->second;

        if (it == this->conn_to_mac.end()) {
            // printf("Handle not found! \n");
        }

        // printf("Size of map is: %d\n", this->mac_to_id.size());
        // printf("%s", "Address associated with this handle is: ");
        // print_address(addr);

        auto it2 = this->mac_to_id.find(addr);

        if (it2 == this->mac_to_id.end()) {
            // printf("Address not found! \n");
        }

        auto controller_id = it2->second;
        
        // printf("Controller ID: %d\n", player_id);

        this->controller_set.disconnect_controller(controller_id);
        this->conn_to_mac.erase(handle);
        this->mac_to_read_char.erase(addr);
        this->mac_to_write_char.erase(addr);

        // printf("Disconnected from controller %d\r\n", controller_id);
        start_activity();
    }

    void onAdvertisingEnd(const ble::AdvertisingEndEvent &event) override
    {
        start_activity();
    }

    void onAdvertisingReport(const ble::AdvertisingReportEvent &event) override {
        /* don't bother with analysing scan result if we're already connecting */
        // printf("Something\n");
        // printf("Is connecting %d\n", is_connecting);
        if (is_connecting) {
            return;
        }

        // printf("Got here\n");
        // printf("Is connectable?: %d\n", event.getType().connectable());
        // if (!event.getType().connectable()) {
        //     /* we're only interested in connectable devices */
        //     return;
        // }

        ble::AdvertisingDataParser adv_data(event.getPayload());

        /* parse the advertising payload, looking for a discoverable device */
        while (adv_data.hasNext()) {
            ble::AdvertisingDataParser::element_t field = adv_data.next();

            // printf("Found a: %s\r\n", field.value.data());
            /* connect to a discoverable device */
            if (field.type == ble::adv_data_type_t::COMPLETE_LOCAL_NAME) {
                //printf("Found a: %s\r\n", field.value.data());
                if (field.value.size() == strlen(controller_name()) &&
                    (memcmp(field.value.data(), controller_name(), field.value.size()) == 0)) {

                    // printf("We found \"%s\", connecting...\r\n", controller_name());

                    ble_error_t error = ble.gap().stopScan();

                    if (error) {
                        print_error(error, "Error caused by Gap::stopScan");
                        return;
                    }

                    const ble::ConnectionParameters connection_params;

                    error = ble.gap().connect(
                        event.getPeerAddressType(),
                        event.getPeerAddress(),
                        connection_params
                    );

                    if (error) {
                        gap.startScan();
                        return;
                    }

                    /* we may have already scan events waiting
                     * to be processed so we need to remember
                     * that we are already connecting and ignore them */
                    is_connecting = true;

                    return;
                }
            }
        }
    }

    void start_activity()
    {
        // start scanning only once
        static bool scan = true;
        // static bool scan = false;
        if (scan) {
            queue.call([this]() { start_scanning(); });
        } else {
            queue.call([this]() { start_advertising(); });
        }
        scan = !scan;
        // scan = true;
        is_connecting = false;
    }

    void start_scanning()
    {
        ble::ScanParameters scan_params;
        gap.setScanParameters(scan_params);
        ble_error_t ret = gap.startScan(ble::scan_duration_t(ble::millisecond_t(5000)));
        if (ret == ble_error_t::BLE_ERROR_NONE) {
            // printf("Started scanning for \"%s\"\r\n", controller_name());
        } else {
            // printf("Starting scan failed\r\n");
        }
    }

    void start_advertising()
    {
        ble_error_t error;

        if (gap.isAdvertisingActive(advertising_handle)) {
            /* we're already advertising */
            return;
        }

        ble::AdvertisingParameters adv_params(
            ble::advertising_type_t::CONNECTABLE_UNDIRECTED,
            ble::adv_interval_t(ble::millisecond_t(40))
        );

        error = gap.setAdvertisingParameters(advertising_handle, adv_params);

        if (error) {
            print_error(error, "");
            printf("_ble.gap().setAdvertisingParameters() failed\r\n");
            return;
        }

        data_builder.clear();
        data_builder.setFlags();
        data_builder.setName(name());

        /* Set payload for the set */
        error = gap.setAdvertisingPayload(
            advertising_handle, data_builder.getAdvertisingData()
        );

        if (error) {
            print_error(error, "Gap::setAdvertisingPayload() failed\r\n");
            return;
        }

        error = gap.startAdvertising(advertising_handle, ble::adv_duration_t(ble::millisecond_t(4000)));

        if (error) {
            print_error(error, "Gap::startAdvertising() failed\r\n");
            return;
        }

        // printf("Advertising as \"%s\"\r\n", name());
    }

    void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *event)
    {
        queue.call(mbed::callback(&event->ble, &BLE::processEvents));
    }

    void on_write(const GattWriteCallbackParams *response)
    {
        // printf(" signal delivered! \r\n");
        // if (response->handle == gesture_characteristic.getValueHandle()) {
        //     this->queue.call_in(5000ms, []{ gesture_characteristic.read(); });
        // }
    }

    void on_read(const GattReadCallbackParams *response)
    {
        // std::cout << "This runs: on read!" << std::endl;

        // fetch the controller corresponding to this connection
        auto &address = this->conn_to_mac.find(response->connHandle)->second;
        auto &player_id = this->mac_to_id.find(address)->second;

        // compute the action read from the controller
        uint8_t received_value = response->data[response->offset];
        Action action = parse_action(received_value);
        uint8_t magnitude = response->data[response->offset + 1];

        // printf("Data length: %d\r\n", response->len);
        // for (int i = 0; i < response->len; i++) {
        //     printf("%d ", response->data[i]);
        // }
        // printf("\r\n");
        // printf("Action is number: %d\r\n", received_value);
        // printf("Magnitude is: %d\r\n", magnitude);
        // printf("Queueing action...\r\n");

        this->controller_set.queue_to_controller(player_id, action, magnitude);
    }

    void start_gatt() {
        ble::WriteCallback_t write_callback;
        ble::ReadCallback_t read_callback;

        write_callback.attach(this, &ControllerConnectionHandler::on_write);
        read_callback.attach(this, &ControllerConnectionHandler::on_read);

        this->gatt.onDataRead(read_callback);
        this->gatt.onDataWritten(write_callback);
    }

    void start_discovery(const ble::ConnectionCompleteEvent &event)
    {
        ServiceDiscovery::TerminationCallback_t termination_callback;
        ServiceDiscovery::ServiceCallback_t service_callback;
        ServiceDiscovery::CharacteristicCallback_t characteristic_callback;

        termination_callback.attach(this, &ControllerConnectionHandler::discovery_termination);
        service_callback.attach(this, &ControllerConnectionHandler::service_discovery);
        characteristic_callback.attach(this, &ControllerConnectionHandler::characteristic_discovery);

        this->gatt.onServiceDiscoveryTermination(termination_callback);
        this->gatt.launchServiceDiscovery(
            event.getConnectionHandle(),
            service_callback,
            characteristic_callback,
            ControllerServiceUUID//,
            // GestureCharacteristicUUID
        );
    }

    void discovery_termination(ble::connection_handle_t connectionHandle)
    {
        if (gesture_characteristic_found) {
            gesture_characteristic_found = false;
            // queue.call([]{ gesture_characteristic.read(); });
        }
    }

    void characteristic_discovery(const DiscoveredCharacteristic *characteristic)
    {
        // printf("%x\r\n", characteristic->getUUID().getShortUUID());
        if (characteristic->getUUID().getShortUUID() == GestureCharacteristicUUID) {
            // printf("Gesture characteristic detected!\r\n");
            // printf("Validating controller!\r\n");

            auto handle = characteristic->getConnectionHandle();
            auto it1 = this->conn_to_mac.find(handle);
            auto addr = it1->second;
            auto it2 = this->mac_to_id.find(addr);
            auto controller_id = it2->second;

            this->controller_set.validate_controller(controller_id);

            // printf("Now controller %d can be used for game\r\n", controller_id);
            
            gesture_characteristic = *characteristic;
            gesture_characteristic_found = true;

            characteristic->read();
            this->mac_to_read_char.emplace(addr, *characteristic);
            
        }
        else if (characteristic->getUUID().getShortUUID() == SignalCharacteristicUUID) {
            // printf("Signal characteristic detected!\r\n");

            auto handle = characteristic->getConnectionHandle();
            auto it1 = this->conn_to_mac.find(handle);
            auto addr = it1->second;
            auto it2 = this->mac_to_id.find(addr);
            auto controller_id = it2->second;

            // printf("Now controller %d can be used for game\r\n", controller_id);
            
            gesture_characteristic = *characteristic;
            gesture_characteristic_found = true;

            this->mac_to_write_char.emplace(addr, *characteristic);
        }
    }

    void service_discovery(const DiscoveredService *service)
    {
        if (service->getUUID().shortOrLong() == UUID::UUID_TYPE_SHORT) {
            if (service->getUUID().getShortUUID() == ControllerServiceUUID) {
                // printf("Controller Service Found!\r\n");
            }
        }
    }
};


