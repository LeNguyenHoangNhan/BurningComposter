#include "task.hpp"

extern LiquidCrystal_I2C lcd;
extern OneWire onewire;
extern String UUID;
extern

    HumiditySensor hs(35);
DallasTemperature ts(&onewire);
ArduinoJson::StaticJsonDocument<512> jsonDoc;
HTTPClient http;

float humd{0.0};
float temp{0.0};

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
int init_task() {
    BaseType_t tsk1_code = xTaskCreatePinnedToCore(
        [](void *pvParameters) {
            for (;;) {
                ts.requestTemperatures();
                humd = hs.readSensorPercent();
                temp = ts.getTempCByIndex(0);
                Serial.printf("Temp: %.2f, Humd: %.2f\n", temp, humd);
                vTaskDelay(2000 / portTICK_PERIOD_MS);
            }
            vTaskDelete(NULL);
        },
        "RDSSTSK", 4096, nullptr, 10, &RDSSTSK_handler, 1);
    if (tsk1_code != pdPASS) {
        return -1;
    }
    BaseType_t tsk2_code = xTaskCreate(
        [](void *pvParameters) {
            for (;;) {
                vTaskDelay(6000);
                if (WiFi.isConnected() == true && WiFi.status() == WL_CONNECTED) {
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
        "DSPLYTSK", 4096, nullptr, 20, &DISPLAY_handler);
    if (tsk2_code != pdPASS) {
        return -2;
    }

    return 0;
}