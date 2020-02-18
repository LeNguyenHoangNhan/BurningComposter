/*
    This file is part of Burning Composter - A compost monitoring device
    based on ESP32 and Arduino Core.

        Copyright (c) 2019 Le Nguyen Hoang Nhan

    Burning Composter  is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Burning Composter  is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Burning Composter.  If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once
#ifndef TASK_HPP__
#define TASK_HPP__
#include <Arduino.h>
#include <AsyncTCP.h>
#include <DallasTemperature.h>
#include <HTTPClient.h>
#include <OneWire.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/projdefs.h>
#include <freertos/task.h>

#include <ArduinoJson.hpp>

#include "lcd.hpp"
#include "sensors.hpp"

extern TaskHandle_t RDSSTSK_handler;
extern TaskHandle_t DISPLAY_handler;
extern TaskHandle_t SENDDATA_hanlder;
extern float humd;
extern float temp;

int init_task();

#endif