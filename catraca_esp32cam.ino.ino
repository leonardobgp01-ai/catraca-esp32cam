#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

//
// ================= WIFI =================
//
const char* ssid = "SEU_WIFI";
const char* password = "SENHA_WIFI";

//
// ================= API =================
//
const char* apiUrl = "http://SEU_SERVIDOR/api/verificar";

//
// ================= RELÉ =================
//
#define RELE_PIN 12

//
// ========= PINOS ESP32-CAM AI THINKER =========
//
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

//
// ================= SETUP CAMERA =================
//
void startCamera() {
  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;

  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK) {
    Serial.println("Erro ao iniciar camera");
    return;
  }

  Serial.println("Camera iniciada");
}

//
// ================= WIFI =================
//
void connectWiFi() {
  WiFi.begin(ssid, password);

  Serial.print("Conectando");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
}

//
// ================= ABRIR CATRACA =================
//
void abrirCatraca() {
  Serial.println("ACESSO LIBERADO");

  digitalWrite(RELE_PIN, HIGH);

  delay(3000);

  digitalWrite(RELE_PIN, LOW);
}

//
// ================= ENVIAR FOTO =================
//
bool enviarFotoParaAPI() {

  camera_fb_t * fb = esp_camera_fb_get();

  if (!fb) {
    Serial.println("Erro ao capturar foto");
    return false;
  }

  HTTPClient http;

  http.begin(apiUrl);

  http.addHeader("Content-Type", "image/jpeg");

  int httpResponseCode = http.POST(fb->buf, fb->len);

  bool autorizado = false;

  if (httpResponseCode > 0) {

    String response = http.getString();

    Serial.println(response);

    DynamicJsonDocument doc(256);

    deserializeJson(doc, response);

    autorizado = doc["autorizado"];
  }
  else {
    Serial.println("Erro na requisição");
  }

  http.end();

  esp_camera_fb_return(fb);

  return autorizado;
}

//
// ================= SETUP =================
//
void setup() {

  Serial.begin(115200);

  pinMode(RELE_PIN, OUTPUT);

  digitalWrite(RELE_PIN, LOW);

  startCamera();

  connectWiFi();
}

//
// ================= LOOP =================
//
void loop() {

  bool acessoLiberado = enviarFotoParaAPI();

  if (acessoLiberado) {
    abrirCatraca();
  }
  else {
    Serial.println("Acesso negado");
  }

  delay(5000);
}