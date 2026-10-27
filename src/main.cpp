
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// Credenciais da rede Wi-Fi
const char* ssid = "iPhone de Brunooo";
const char* password = "brunoo123";

WebServer server(80);

// Variáveis para armazenar os dados climáticos
float currentTemp = 0.0;
float currentWind = 0.0;
float currentDirection = 0.0;

bool weatherAvailable = false;

void fetchWeatherData() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi desconectado.");
    weatherAvailable = false;
    return;
  }

  WiFiClientSecure client;
  client.setInsecure(); // Apenas para testes

  HTTPClient http;

  // URL corrigida: parâmetros atuais da API Open-Meteo
  String apiUrl =
      "https://api.open-meteo.com/v1/forecast"
      "?latitude=-23.49968"
      "&longitude=-46.89192"
      "&current=temperature_2m,wind_speed_10m,wind_direction_10m";

  if (!http.begin(client, apiUrl)) {
    Serial.println("Falha ao iniciar a requisição HTTP.");
    weatherAvailable = false;
    return;
  }

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      // Os valores atuais ficam dentro do objeto "current"
      JsonObject current = doc["current"];

      if (!current.isNull() &&
          !current["temperature_2m"].isNull() &&
          !current["wind_speed_10m"].isNull() &&
          !current["wind_direction_10m"].isNull()) {

        currentTemp = current["temperature_2m"].as<float>();
        currentWind = current["wind_speed_10m"].as<float>();
        currentDirection = current["wind_direction_10m"].as<float>();

        weatherAvailable = true;

        Serial.printf(
            "Temperatura: %.1f C | Vento: %.1f km/h | Direcao: %.1f graus\n",
            currentTemp, currentWind, currentDirection
        );
      } else {
        Serial.println("Os dados climaticos nao foram encontrados no JSON.");
        weatherAvailable = false;
      }
    } else {
      Serial.print("Falha ao ler JSON: ");
      Serial.println(error.c_str());
      weatherAvailable = false;
    }
  } else {
    Serial.printf("Erro HTTP: %d\n", httpCode);
    weatherAvailable = false;
  }

  http.end();
}

void handleRoot() {
  fetchWeatherData();

  String html = "<!DOCTYPE html><html lang='pt-BR'><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<meta http-equiv='refresh' content='60'>";
  html += "<title>Painel Climatico - ESP32</title>";

  html += "<style>";
  html += "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; ";
  html += "background: #7a9ceb; color: #f8fafc; display: flex; ";
  html += "justify-content: center; align-items: center; min-height: 100vh; margin: 0; }";

  html += ".card { background: #0062ff69; padding: 2rem; border-radius: 12px; ";
  html += "text-align: center; max-width: 400px; width: 90%; ";
  html += "border: 1px solid #334155; box-shadow: 0 10px 25px rgba(0,0,0,0.3); }";

  html += "h1 { color: #ffffff; font-size: 1.2rem; text-transform: uppercase; ";
  html += "letter-spacing: 1px; margin-bottom: 1.5rem; }";

  html += ".metric { font-size: 2.5rem; font-weight: bold; ";
  html += "color: #f1f5f9; margin: 10px 0; }";

  html += ".sub-metric { font-size: 1.1rem; color: #131313; margin-bottom: 20px; }";

  html += ".btn { background: #00ccff; color: #0f172a; border: none; ";
  html += "padding: 12px 24px; font-weight: bold; border-radius: 6px; ";
  html += "cursor: pointer; transition: 0.2s; }";

  html += ".btn:hover { background: #0284c7; color: #ffffff; }";
  html += ".error { color: #ffffff; font-weight: bold; }";
  html += "</style></head><body>";

  html += "<div class='card'>";
  html += "<h1>Clima em Tempo Real (Carapicuiba)</h1>";

  if (weatherAvailable) {
    html += "<div class='metric'>" + String(currentTemp, 1) + " &deg;C</div>";
    html += "<div class='sub-metric'>Vento: " + String(currentWind, 1) + " km/h</div>";
    html += "<div class='sub-metric'>Direcao: " + String(currentDirection, 1) + " &deg;</div>";
  } else {
    html += "<p class='error'>Nao foi possivel obter os dados climaticos.</p>";
    html += "<p>Tente atualizar a pagina.</p>";
  }

  html += "<button class='btn' onclick='location.reload()'>Atualizar Dados</button>";
  html += "</div></body></html>";

  server.send(200, "text/html; charset=UTF-8", html);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(ssid, password);

  Serial.print("Conectando ao Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi conectado!");
  Serial.print("Endereco IP do ESP32: http://");
  Serial.println(WiFi.localIP()); 

  server.on("/", handleRoot);
  server.begin();

  Serial.println("Servidor HTTP iniciado com sucesso.");
}

void loop() {
  server.handleClient();
}
