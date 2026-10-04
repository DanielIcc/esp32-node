/*
  Cliente HTTP del taller ESP32 + API local.

  Ajusta WIFI_SSID, WIFI_PASS y URL_COMANDO antes de cargar.
  URL_COMANDO usa la IP de la computadora donde corre servidor.js.
  No uses 127.0.0.1: en el ESP32 esa dirección es el propio ESP32.
*/

#include <WiFi.h>
#include <HTTPClient.h>

const char* WIFI_SSID = "TU_RED";
const char* WIFI_PASS = "TU_CONTRASENA";
const char* URL_COMANDO = "http://192.168.1.20:3000/comando";

const int LED_PIN = 2;




void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("ESP32 en ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    return;
  }

  WiFiClient cliente;
  HTTPClient http;
  if (!http.begin(cliente, URL_COMANDO)) {
    Serial.println("No se pudo preparar la peticion");
    delay(1000);
    return;
  }

  int codigo = http.GET();
  if (codigo == 200) {
    String cuerpo = http.getString();
    Serial.println(cuerpo);
    bool encender = cuerpo.indexOf("true") >= 0;
    digitalWrite(LED_PIN, encender ? HIGH : LOW);
  } else {
    Serial.printf("Fallo: %d\n", codigo);
  }

  http.end();
  delay(1000);
}
