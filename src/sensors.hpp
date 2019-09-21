#ifndef _SENSORS_H_
#define _SENSORS_H_

#include <OneWire.h>
#include <DallasTemperature.h>

class HumiditySensor {
    public:
        HumiditySensor();
        HumiditySensor(uint8_t pin);
        float readSensorPercent();
        int getSensorsPin();
        ~HumiditySensor();
    private:
        uint8_t _sensors_pin = 0;
};

#endif