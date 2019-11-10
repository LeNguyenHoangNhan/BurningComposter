#pragma once
#ifndef TASK_HPP__
#define TASK_HPP__
#include <freertos/FreeRTOS.h>
#include <Arduino.h>
#include <freertos/task.h>
#include <freertos/projdefs.h>
#include "sensors.hpp"
#include <DallasTemperature.h>
#include <OneWire.h>
#include "lcd.hpp"
#include <WiFi.h>
#include <ArduinoJson.hpp>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <HTTPClient.h>

static TaskHandle_t RDSSTSK_handler;
static TaskHandle_t DISPLAY_handler;
static TaskHandle_t SENDDATA_hanlder;



int init_task();

#endif