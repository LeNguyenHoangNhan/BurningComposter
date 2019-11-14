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