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
#define LCD_ADDR 0x27
#define LCD_ROW 2
#define LCD_COL 16

char *SSID = "Nguyen";
char *PASSWD = "1234567891";
char *fUUID = "abcdefgh";

const char *const HOSTNAME = "ESP_32_a249c5";
const char *URL = "uphan.makerspace.nqdclub.com";
const int SERVER_PORT = 80;
const char *PATH = "/giatricambien";

float temp{0.0}, humd{0.0};
bool wifi_connected{false};
bool gotUUID = true; // Test get UUID function when possible, change this to false

HTTPClient http;
OneWire onewire(TS_PIN);
DallasTemperature ts(&onewire);
HumiditySensor hs(HS_PIN);
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COL, LCD_ROW);
String UUID = fUUID; // Change this to "" when testing get UUID function
void SendData() {
    StaticJsonDocument<500> data;
    data["uuid"] = fUUID;
    data["temp"] = temp;
    data["humidity"] = humd;
    
    http.begin("nqdbeta.tk", 80, "/giatricambien");
    http.addHeader("Content-Type", "application/json");
    String posthttp;
    serializeJson(data, posthttp);
    Serial.println(posthttp);
    int httpCode = http.POST(posthttp);
    Serial.print("httpCode: ");
    Serial.println(httpCode);
    http.end();
}

void WiFiDisconnectedLoop() {
    ts.requestTemperatures();
    delay(500);
    float prevtemp = temp;
    if ((temp = ts.getTempCByIndex(0)) == -127) {
        temp = prevtemp;
    } 
    humd = hs.readSensorPercent();
    Serial.print("Temperature: ");
    Serial.println(temp);
    Serial.print("Moisture: ");
    Serial.println(humd);
    lcd.setCursor(6, 0);
    lcd.print(temp);
    lcd.setCursor(6, 1);
    lcd.print(humd);
}

void WiFiConnectedLoop() {
    WiFiDisconnectedLoop();
    SendData();
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

void getUUID() {
    http.begin("https://www.uuidgenerator.net/api/version4", getUUIDRootCA);
    int http_Code = http.GET();
    if (http_Code > 0) {
        String payload = http.getString();
        Serial.printf("Got UUID: %s", payload); 
        UUID = payload;
    } else {
        UUID = fUUID;
    }
    http.end();
    gotUUID = true;
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
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.setCursor(0, 1);
    lcd.print("Humd: ");
    WiFi.disconnect();
    WiFi.mode(WIFI_MODE_STA);
    WiFi.onEvent(WiFiEvent);
    WiFi.begin(SSID, PASSWD);
}
void loop() {
    if (WiFi.status() == WL_CONNECTED && wifi_connected == true) {
        if (gotUUID) {
            WiFiConnectedLoop();
        } else {
            getUUID();
        }
    } else {
        WiFiDisconnectedLoop();
    }
    delay(60000);
}
