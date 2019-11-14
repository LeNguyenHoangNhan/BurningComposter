#ifndef MAIN_HPP__
#define MAIN_HPP__

#include <Arduino.h>
#include <AsyncJson.h>
#include <DallasTemperature.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <wchar.h>


extern HumiditySensor hs1;
extern DallasTemperature ts1;

extern HumiditySensor hs2;
extern DallasTemperature ts3;

extern HumiditySensor hs3;
extern DallasTemperature ts3;

extern LiquidCrystal_I2C lcd;
extern OneWire onewire;


#endif