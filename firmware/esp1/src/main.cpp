#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define DEVICE_NAME "BEANSTALK_DEVICE"

#define SERVICE_UUID    "7a1e0001-8b5a-4c7a-9f11-123456789abc"
#define ALERT_CHAR_UUID "7a1e0002-8b5a-4c7a-9f11-123456789abc"
#define COMMAND_CHAR_UUID "7a1e0004-8b5a-4c7a-9f11-123456789abc"
#define STATUS_CHAR_UUID "7a1e0003-8b5a-4c7a-9f11-123456789abc"

#define ALERT_BUTTON_PIN 0

BLEServer* server = nullptr;
BLECharacteristic* alertCharacteristic = nullptr;
BLECharacteristic* commandCharacteristic = nullptr;
BLECharacteristic* statusCharacteristic = nullptr;

bool monitoringEnabled = false;

bool deviceConnected = false;
bool lastButtonState = HIGH;

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

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

    if (!monitoringEnabled)
    {
        Serial.println("Alert blocked: monitoring disabled");
        return;
    }

    alertCharacteristic->setValue("TEST_ALERT");
    alertCharacteristic->notify();

    Serial.println("Sent: TEST_ALERT");
}
void publishStatus(const char* status)
{
    statusCharacteristic->setValue(status);

    if (deviceConnected)
    {
        statusCharacteristic->notify();
    }

    Serial.print("Status: ");
    Serial.println(status);
}
class CommandCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic* characteristic) override
    {
        String command = characteristic->getValue().c_str();
        command.trim();

        Serial.print("Command received: ");
        Serial.println(command);

        if (command == "START_MONITORING")
        {
            monitoringEnabled = true;
            publishStatus("MONITORING");
        }
        else if (command == "STOP_MONITORING")
        {
            monitoringEnabled = false;
            publishStatus("IDLE");
        }
        else
        {
            Serial.println("Unknown command");
        }
    }
};
void setup()
{
    Serial.begin(115200);
    delay(1000);

    pinMode(ALERT_BUTTON_PIN, INPUT_PULLUP);

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
    
    commandCharacteristic = service->createCharacteristic(
    COMMAND_CHAR_UUID,
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_WRITE_NR
);

    commandCharacteristic->setCallbacks(new CommandCallbacks());
    statusCharacteristic = service->createCharacteristic(
        STATUS_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_NOTIFY
    );

    statusCharacteristic->addDescriptor(new BLE2902());
    statusCharacteristic->setValue("IDLE");

    service->start();

    BLEAdvertising* advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->setScanResponse(true);

    BLEDevice::startAdvertising();

    Serial.println("System initialized");
    Serial.println("BLE initialized");
    Serial.println("Advertising as: BEANSTALK_DEVICE");
    Serial.println("Waiting for phone...");
    Serial.println("Press BOOT to send TEST_ALERT");
}

void loop()
{
    static bool previousButtonState = HIGH;

    bool currentButtonState = digitalRead(ALERT_BUTTON_PIN);

    if (previousButtonState == HIGH &&
        currentButtonState == LOW)
    {
        Serial.println("BOOT button pressed");

        sendTestAlert();

        delay(250);
    }

    previousButtonState = currentButtonState;

    delay(10);
}