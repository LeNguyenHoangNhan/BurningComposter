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

#include <Arduino.h>
#include <AsyncJson.h>
#include <DallasTemperature.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <FS.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <SPIFFS.h>
#include <StreamUtils.h>
#include <WiFi.h>
#include <mdns.h>

#include <ArduinoJson.hpp>

#include "config.hpp"
#include "error_lcd.hpp"
#include "lcd.hpp"
#include "sensors.hpp"
#include "task.hpp"

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

char AP_SSID_char[64]; // Buffer for AP_SSID (The SSID of the device own AP)
char AP_PASS_char[64]; // Buffer for AP_PASS (The password of the device own AP)
char STA_SSID_char[64];  // Buffer for STA_SSID (The SSID of the WiFi Station
                         // the device will try to connect to)
char STA_PASS_char[256]; // Buffer for STA_PASS (The password of the WiFi
                         // Station the device will try to connect to)
char UUID_char[64];      // Buffer for device's UUID
char mDNS_char[64];

AsyncWebServer
    server(80); // Create an async web server to handle device's web interface
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_ROW,
                      LCD_COL); // Create an lcd to display info locally

bool SPIFFS_OK{false};      // Is SPIFFS ok?
bool WiFi_CONNECTED{false}; // Is WiFi connected?
bool WiFi_GOTIP{false};     // Does we got IP Address?

void init_lcd() {
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
}

template <size_t size>
int ReadJsonFile(char (&target)[size], const char *path, const char *field) {
  Serial.printf("Reading field:%s from file: %s\n", field,
                path); // Debug purpose
  File file = SPIFFS.open(path, "r");
  ReadBufferingStream bufferedFile_r{file, 64}; // Use buffered File
  // 1024 Bytes Json doc
  ArduinoJson::StaticJsonDocument<1024> doc;
  ArduinoJson::DeserializationError err =
      ArduinoJson::deserializeJson(doc, bufferedFile_r);
  if (err) {
    Serial.println("Error parsing JSON");
    file.close();
    return -127; // No point of continuing
  }
  const char *field_value = doc[field];
  Serial.printf("Field value: %s\n", field_value); // Debug purpose
  snprintf(target, size, "%s",
           field_value); // Copy field's value to target buffer, up to target
                         // 'size' bytes, null terminate the target
  Serial.printf("Target value: %s\n", target); // Debug purpose
  bufferedFile_r.flush();
  file.close();
  return 0;
}

// Big FUNC here!!
int WriteJsonFile(const char *target_field, const char *target_value,
                  const char *file_path) {
  Serial.printf("Change field %s of file %s to %s\n", target_field, file_path,
                target_value); // Debug purpose

  File file_r = SPIFFS.open(file_path, "r");
  if (!file_r) {
    Serial.println("Error open file");
    file_r.close();
    return -127;
  }
  ReadBufferingStream bufferedFile_r{file_r, 64}; // Use buffered file
  ArduinoJson::StaticJsonDocument<1024> doc;
  ArduinoJson::DeserializationError deserial_err =
      ArduinoJson::deserializeJson(doc, bufferedFile_r);
  bufferedFile_r.flush();
  file_r.close();

  if (deserial_err) {
    Serial.println("Error parsing JSON file");
    return (-1);
  }
  doc[target_field] = target_value;

  SPIFFS.remove(file_path); // Delete file to write new file

  File file_w =
      SPIFFS.open(file_path, "w"); // Open file in 'write' mode for writing

  if (!file_w) {
    Serial.println("Error open file");
    file_r.close();
    return -127;
  }
  WriteBufferingStream bufferedFile_w{file_w, 64}; // Use buffered file
  Serial.println("Write content:");
  ArduinoJson::serializeJsonPretty(doc, Serial);
  ArduinoJson::serializeJson(doc, bufferedFile_w);
  bufferedFile_w.flush();
  file_w.close();

  File file_rb = SPIFFS.open(file_path, "r");
  ReadBufferingStream bufferedFile_rb{file_rb, 64};
  ArduinoJson::StaticJsonDocument<1024> doc_rb;
  deserial_err = ArduinoJson::deserializeJson(doc_rb, bufferedFile_rb);
  bufferedFile_rb.flush();
  file_rb.close();
  if (doc_rb == doc) {
    Serial.println("Read back OK");
    return 0;
  } else {
    Serial.println("Read back not match");
    return (-256);
  }
  return 0;
}

void setup() {
  Serial.begin(9600);
  init_lcd();
  if (SPIFFS.begin()) {
    SPIFFS_OK = true;
  } else {
    Serial.println("Error init SPIFFS!");
    lcd_err_clr_pr(lcd, LCD_ERR_FAILED_INIT_SPIFFS);
    delay(1000);
  }
  Serial.println("Reading Config File");

  // 0 = false, != 0 = true;
  if (!SPIFFS_OK) {
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
    if (ReadJsonFile(mDNS_char, "/wfcfg.json", "MDNS")) {
      lcd_err_clr_pr(lcd, LCD_ERR_FAILED_READ_CONFIG_MDNS);
      delay(1000);
      snprintf(mDNS_char, sizeof(mDNS_char), "esp32");
    }
  } else {
    snprintf(AP_SSID_char, sizeof(AP_SSID_char), "WiFi_Config");
    snprintf(AP_PASS_char, sizeof(AP_PASS_char), "12345678");
    snprintf(STA_SSID_char, sizeof(STA_SSID_char), "hNiP");
    snprintf(STA_PASS_char, sizeof(STA_PASS_char), "LunarQueen12273");
    snprintf(UUID_char, sizeof(UUID_char), "abcdefgh");
    snprintf(mDNS_char, sizeof(mDNS_char), "esp32");
  }

  Serial.println("Read config file done");

  Serial.printf("AP_SSID: %s\n", AP_SSID_char);
  Serial.printf("AP_PASS: %s\n", AP_PASS_char);
  Serial.printf("STA_SSID: %s\n", STA_SSID_char);
  Serial.printf("STA_PASS: %s\n", STA_PASS_char);
  WiFi.setAutoReconnect(true);
  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.begin(STA_SSID_char, STA_PASS_char);
  WiFi.softAPConfig(IPAddress(172, 16, 0, 1), IPAddress(172, 16, 0, 1),
                    IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_SSID_char, AP_PASS_char);
  if (!MDNS.begin(mDNS_char)) {
    Serial.println("Error setting up MDNS responder!");
  } else {
    Serial.println("mDNS responder started");
  }

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
    request->send(SPIFFS, "/generic.js", "application/javascript", false,
                  nullptr);
  });
  // server.on("/monitor", HTTP_GET, [](AsyncWebServerRequest *request) {
  //     request->send(SPIFFS, "/monitor.html", "text/html", false,
  //     nullptr);
  // });
  server.on("/monitor.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/monitor.html", "text/html", false, nullptr);
  });
  server.on("/about.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/about.html", "text/html", false,
                  [](const String &pp_templ) -> String {
                    Serial.printf("About page preprocessor: %s\n",
                                  pp_templ.c_str());
                    if (pp_templ == "FWVS") {
                      return String(FIRMWARE_VER);
                    } else if (pp_templ == "WFCD") {
                      return String(COMP_DATE);
                    } else if (pp_templ == "WMC") {
                      return WiFi.macAddress();
                    } else if (pp_templ == "FHP") {
                      return String(ESP.getFreeHeap());
                    } else if (pp_templ == "UUID") {
                      return String(UUID_char);
                    }
                    return String();
                  });
  });
  server.on("/wfcf.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/wfcf.js", "application/javascript", false, nullptr);
  });

  server.on("/wificonfig.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/wificonfig.html", "text/html", false,
                  [](const String &pp_temp) -> String {
                    if (pp_temp == "SSID") {
                      return String(STA_SSID_char);
                    } else if (pp_temp == "PASS") {
                      return String(STA_PASS_char);
                    } else {
                      return String();
                    }
                  });
  });

  AsyncCallbackJsonWebHandler *wfcfJsonHanler = new AsyncCallbackJsonWebHandler(
      "/wificonfig",
      [](AsyncWebServerRequest *request, ArduinoJson::JsonVariant &jsonVar) {
        ArduinoJson::JsonObject jsonObj = jsonVar.as<ArduinoJson::JsonObject>();
        const char *ssid = jsonObj["ssid"];
        const char *pass = jsonObj["pass"];
        Serial.printf("Recieved SSID: %s\n", ssid);
        Serial.printf("Recieved PASS: %s\n", pass);
        WriteJsonFile("STA_SSID", ssid, "/wfcfg.json");
        WriteJsonFile("STA_PASS", pass, "/wfcfg.json");
      });

  server.addHandler(wfcfJsonHanler);
  server.begin();
  MDNS.addService("http", "tcp", 80);

  ts1.begin();
  int ts_num = ts1.getDeviceCount();
  if (!ts_num) {
    lcd_err_clr_pr(lcd, LCD_ERR_TS_DEVICE_MISSING);
    delay(1000);
    for (;;) {
    }
  }
  int err_task_code = init_task();
  if (err_task_code == -1) {
    lcd_err_clr_pr(lcd, LCD_ERR_FAILED_TASK_CREATE_SENSOR);
    delay(1000);
    for (;;) {
    }
  } else if (err_task_code == -2) {
    lcd_err_clr_pr(lcd, LCD_ERR_FAILED_TASK_CREATE_SENDDATA);
    delay(1000);
  }
}
void loop() {
#if USE_LOOP
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
