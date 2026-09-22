#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define DEVICE_NAME "BEANSTALK_DEVICE"

#define SERVICE_UUID    "7a1e0001-8b5a-4c7a-9f11-123456789abc"
#define ALERT_CHAR_UUID "7a1e0002-8b5a-4c7a-9f11-123456789abc"

BLEServer* server = nullptr;
BLECharacteristic* alertCharacteristic = nullptr;

bool deviceConnected = false;

class ServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer* pServer) override
    {
        deviceConnected = true;
        Serial.println("Phone connected");
    }

    void onDisconnect(BLEServer* pServer) override
    {
        deviceConnected = false;
        Serial.println("Phone disconnected");

        delay(100);
        BLEDevice::startAdvertising();

        Serial.println("Advertising restarted");
    }
};

void sendTestAlert()
{
    if (!deviceConnected)
    {
        Serial.println("Cannot send alert: no phone connected");
        return;
    }

    alertCharacteristic->setValue("TEST_ALERT");
    alertCharacteristic->notify();

    Serial.println("Sent: TEST_ALERT");
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("========================");
    Serial.println("BEANSTALK");
    Serial.println("Firmware v0.1");
    Serial.println("========================");

    BLEDevice::init(DEVICE_NAME);

    server = BLEDevice::createServer();
    server->setCallbacks(new ServerCallbacks());

    BLEService* service = server->createService(SERVICE_UUID);

    alertCharacteristic = service->createCharacteristic(
        ALERT_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_NOTIFY
    );

    alertCharacteristic->addDescriptor(new BLE2902());

    alertCharacteristic->setValue("READY");

    service->start();

    BLEAdvertising* advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->setScanResponse(true);

    BLEDevice::startAdvertising();

    Serial.println("System initialized");
    Serial.println("BLE initialized");
    Serial.println("Advertising as: BEANSTALK_DEVICE");
    Serial.println("Waiting for phone...");
    Serial.println();
    Serial.println("Type 't' to send TEST_ALERT");
}

void loop()
{
    if (Serial.available())
    {
        char command = Serial.read();

        if (command == 't' || command == 'T')
        {
            sendTestAlert();
        }
    }

    delay(10);
}