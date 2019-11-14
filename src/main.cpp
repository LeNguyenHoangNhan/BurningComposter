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

#include <ArduinoJson.hpp>

#include "error_lcd.hpp"
#include "lcd.hpp"
#include "sensors.hpp"
#include "task.hpp"
#include "config.hpp"


#define ts1_PIN (int)15 // temp sensor 1 pin
#define HS_PIN1 (int)35 // humidity sensor 1 pin

#define ts2_PIN (int)16 // temp sensor 2 pin
#define HS_PIN2 (int)34 // humidity sensor 2 pin

#define ts3_PIN (int)17 // temp sensor 3 pin
#define HS_PIN3 (int)33 // humidity sensor 3 pin

OneWire onewire1(ts1_PIN);
HumiditySensor hs1(HS_PIN1);
DallasTemperature ts1(&onewire1);

OneWire onewire2(ts2_PIN);
HumiditySensor hs2(HS_PIN2);
DallasTemperature ts2(&onewire2);

OneWire onewire3(ts3_PIN);
HumiditySensor hs3(HS_PIN3);
DallasTemperature ts3(&onewire3);



HTTPClient http; // HTTPClient to handle HTTP request (send data back to server)

#if USE_STATIC_MEMORY  // "C" strings instead of normal "String", it use less memory and minimize heap fragmentation?
char AP_SSID_char[64]; // Buffer for AP_SSID (The SSID of the device own AP)
char AP_PASS_char[64]; // Buffer for AP_PASS (The password of the device own AP)
char STA_SSID_char[64]; // Buffer for STA_SSID (The SSID of the WiFi Station the device will try to connect to)
char STA_PASS_char[256]; // Buffer for STA_PASS (The password of the WiFi Station the device will try to connect to)
char UUID_char[64]; // Buffer for device's UUID
#else // Use "String" string as normal
String AP_SSID, AP_PASS, STA_SSID, STA_PASS, UUID;
#endif

AsyncWebServer server(80); // Create an async web server to handle device's web interface
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_ROW, LCD_COL); // Create an lcd to display info locally

bool SPIFFS_OK{false}; // Is SPIFFS ok?
bool WiFi_CONNECTED{false}; // Is WiFi connected?
bool WiFi_GOTIP{false}; // Does we got IP Address



void init_lcd() {
    lcd.init();
    lcd.clear();
    lcd.backlight();
    lcd.setCursor(0, 0);
}

int ReadJsonFile(String &target, const char *path, const char *field) {
    File file = SPIFFS.open(path, "r");
    if (!file) {
        Serial.println("Error open file");
        file.close();
        target = String();
        return -127;
    }
    String file_content = file.readString();
    file.close();
    ArduinoJson::DynamicJsonDocument doc(1024);
    ArduinoJson::DeserializationError err =
        ArduinoJson::deserializeJson(doc, file_content);
    if (err) {
        Serial.println("Error parsing JSON file");
        target = String();
        return -1;
    }
    target = doc[field].as<String>();
    return 0;
}
/** Implement JSON reader function using stack allocated memory
 *  !!CAUTION!! USE WITH CARE, NOT TESTED MUCH
 */
template <size_t size>
int ReadJsonFile(char (&target)[size], const char *path, const char *field) {
    Serial.printf("Reading field:%s from file: %s\n", field, path);
    File file = SPIFFS.open("/wfcfg.json", "r");
    ArduinoJson::StaticJsonDocument<1024> doc;
    ArduinoJson::DeserializationError err =
        ArduinoJson::deserializeJson(doc, file);
    if (err) {
        Serial.println("Error parsing JSON");
        file.close();
        return -127;
    }
    const char *field_value = doc[field];
    Serial.printf("Field value: %s\n", field_value);
    snprintf(target, size, "%s", field_value);
    Serial.printf("Target value: %s\n", target);
    file.close();
    return 0;
}
int WriteJsonFile(const char *target_field, const char *target_value,
                  const char *file_path) {
    File file_r = SPIFFS.open(file_path, "r");
    if (!file_r) {
        Serial.println("Error open file");
        file_r.close();
        return (-127);
    }
    String file_content = file_r.readString();
    Serial.printf("File content: %s\n", file_content.c_str());
    file_r.close();
    ArduinoJson::DynamicJsonDocument doc(1024);
    ArduinoJson::DeserializationError err =
        ArduinoJson::deserializeJson(doc, file_content);
    if (err) {
        Serial.println("Error parsing JSON file");
        return (-1);
    }
    doc[target_field] = target_value;
    String file_content_write = "";
    ArduinoJson::serializeJsonPretty(doc, file_content_write);
    Serial.println("Content to be write: " + file_content_write);
    SPIFFS.remove(file_path);
    File file_w = SPIFFS.open(file_path, "w");
    file_w.print(file_content_write);
    file_w.close();
    File file_cs = SPIFFS.open(file_path, "r");
    if (file_cs.readString() == file_content_write) {
        Serial.println("Check sum OK");
        file_cs.close();
        return 0;
    } else {
        Serial.println("Check sum not OK");
        file_cs.close();
        return (-256);
    }
}

void setup() {
    Serial.begin(9600);
    if (SPIFFS.begin()) {
        SPIFFS_OK = true;

        Serial.println("Reading Config File");
#if USE_STATIC_MEMORY
        if (ReadJsonFile(AP_SSID_char, "/wfcfg.json", "AP_SSID")) {
            lcd_err_clr_pr(lcd, LCD_ERR_FAILED_READ_CONFIG_AP_SSID);
            delay(1000);
            snprintf(AP_SSID_char, sizeof(AP_SSID_char), "WiFi_Config");
        }
        if (ReadJsonFile(AP_PASS_char, "/wfcfg.json", "AP_PASS")) {
            lcd_err_clr_pr(lcd, LCD_ERR_FAILED_READ_CONFIG_AP_PASS);
            delay(1000);
            snprintf(AP_PASS_char, sizeof(AP_PASS_char), "12345678");
        }
        if (ReadJsonFile(STA_SSID_char, "/wfcfg.json", "STA_SSID")) {
            lcd_err_clr_pr(lcd, LCD_ERR_FAILED_READ_CONFIG_STA_SSID);
            delay(1000);
            snprintf(STA_SSID_char, sizeof(STA_SSID_char), "hNiP");
        }
        if (ReadJsonFile(STA_PASS_char, "/wfcfg.json", "STA_PASS")) {
            lcd_err_clr_pr(lcd, LCD_ERR_FAILED_READ_CONFIG_STA_PASS);
            delay(1000);
            snprintf(STA_PASS_char, sizeof(STA_PASS_char), "LunarQueen12273");
        }
        if (ReadJsonFile(UUID_char, "/wfcfg.json", "UUID")) {
            lcd_err_clr_pr(lcd, LCD_ERR_FAILED_READ_CONFIG_STA_PASS);
            delay(1000);
            snprintf(UUID_char, sizeof(UUID_char), "abcdefgh");
        }
#else
        if (ReadJsonFile(AP_SSID, "/wfcfg.json", "AP_SSID")) {
            lcd_err_clr_pr(lcd, LCD_ERR_FAILED_READ_CONFIG_AP_SSID);
            delay(1000);
            AP_SSID = "WiFi_Config";
        }
        if (ReadJsonFile(AP_PASS, "/wfcfg.json", "AP_PASS")) {
            lcd_err_clr_pr(lcd, LCD_ERR_FAILED_READ_CONFIG_AP_PASS);
            delay(1000);
            AP_PASS = "12345678";
        }
        if (ReadJsonFile(STA_SSID, "/wfcfg.json", "STA_SSID")) {
            lcd_err_clr_pr(lcd, LCD_ERR_FAILED_READ_CONFIG_STA_SSID);
            delay(1000);
            STA_SSID = "hNiP";
        }
        if (ReadJsonFile(STA_PASS, "/wfcfg.json", "STA_PASS")) {
            lcd_err_clr_pr(lcd, LCD_ERR_FAILED_READ_CONFIG_STA_PASS);
            delay(1000);
            STA_PASS = "LunarQueen12273";
        }
        if (ReadJsonFile(UUID, "/wfcfg.json", "UUID")) {
            lcd_err_clr_pr(lcd, LCD_ERR_FAILED_READ_CONFIG_STA_PASS);
            delay(1000);
            UUID = "abcdefgh";
        }
#endif
        Serial.println("Read config file done");
    }
#if USE_STATIC_MEMORY
    Serial.printf("AP_SSID: %s\n", AP_SSID_char);
    Serial.printf("AP_PASS: %s\n", AP_PASS_char);
    Serial.printf("STA_SSID: %s\n", STA_SSID_char);
    Serial.printf("STA_PASS: %s\n", STA_PASS_char);
    WiFi.setAutoReconnect(true);
    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.begin(STA_PASS_char, STA_PASS_char);
    WiFi.softAP(AP_SSID_char, AP_PASS_char);
    WiFi.softAPConfig(IPAddress(172, 16, 0, 1), IPAddress(172, 16, 0, 1),
                      IPAddress(255, 255, 255, 0));
#else
    Serial.println("AP_SSID: " + (USE_STATIC_MEMORY ? AP_SSID_char : AP_SSID));
    Serial.println("AP_PASS: " + (USE_STATIC_MEMORY ? AP_PASS_char : AP_PASS));
    Serial.println("STA_SSID: " +
                   (USE_STATIC_MEMORY ? STA_SSID_char : STA_SSID));
    Serial.println("STA_PASS: " +
                   (USE_STATIC_MEMORY ? STA_PASS_char : STA_PASS));
    WiFi.setAutoReconnect(true);
    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.begin(USE_STATIC_MEMORY ? STA_PASS_char : STA_SSID.c_str(),
               USE_STATIC_MEMORY ? STA_PASS_char : STA_PASS.c_str());
    WiFi.softAP(USE_STATIC_MEMORY? AP_SSID_char : AP_SSID.c_str(), USE_STATIC_MEMORY? AP_PASS_char, AP_PASS.c_str());
    WiFi.softAPConfig(IPAddress(172, 16, 0, 1), IPAddress(172, 16, 0, 1),
                      IPAddress(255, 255, 255, 0));
#endif
    if (SPIFFS_OK) {
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/index.html", "text/html", false, nullptr);
        });
        server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/index.html", "text/html", false, nullptr);
        });
        server.on("/generic.css", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/generic.css", "text/css", false, nullptr);
        });
        server.on("/generic.js", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/generic.js", "application/javascript",
                          false, nullptr);
        });
        server.on("/monitor", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/monitor.html", "text/html", false, nullptr);
        });
        server.on("/monitor.html", HTTP_GET,
                  [](AsyncWebServerRequest *request) {
                      request->send(SPIFFS, "/monitor.html", "text/html", false,
                                    nullptr);
                  });
        server.on("/about", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/about.html", "text/html", false, nullptr);
        });
        server.on("/about.html", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/about.html", "text/html", false, nullptr);
        });
        server.on("/wfcf.js", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/wfcf.js", "application/javascript", false,
                          nullptr);
        });
#if USE_STATIC_MEMORY
        server.on("/wificonfig.html", HTTP_GET,
                  [](AsyncWebServerRequest *request) {
                      request->send(SPIFFS, "/wificonfig.html", "text/html",
                                    false, [](const String &pp_temp) -> String {
                                        if (pp_temp == "SSID") {
                                            return String(STA_SSID_char);
                                        } else if (pp_temp == "PASS") {
                                            return String(STA_PASS_char);
                                        } else {
                                            return String();
                                        }
                                    });
                  });
#endif
        server.on("/wificonfig", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/wificonfig.html", "text/html", false,
                          nullptr);
        });
        AsyncCallbackJsonWebHandler *wfcfJsonHanler =
            new AsyncCallbackJsonWebHandler(
                "/wificonfig", [](AsyncWebServerRequest *request,
                                  ArduinoJson::JsonVariant &jsonVar) {
                    ArduinoJson::JsonObject jsonObj =
                        jsonVar.as<ArduinoJson::JsonObject>();
                    String ssid = jsonObj["ssid"].as<String>();
                    String pass = jsonObj["pass"].as<String>();
                    Serial.printf("Recieved SSID: %s\n", ssid.c_str());
                    Serial.printf("Recieved PASS: %s\n", pass.c_str());
                    WriteJsonFile("STA_SSID", ssid.c_str(), "/wfcfg.json");
                    WriteJsonFile("STA_PASS", pass.c_str(), "/wfcfg.json");
                });
        server.addHandler(wfcfJsonHanler);
        server.begin();
    }
    ts1.begin();
    Serial.printf("Sensor num: %d\n", ts1.getDeviceCount());
    init_lcd();
    init_task();
}
void loop() {
#define USE_LOOP
#ifdef USE_LOOP
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("Humd: %.2f", humd);
    lcd.setCursor(0, 1);
    lcd.printf("Temp: %.2f", temp);
    vTaskDelay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("IP ADDRESS");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    vTaskDelay(2000);

#endif
}