#include "sensors.hpp"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <DallasTemperature.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <WiFi.h>

#define TS_PIN 15
#define HS_PIN 35
#define LCD_ADDR 0x2F
#define LCD_ROW 2
#define LCD_COL 16

const char *const SSID = "Tomato24";
const char *const PASSWD = "TalkischeapShowmethecode";
const char *const UUID = "a4015fb6-bb61-4a59-be61-cb8a23a249c5";
const char *const HOSTNAME = "ESP_32_a249c5";
const char *URL = "uphan.makerspace.nqdclub.com";
const int SERVER_PORT = 80;
const char *PATH = "/postjson";

float temp{0.0}, humd{0.0};
bool wifi_connected{false};

HTTPClient http;
OneWire onewire(TS_PIN);
DallasTemperature ts(&onewire);
HumiditySensor hs(HS_PIN);
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COL, LCD_ROW);

void SendData() {
    StaticJsonDocument<200> data;
    data["temp"] = temp;
    data["humidity"] = humd;
    http.begin("uphan.makerspace.nqdclub.com", 80, "/postjson");
    http.addHeader("Content-Type", "application/json");
    String posthttp;
    serializeJson(data, posthttp);
    Serial.println(posthttp);
    int httpCode = http.POST(posthttp);
    Serial.print("httpCode: ");
    Serial.println(httpCode);
    http.end();
}

void WiFiConnectedLoop() {
    WiFiDisconnectedLoop();
    SendData();
}

void WiFiDisconnectedLoop() {
    temp = ts.getTempCByIndex(0);
    humd = hs.readSensorPercent();
    Serial.print("Temperature: ");
    Serial.println(temp);
    Serial.print("Moisture: ");
    Serial.println(humd);
}

void WiFiEvent(WiFiEvent_t event) {
    switch (event) {
    case SYSTEM_EVENT_STA_START:
        WiFi.setHostname(HOSTNAME);
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        WiFi.enableIpV6();
        break;
    case SYSTEM_EVENT_GOT_IP6:
        Serial.print("Got IPv6: ");
        Serial.println(WiFi.localIPv6());
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        wifi_connected = true;
        Serial.print("Got IPv4: ");
        Serial.println(WiFi.localIP());
        digitalWrite(2, 1);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        wifi_connected = false;
        Serial.println("WiFi disconnected");
        digitalWrite(2, 0);
    default:
        break;
    }
}

void setup() {
    Serial.begin(9600);
    Serial.println("Starting up... please wait...");
    delay(100);

    pinMode(2, OUTPUT);

    ts.begin();

    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Hello, world!");
    delay(1000);
    lcd.clear();

    WiFi.disconnect();
    WiFi.mode(WIFI_MODE_STA);
    WiFi.onEvent(WiFiEvent);
    WiFi.begin(SSID, PASSWD);
}
void loop() {
    if (WiFi.status() == WL_CONNECTED && wifi_connected == true) {
        WiFiConnectedLoop();
    } else {
        WiFiDisconnectedLoop();
    }
}
