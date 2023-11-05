#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <SSD1306Wire.h>
#include <WiFi.h>
#include <Wire.h>

#define LED_BUILTIN 2

const char* ssid = "NAME";
const char* password = "PASSWORD";

SSD1306Wire display(0x3c, 5, 4);

int nextUpdate = 0;
const int updateInterval = 1000 * 5;

void setup() {
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);

    WiFi.begin(ssid, password);
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
    }
}

void loop() {
    if ((WiFi.status() == WL_CONNECTED)) {
        if (millis() >= nextUpdate or nextUpdate == 0) {
        nextUpdate = millis() + updateInterval;
        update("crypto-com-chain", "usd");
        }
    } else {
        display.clear();
        display.drawString(0, 0, "Connecting to WiFi..");
        display.display();
    }
}

void update(char* coin_id, char* vs_currency) {
    HTTPClient http;
    String url =
        "https://api.coingecko.com/api/v3/simple/price?ids=" + String(coin_id) +
        "&vs_currencies=" + String(vs_currency) + "&include_24hr_change=true";
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
        String payload = http.getString();
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        Serial.println(String(doc[coin_id][vs_currency].as<float>())) + " " +
            String(doc[coin_id][String(vs_currency) + "_24h_change"].as<float>());
        float price = doc[coin_id][vs_currency].as<float>();
        float percent =
            doc[coin_id][String(vs_currency) + "_24h_change"].as<float>();
        display.clear();
        display.setFont(ArialMT_Plain_16);
        display.drawString(0, 0, "Cronos");
        display.setFont(ArialMT_Plain_24);
        display.drawString(0, 13, "$" + String(price, 5));
        display.setFont(ArialMT_Plain_16);
        display.drawString(0, 40, String(percent, 2) + "%");
        display.display();
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
    } else {
        display.clear();
        display.setFont(ArialMT_Plain_16);
        display.drawString(0, 0, "Error");
        display.display();
    }
    http.end();
}