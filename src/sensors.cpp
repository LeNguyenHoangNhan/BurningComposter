/*
    This file is part of Burning Composter - A compost monitoring device
    based on ESP32 and Arduino Core.

    Burning Composter  is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Burning Composter  is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <https://www.gnu.org/licenses/>.
*/
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
