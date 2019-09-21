#include <sensors.hpp>

HumiditySensor::HumiditySensor() {
    this->_sensors_pin = 15;
//    pinMode(_sensors_pin, INPUT);
}
HumiditySensor::HumiditySensor(uint8_t pin_num) {
    this->_sensors_pin = pin_num;
//    pinMode(_sensors_pin, INPUT);
}
HumiditySensor::~HumiditySensor() {
    this->_sensors_pin = 0;
}
float HumiditySensor::readSensorPercent() {
    return 100 - ((analogRead(_sensors_pin) * 100.0) / 4096.0);
}
int HumiditySensor::getSensorsPin() {
    return _sensors_pin;
}
