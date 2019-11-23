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
#include "task.hpp"

#include "config.hpp"
#include "main.hpp"

#if USE_STATIC_MEMORY
extern char UUID_char[64];
#else
extern String UUID;
#endif

ArduinoJson::StaticJsonDocument<512> jsonDoc;
TaskHandle_t RDSSTSK_handler;
TaskHandle_t SENDDATA_hanlder;

extern HTTPClient http;

float humd{0.0};
float temp{0.0};

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
int init_task() {
    BaseType_t tsk1_code = xTaskCreatePinnedToCore(
        [](void *pvParameters) {
            for (;;) {
                ts1.requestTemperatures();
                humd = hs1.readSensorPercent();
                temp = ts1.getTempCByIndex(0);
                Serial.printf("Temp: %.2f, Humd: %.2f\n", temp, humd);
                vTaskDelay(2000 / portTICK_PERIOD_MS);
            }
            vTaskDelete(NULL);
        },
        "RDSSTSK", 4096, nullptr, 10, &RDSSTSK_handler, 1);
    if (tsk1_code != pdPASS) {
        return -1;
    }

#if USE_STATIC_MEMORY
    BaseType_t tsk2_code = xTaskCreate(
        [](void *pvParameters) {
            for (;;) {
                vTaskDelay(60000);
                if (WiFi.isConnected() == true &&
                    WiFi.status() == WL_CONNECTED) {
                    Serial.println("Start send data to server");
                    Serial.printf("UUID: %s\n", UUID_char);
                    jsonDoc["uuid"] = UUID_char;
                    jsonDoc["temp"] = temp;
                    jsonDoc["humidity"] = humd;
                    http.begin("nqdbeta.tk", 80, "/giatricambien");
                    http.addHeader("Content-Type", "application/json");
                    char buffer[512];
                    ArduinoJson::serializeJson(jsonDoc, buffer);
                    Serial.println(buffer);
                    int httpCode = http.POST(buffer);
                    Serial.print("httpCode: ");
                    Serial.println(httpCode);
                    http.end();
                }
            }
            vTaskDelete(NULL);
        },
        "SNDDTTSK", 4096, nullptr, 20, &SENDDATA_hanlder);
    if (tsk2_code != pdPASS) {
        return -2;
    }
#else
    BaseType_t tsk2_code = xTaskCreate(
        [](void *pvParameters) {
            for (;;) {
                vTaskDelay(60000);
                if (WiFi.isConnected() == true &&
                    WiFi.status() == WL_CONNECTED) {
                    Serial.println("Start send data to server");
                    jsonDoc["uuid"] = UUID;
                    jsonDoc["temp"] = temp;
                    jsonDoc["humidity"] = humd;
                    http.begin("nqdbeta.tk", 80, "/giatricambien");
                    http.addHeader("Content-Type", "application/json");
                    char buffer[512];
                    ArduinoJson::serializeJson(jsonDoc, buffer);
                    Serial.println(buffer);
                    int httpCode = http.POST(buffer);
                    Serial.print("httpCode: ");
                    Serial.println(httpCode);
                    http.end();
                }
            }
            vTaskDelete(NULL);
        },
        "SNDDTTSK", 4096, nullptr, 20, &DISPLAY_handler);
    if (tsk2_code != pdPASS) {
        return -2;
    }
#endif

    return 0;
}