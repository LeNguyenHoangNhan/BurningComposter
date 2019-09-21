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

const char *const SSID = "Tomato24";
const char *const PASSWD = "TalkischeapShowmethecode";
const char *const fUUID = "a4015fb6bb614a59be61cb8a23a249c5";
const char *const getUUIDRootCA = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIF3jCCA8agAwIBAgIQAf1tMPyjylGoG7xkDjUDLTANBgkqhkiG9w0BAQwFADCB\n" \
"iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl\n" \
"cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV\n" \
"BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTAw\n" \
"MjAxMDAwMDAwWhcNMzgwMTE4MjM1OTU5WjCBiDELMAkGA1UEBhMCVVMxEzARBgNV\n" \
"BAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0plcnNleSBDaXR5MR4wHAYDVQQKExVU\n" \
"aGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNVBAMTJVVTRVJUcnVzdCBSU0EgQ2Vy\n" \
"dGlmaWNhdGlvbiBBdXRob3JpdHkwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIK\n" \
"AoICAQCAEmUXNg7D2wiz0KxXDXbtzSfTTK1Qg2HiqiBNCS1kCdzOiZ/MPans9s/B\n" \
"3PHTsdZ7NygRK0faOca8Ohm0X6a9fZ2jY0K2dvKpOyuR+OJv0OwWIJAJPuLodMkY\n" \
"tJHUYmTbf6MG8YgYapAiPLz+E/CHFHv25B+O1ORRxhFnRghRy4YUVD+8M/5+bJz/\n" \
"Fp0YvVGONaanZshyZ9shZrHUm3gDwFA66Mzw3LyeTP6vBZY1H1dat//O+T23LLb2\n" \
"VN3I5xI6Ta5MirdcmrS3ID3KfyI0rn47aGYBROcBTkZTmzNg95S+UzeQc0PzMsNT\n" \
"79uq/nROacdrjGCT3sTHDN/hMq7MkztReJVni+49Vv4M0GkPGw/zJSZrM233bkf6\n" \
"c0Plfg6lZrEpfDKEY1WJxA3Bk1QwGROs0303p+tdOmw1XNtB1xLaqUkL39iAigmT\n" \
"Yo61Zs8liM2EuLE/pDkP2QKe6xJMlXzzawWpXhaDzLhn4ugTncxbgtNMs+1b/97l\n" \
"c6wjOy0AvzVVdAlJ2ElYGn+SNuZRkg7zJn0cTRe8yexDJtC/QV9AqURE9JnnV4ee\n" \
"UB9XVKg+/XRjL7FQZQnmWEIuQxpMtPAlR1n6BB6T1CZGSlCBst6+eLf8ZxXhyVeE\n" \
"Hg9j1uliutZfVS7qXMYoCAQlObgOK6nyTJccBz8NUvXt7y+CDwIDAQABo0IwQDAd\n" \
"BgNVHQ4EFgQUU3m/WqorSs9UgOHYm8Cd8rIDZsswDgYDVR0PAQH/BAQDAgEGMA8G\n" \
"A1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEMBQADggIBAFzUfA3P9wF9QZllDHPF\n" \
"Up/L+M+ZBn8b2kMVn54CVVeWFPFSPCeHlCjtHzoBN6J2/FNQwISbxmtOuowhT6KO\n" \
"VWKR82kV2LyI48SqC/3vqOlLVSoGIG1VeCkZ7l8wXEskEVX/JJpuXior7gtNn3/3\n" \
"ATiUFJVDBwn7YKnuHKsSjKCaXqeYalltiz8I+8jRRa8YFWSQEg9zKC7F4iRO/Fjs\n" \
"8PRF/iKz6y+O0tlFYQXBl2+odnKPi4w2r78NBc5xjeambx9spnFixdjQg3IM8WcR\n" \
"iQycE0xyNN+81XHfqnHd4blsjDwSXWXavVcStkNr/+XeTWYRUc+ZruwXtuhxkYze\n" \
"Sf7dNXGiFSeUHM9h4ya7b6NnJSFd5t0dCy5oGzuCr+yDZ4XUmFF0sbmZgIn/f3gZ\n" \
"XHlKYC6SQK5MNyosycdiyA5d9zZbyuAlJQG03RoHnHcAP9Dc1ew91Pq7P8yF1m9/\n" \
"qS3fuQL39ZeatTXaw2ewh0qpKJ4jjv9cJ2vhsE/zB+4ALtRZh8tSQZXq9EfX7mRB\n" \
"VXyNWQKV3WKdwrnuWih0hKWbt5DHDAff9Yk2dDLWKMGwsAvgnEzDHNb842m1R0aB\n" \
"L6KCq9NjRHDEjf8tM7qtj3u1cIiuPhnPQCjY/MiQu12ZIvVS5ljFH4gxQ+6IHdfG\n" \
"jjxDah2nGN59PRbxYvnKkKj9\n" \
"-----END CERTIFICATE-----\n";

const char *const HOSTNAME = "ESP_32_a249c5";
const char *URL = "uphan.makerspace.nqdclub.com";
const int SERVER_PORT = 80;
const char *PATH = "/postjson";

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
    data["temp"] = temp;
    data["humidity"] = humd;
    data["uuid"] = UUID;
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
    delay(1000);
}
