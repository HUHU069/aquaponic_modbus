#ifdef ESP8266
#include <ESP8266WiFi.h>
#else  // ESP32
#include <WiFi.h>
#endif
#include <ModbusIP_ESP8266.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

#define SSID "WiFi - Mekatronika"
#define PASSWORD "tanyapakimron"
#define DHTPIN 5
#define DHTTYPE DHT11

IPAddress myIP(10, 4, 106, 202);
IPAddress gateway(10, 4, 106, 254);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(10, 4, 213, 37);

ModbusIP mb;
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x3f, 16, 2);

float h;
float t;

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

#define LAMPU 2
#define WATER_PUMP 1
#define pin_WATER_PUMP 14
#define pin_LAMPU 12
#define humidity 3
#define temperature 4

unsigned long time_now = 0;
unsigned long period = 2000 ;  // Periode pembacaan sensor (2 detik)

void setup() {
  Serial.begin(9600);
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

  lcd.init();  // initialize the lcd
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.backlight();
  time_now = millis();
}

void loop() {
  tryConnect("try to connect");
  mb.task();

  // Baca sensor setiap periode
  if (millis() - time_now > period) {
    time_now = millis();
    h = dht.readHumidity();
    t = dht.readTemperature();
    
    // Kirim data sensor ke Modbus
    mb.Ireg(humidity, (uint16_t)(h * 10));  // dikalikan 10 untuk mendapatkan nilai desimal
    mb.Ireg(temperature, (uint16_t)(t * 10));  // dikalikan 10 untuk mendapatkan nilai desimal
  }

  // Kontrol perangkat berdasarkan nilai Modbus
  digitalWrite(pin_WATER_PUMP, !mb.Coil(WATER_PUMP));
  digitalWrite(pin_LAMPU, !mb.Coil(LAMPU));

  // Tampilkan data pada LCD
  if (millis() - time_now > period) {
    time_now = millis();

    if (isnan(h) || isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));

      // Menampilkan pesan kesalahan di LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sensor Error!");
      lcd.setCursor(0, 1);
      lcd.print("Check connection");

      // Tunggu beberapa detik sebelum melanjutkan
      delay(5000);
      return;
    }

    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C "));

    lcd.setCursor(0, 0);
    lcd.print("Temperatur:");
    lcd.setCursor(0, 1);
    lcd.print("Kelembapan:");

    lcd.setCursor(14, 0);
    lcd.print(" C");
    lcd.setCursor(14, 1);
    lcd.print(" %");

    lcd.setCursor(12, 0);
    lcd.print((uint16_t)t);

    lcd.setCursor(12, 1);
    lcd.print((uint16_t)h);


  }
}
