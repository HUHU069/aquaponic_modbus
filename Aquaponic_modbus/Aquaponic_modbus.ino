#ifdef ESP8266
#include <ESP8266WiFi.h>
#else // ESP32
#include <WiFi.h>
#endif
#include <ModbusIP_ESP8266.h>
#include <DHT.h>

#define SSID "WiFi - Mekatronika"
#define PASSWORD "tanyapakimron"
#define DHTPIN 10
#define DHTTYPE DHT11

IPAddress myIP(10, 4, 106, 202);
IPAddress gateway(10, 4, 106, 254);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(10, 4, 213, 37);

ModbusIP mb;
DHT dht(DHTPIN, DHTTYPE);

void tryConnect(const char *info) {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(SSID, PASSWORD);
    Serial.print("[INFO]");
    Serial.print(info);
    while (WiFi.status() != WL_CONNECTED) {
      delay(250);
      Serial.print(",");
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  }
}

//coil
#define LAMPU 2
#define WATER_PUMP 1
#define pin_WATER_PUMP 14
#define pin_LAMPU 12
//intup reg
#define humidity 3
#define temperature 4

long ts;

void setup() {
  // Serial.begin(115200);

  dht.begin();

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(pin_WATER_PUMP, OUTPUT);
  pinMode(pin_LAMPU, OUTPUT);

  mb.server(502);
  mb.addCoil(WATER_PUMP);
  mb.addCoil(LAMPU);
  mb.addIreg(humidity);
  mb.addIreg(temperature);

  if (!WiFi.config(myIP, gateway, subnet, dns)) {
    Serial.println("[ERROR] STA Failed to configure");
    while (1) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      delay(100);
    }
  }

  ts = millis();
}

void loop() {
  tryConnect("try to connect");
  mb.task();

  uint16_t h = dht.readHumidity();
  uint16_t t = dht.readTemperature();

  mb.Ireg(humidity, h);
  mb.Ireg(temperature, t);

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));


  digitalWrite(pin_WATER_PUMP, !mb.Coil(WATER_PUMP));
  digitalWrite(pin_LAMPU, !mb.Coil(LAMPU));

  //delay(5000);
}
