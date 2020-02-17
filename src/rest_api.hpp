#include <Arduino.h>

namespace restApi {
const enum HTTP_TYPE {
  HTTP_GET,
  HTTP_POST,
  HTTP_PUT,
  HTTP_DELETE,
};
class restApi {
  restApi() {
    Serial.println("HTTP");
    Serial.println("HTTP");
  }
  void send(const char* payload) {}
};
}  // namespace restApi