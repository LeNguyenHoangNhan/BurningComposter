#include "sensors.hpp"
#include <Arduino.h>
#include <ArduinoJson.hpp>
#include <AsyncJson.h>
#include <AsyncTCP.h>
#include <DallasTemperature.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <SPIFFS.h>
#include <WiFi.h>

#define TS_PIN (int)15
#define HS_PIN (int)35
#define LCD_ADDR (int)0x27
#define LCD_ROW (int)2
#define LCD_COL (int)16

String AP_SSID, AP_PASS, STA_SSID, STA_PASS, UUID;
AsyncWebServer server(80);
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_ROW, LCD_COL);

 

int ReadJsonFile(String &target, const char *path, const char *field) {
    File file = SPIFFS.open(path, "r");
    if (file) {
        Serial.println("Error open file");
        target = String();
        return -127;
    }
    String file_content = file.readString();
    file.close();
    ArduinoJson::DynamicJsonDocument doc(1024);
    ArduinoJson::DeserializationError err = ArduinoJson::deserializeJson(doc, file_content);
    if (err) {
        Serial.println("Error parsing JSON file");
        target = String();
        return -1;
    }
    target = doc[field].as<String>();
    return 0;
}
int WriteJsonFile(const char *target_field, const char *target_value, const char *file_path) {
    File file_r = SPIFFS.open(file_path, "r");
    if (file_r) {
        Serial.println("Error open file");
        return (-127);
    }
    String file_content = file_r.readString();
    Serial.printf("File content: %s\n", file_content.c_str());
    file_r.close();
    ArduinoJson::DynamicJsonDocument doc(1024);
    ArduinoJson::DeserializationError err = ArduinoJson::deserializeJson(doc, file_content);
    if (err) {
        Serial.println("Error parsing JSON file");
        return (-1);
    }
    doc[target_field] = target_value;
    ArduinoJson::serializeJsonPretty(doc, file_content);
    Serial.println("Content to be write: " + file_content);
    SPIFFS.remove(file_path);
    File file_w = SPIFFS.open(file_path, "w");
    file_w.print(file_content);
    file_w.close();
    File file_cs = SPIFFS.open(file_path, "r");
    if (file_cs.readString() == file_content) {
        Serial.println("Check sum OK");
        file_cs.close();
        return 0;
    } else {
        Serial.println("Check sum not OK");
        file_cs.close();
        return (-256);
    }
}
void lcd_err_clr_pr(LiquidCrystal_I2C &lcd, const char *content) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error:");
    lcd.setCursor(0, 1);
    lcd.print(content);
}
void lcd_err_clr_pr(LiquidCrystal_I2C &lcd, String content) {
    lcd_err_clr_pr(lcd, content.c_str());
}
void lcd_err_clr_pr(LiquidCrystal_I2C &lcd, int err_code) {
    lcd_err_clr_pr(lcd, String(err_code).c_str());
}
void lcd_clr_pr() {
}
void setup() {
    Serial.begin(9600);
    if (SPIFFS.begin()) {
        Serial.println("Reading Config File");
        if (ReadJsonFile(AP_SSID, "/wfcfg.json", "AP_SSID")) {
            lcd_err_clr_pr(lcd, "001");
            delay(1000);
            AP_SSID = "WiFi_Config";
        }
        if (ReadJsonFile(AP_PASS, "/wfcfg.json", "AP_PASS")) {
            lcd_err_clr_pr(lcd, "002");
            delay(1000);
            AP_PASS = "12345678";
        }
        if (ReadJsonFile(STA_PASS, "/wfcfg.json", "STA_SSID")) {
            lcd_err_clr_pr(lcd, "003");
            delay(1000);
            STA_SSID = "hNiP";
        }
        if (ReadJsonFile(STA_SSID, "/wfcfg.json", "STA_PASS")) {
            lcd_err_clr_pr(lcd, "004");
            delay(1000);
            STA_PASS = "LunarQueen12273";
        }
        Serial.println("Read config file done");
    } else {

    }
        Serial.println("AP_SSID: " + AP_SSID);
        Serial.println("AP_PASS: " + AP_PASS);
        Serial.println("STA_SSID: " + STA_SSID);
        Serial.println("STA_PASS: " + STA_PASS);
        WiFi.setAutoReconnect(true);
        WiFi.mode(WIFI_MODE_APSTA);
        WiFi.begin(STA_SSID.c_str(), STA_PASS.c_str());
        WiFi.softAP(AP_SSID.c_str(), AP_PASS.c_str());
        WiFi.softAPConfig(IPAddress(172, 16, 0, 1), IPAddress(172, 16, 0, 1), IPAddress(255, 255, 255, 0));
    }
    void loop() {
    }