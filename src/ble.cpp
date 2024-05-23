#include <Arduino.h>
#include "ble.h"

static BLEUUID serviceUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
static BLEUUID charRdUUID("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
static BLEUUID charTxUUID("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");
static const char *BLE_DEVICE_NAME = "BittleAD";

void PetoiBLE::onResult(BLEAdvertisedDevice advertisedDevice)
{
    Serial.print(F("BLE Advertised Device found: "));
    Serial.println(advertisedDevice.toString().c_str());

    constexpr size_t len = sizeof(BLE_DEVICE_NAME);
    if (advertisedDevice.getName().compare(0, len, BLE_DEVICE_NAME) == 0)
    {
        BLEDevice::getScan()->stop();
        this->petoiBleDevice = new BLEAdvertisedDevice(advertisedDevice);
        Serial.print(F("Found BLE Service."));
    }
}

void PetoiBLE::onConnect(BLEClient *pclient)
{
}

void PetoiBLE::onDisconnect(BLEClient *pclient)
{
    this->petoiConnected = false;
    this->petoiCommandChannel = nullptr;
    Serial.println("petoi disconnect");
}

void PetoiBLE::init_scan()
{
    BLEDevice::init("PETOI_ATOM_ECHO");
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(this);
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false);
}

void PetoiBLE::init_ble_connection()
{
    if (!this->petoiBleDevice || this->petoiConnected)
    {
        return;
    }

    Serial.println(F("Connecting Petoi"));

    Serial.print("Forming a connection to petoi: ");
    Serial.println(petoiBleDevice->getAddress().toString().c_str());

    BLEClient *pClient = BLEDevice::createClient();
    pClient->setClientCallbacks(this);
    pClient->connect(petoiBleDevice);
    Serial.println(" - Connected to server");

    BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr)
    {
        Serial.print("Failed to find service UUID: ");
        Serial.println(serviceUUID.toString().c_str());
        pClient->disconnect();
        return;
    }
    Serial.println(" - Found service");

    this->petoiCommandChannel = pRemoteService->getCharacteristic(charRdUUID);
    if (this->petoiCommandChannel == nullptr)
    {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(charRdUUID.toString().c_str());
        pClient->disconnect();
        return;
    }
    Serial.println(" - Found characteristic");

    petoiConnected = true;
}