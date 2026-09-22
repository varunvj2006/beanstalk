#include <Arduino.h>

void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println("========================");
    Serial.println("BEANSTALK");
    Serial.println("Firmware v0.1");
    Serial.println("========================");
    Serial.println("System initialized");
}

void loop()
{
}