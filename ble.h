#ifndef _BLE_H
#define _BLE_H

#include <BLEDevice.h>

class PetoiBLE: BLEClientCallbacks, BLEAdvertisedDeviceCallbacks
{
public:
    PetoiBLE() : petoiBleDevice(nullptr), petoiConnected(false), petoiCommandChannel(nullptr)
    {
    }
    void init_scan();
    void init_ble_connection();
    BLERemoteCharacteristic *get_command_channel() {
        if (!this->petoiConnected) {
            return nullptr;
        }
        return this->petoiCommandChannel;
    }

private:
    BLEAdvertisedDevice *petoiBleDevice;
    bool petoiConnected;
    BLERemoteCharacteristic *petoiCommandChannel;

    void onConnect(BLEClient *);
    void onDisconnect(BLEClient *);
    void onResult(BLEAdvertisedDevice advertisedDevice);
};

#endif