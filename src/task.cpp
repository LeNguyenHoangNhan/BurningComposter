#include "task.hpp"

extern LiquidCrystal_I2C lcd;
extern OneWire onewire;
HumiditySensor hs;
DallasTemperature ts(&onewire);

float humd;
float temp;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
int init_task() {
    BaseType_t tsk1_code = xTaskCreate(
        [](void *pvParameters) {
            for (;;) {
                humd = hs.readSensorPercent();
                temp = ts.getTempCByIndex(0);
                vTaskDelay(2000 / portTICK_PERIOD_MS);
            }
            vTaskDelete(NULL);
        },
        "RDSSTSK", 4096, nullptr, 10, &RDSSTSK_handler);
    if (tsk1_code != pdPASS) {
        return -1;
    }
    BaseType_t tsk2_code = xTaskCreate(
        [](void *pvParameters) {
            for (;;) {
                vTaskDelay(1000);
            }
            vTaskDelete(NULL);
        },
        "DSPLYTSK", 4096, nullptr, 20, &DISPLAY_handler);
    if (tsk2_code != pdPASS) {
        return -2;
    }

    return 0;
}