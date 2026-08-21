/* 
  ==================================================================================
  ESP32-C6 Autonomous Low-Power IoT Weather Station
  Автономна енергоефективна IoT Метеостанція на ESP32-C6
  
  [EN] ENVIRONMENT SETUP / [UA] НАЛАШТУВАННЯ СЕРЕДОВИЩА:
  ----------------------------------------------------------------------------------
  - Board / Плата:             ESP32C6 Dev Module (or Generic ESP32-C6 Module)
  - Core / Ядро плат:          esp32 by Espressif Systems (v3.0.0 or higher / або вище)
  - Flash Size / Пам'ять:      4MB
  - Tools -> USB CDC On Boot:  "Enabled" ⚠️ (Required for Serial Monitor / Критично для Serial)
  
  REQUIRED LIBRARIES / НЕОБХІДНІ БІБЛІОТЕКИ:
  ----------------------------------------------------------------------------------
  1. PubSubClient            by Nick O'Leary       (v2.8.0+) -> For MQTT communication / Для MQTT
  2. Adafruit NeoPixel       by Adafruit           (v1.12.0+) -> For onboard RGB LED / Для світлодіода
  3. Adafruit AHTX0          by Adafruit           (v2.0.5+) -> For AHT20 Temp/Hum sensor / Для AHT20
  4. Adafruit BMP280 Library by Adafruit           (v2.6.8+) -> For BMP280 Pressure sensor / Для BMP280
  
  License / Ліцензія: MIT
  ==================================================================================
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_NeoPixel.h>

// Include secret Wi-Fi credentials / Підключення секретних налаштувань Wi-Fi
#include "myssid.h" 

// Wi-Fi settings from myssid.h / Налаштування Wi-Fi з файлу myssid.h
const char* ssid = MY_WIFI_SSID;
const char* password = MY_WIFI_PASS;

// MQTT Broker configuration / Налаштування MQTT брокера
const char* mqtt_server = "192.168.1.4"; // Raspberry Pi 5 IP address
const int mqtt_port = 1883;

// Pin Configuration / Конфігурація пінів
#define SENSOR_POWER_PIN 22  // GPIO22 - Sensor VCC / Живлення датчиків
#define I2C_SDA 19           // GPIO19 - I2C Data / Дані I2C
#define I2C_SCL 20           // GPIO20 - I2C Clock / Синхронізація I2C
#define RGB_PIN 8            // GPIO8  - Onboard WS2812B RGB LED / Вбудований світлодіод
#define NUM_PIXELS 1

// Sensor and Client initialization / Ініціалізація датчиків та клієнтів
Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;
Adafruit_NeoPixel rgb(NUM_PIXELS, RGB_PIN, NEO_GRB + NEO_KHZ800);

WiFiClient espClient;
PubSubClient client(espClient);

// Store boot counter in non-volatile RTC memory / Зберігаємо лічильник прокидань в RTC-пам'яті
RTC_DATA_ATTR int bootCount = 0;

// Deep Sleep time (10 minutes = 600 seconds) / Час глибокого сну (10 хвилин = 600 секунд)
const uint64_t SLEEP_TIME_US = 600 * 1000000; 

void setup() {
  // 1. Power up sensors immediately / Подаємо живлення на датчики в першу чергу
  pinMode(SENSOR_POWER_PIN, OUTPUT);
  digitalWrite(SENSOR_POWER_PIN, HIGH);

  Serial.begin(115200);
  delay(500); // Allow USB Serial to initialize / Даємо час для ініціалізації Serial

  bootCount++;
  Serial.println("\n--- IoT Weather Station starting / Стартує ---");
  Serial.printf("Boot cycle / Номер циклу роботи: %d\n", bootCount);

  // Initialize RGB LED (ORANGE - starting process) / Ініціалізація світлодіода (ЖОВТИЙ - старт)
  rgb.begin();
  rgb.setBrightness(20); // Low brightness to save battery / Низька яскравість для економії
  rgb.setPixelColor(0, rgb.Color(255, 100, 0));
  rgb.show();

  // Wait 200ms for sensor voltage stabilization / Даємо 200 мс на стабілізацію живлення датчиків
  delay(200); 

  // Initialize I2C bus on defined pins / Ініціалізація I2C шини на обраних пінах
  Wire.begin(I2C_SDA, I2C_SCL); 

  // Start AHT20 sensor / Запуск AHT20
  if (!aht.begin()) {
    Serial.println("Error: AHT20 not found! / Помилка: AHT20 не знайдено!");
  }

  // Start BMP280 (I2C address 0x77) / Запуск BMP280 (Адреса 0x77)
  if (!bmp.begin(0x77)) {
    Serial.println("Error: BMP280 not found! / Помилка: BMP280 не знайдено!");
  }

  // Configure BMP280 to FORCED mode for power saving / Налаштування BMP280 на одиночний замір
  bmp.setSampling(Adafruit_BMP280::MODE_FORCED,
                  Adafruit_BMP280::SAMPLING_X1, // Temperature / Температура
                  Adafruit_BMP280::SAMPLING_X1, // Pressure / Тиск
                  Adafruit_BMP280::FILTER_OFF);

  // 2. Read sensor metrics / Зчитування метеоданих
  sensors_event_t humidity_event, temp_event;
  aht.getEvent(&humidity_event, &temp_event); 

  float humidity = humidity_event.relative_humidity;
  float temp_aht = temp_event.temperature;

  bmp.takeForcedMeasurement(); // Request single measurement / Робимо один замір прямо зараз
  float temp_bmp = bmp.readTemperature();
  float pressure = bmp.readPressure() / 133.3223; // Pascal to mm Hg / Паскалі в мм рт.ст.

  Serial.printf("AHT20 Temp: %.2f *C, Hum: %.2f %%\n", temp_aht, humidity);
  Serial.printf("BMP280 Temp: %.2f *C, Press: %.2f mm Hg\n", temp_bmp, pressure);

  // 3. Connect to Wi-Fi network / Підключення до Wi-Fi мережі
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi / Підключення до Wi-Fi...");
  
  int attempts = 0;
  // Maximum timeout 15 seconds / Максимальне очікування 15 секунд
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected / Успішно підключено!");
    
    // BLUE LED - MQTT connecting / СИНІЙ колір — підключення до MQTT
    rgb.setPixelColor(0, rgb.Color(0, 0, 255));
    rgb.show();

    // 4. Connect to Mosquitto MQTT Broker / Підключення до MQTT брокера
    client.setServer(mqtt_server, mqtt_port);
    String clientId = "MeteoStation_C6_" + String(bootCount);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("MQTT connected / Підключено!");
      
      // GREEN LED - Data transmitting / ЗЕЛЕНИЙ колір — відправка даних
      rgb.setPixelColor(0, rgb.Color(0, 255, 0));
      rgb.show();

      // Formulate JSON telemetry payload / Формуємо JSON-пакет з метриками
      String payload = "{";
      payload += "\"boot\":" + String(bootCount) + ",";
      payload += "\"temp_aht\":" + String(temp_aht, 2) + ",";
      payload += "\"humidity\":" + String(humidity, 1) + ",";
      payload += "\"temp_bmp\":" + String(temp_bmp, 2) + ",";
      payload += "\"pressure\":" + String(pressure, 1);
      payload += "}";

      // Publish payload to the main metrics topic / Публікація в основний робочий топік
      if (client.publish("home/weather/metrics", payload.c_str())) {
        Serial.println("Data published successfully! / Дані успішно надіслано!");
      } else {
        Serial.println("MQTT publication failed. / Помилка публікації в MQTT.");
      }
      delay(150); // Ensure packet leaves network stack / Даємо пакету гарантовано піти в ефір
    } else {
      Serial.printf("MQTT connection failed, code / Помилка підключення, код: %d\n", client.state());
    }
  } else {
    Serial.println("\nWi-Fi connection failed. / Не вдалося підключитися до Wi-Fi.");
  }

  // 5. Power optimization before sleep / Максимальне енергозбереження перед сном
  Serial.println("Shutting down peripherals / Знеструмлення периферії...");
  
  // Cut off sensor power rail / Вимикаємо живлення датчиків
  digitalWrite(SENSOR_POWER_PIN, LOW);
  pinMode(SENSOR_POWER_PIN, INPUT); // Set pin to High-Z state / Переводим в High-Z стан
  
  // Isolate I2C bus lines / Відпускаємо шину I2C
  pinMode(I2C_SDA, INPUT);
  pinMode(I2C_SCL, INPUT);

  // Turn off onboard RGB LED / Гасимо вбудований світлодіод
  rgb.setPixelColor(0, rgb.Color(0, 0, 0)); 
  rgb.show();
  
  // Turn off radio modems / Вимикаємо радіомодулі
  WiFi.disconnect(true); 
  WiFi.mode(WIFI_OFF);   

  Serial.println("Entering Deep Sleep / Засинаю...");
  Serial.flush(); // Flush Serial buffer / Чекаємо завершення виводу в консоль       

  // 6. Enter Deep Sleep mode / Перехід у глибокий сон
  esp_sleep_enable_timer_wakeup(SLEEP_TIME_US);
  esp_deep_sleep_start();
}

void loop() {
  // Empty. System restarts from setup() / Порожньо. Після сну система перезапускає setup()
}


