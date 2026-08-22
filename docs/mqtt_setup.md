# 📡 MQTT Mosquitto Setup Guide / Налаштування MQTT Mosquitto



This guide explains how to install, configure, and test the Mosquitto MQTT broker on a clean Raspberry Pi OS.

Цей гайд пояснює, як встановити, налаштувати та протестувати MQTT-брокер Mosquitto на чистій системі Raspberry Pi OS.



---



## 🛠️ 1. Installation / Встановлення



Open your Raspberry Pi terminal and run the following commands:

Відкрийте термінал вашої Raspberry Pi та виконайте такі команди:



```bash

# Update package list / Оновлюємо список пакетів

sudo apt update



# Install Mosquitto broker and clients / Встановлюємо брокер та клієнтські утиліти

sudo apt install -y mosquitto mosquitto-clients

```



---



## ⚙️ 2. Configuration / Налаштування



By default, external connections are blocked. To allow access for your ESP32-C6, modify the config:

За замовчуванням зовнішні підключення заблоковані. Щоб дозволити доступ вашій ESP32-C6, відредагуйте конфіг:



1. Open the configuration file / Відкрийте файл конфігурації:

```bash

sudo nano /etc/mosquitto/mosquitto.conf

```

2. Append the following lines to the end of the file / Додайте ці рядки в кінець файлу:

```text

listener 1883 0.0.0.0

allow_anonymous true

```

3. Save and exit (**Ctrl+O**, **Enter**, **Ctrl+X**) / Збережіть та вийдіть.

4. Enable autostart and run the service / Увімкніть автозапуск та запустіть службу:

```bash

sudo systemctl enable --now mosquitto

```

5. Check status / Перевірте статус служби:

```bash

sudo systemctl status mosquitto

```



---



## 🔍 3. Diagnostics & Testing / Діагностика та тести



### Subscribe to topics (Listen) / Підписка на топіки (Слухаємо ефір):

```bash

# Listen to the weather station metrics / Слухати метрики метеостанції

mosquitto_sub -h localhost -t "home/weather/metrics" -v



# Listen to absolutely all topics / Слухати абсолютно всі топіки у системі

mosquitto_sub -h localhost -t "#" -v

```



### Publish test messages / Публікація повідомлень (Надсилаємо дані):

```bash

# Simulate your station by sending a test JSON packet / Емуляція надсилання JSON-пакета

mosquitto_pub -h localhost -t "home/weather/metrics" -m '{"boot":1,"temp_aht":25.5,"humidity":42.0,"temp_bmp":25.6,"pressure":752.1}'

```



### Clear a stuck topic / Очищення застряглого топіка:

```bash

mosquitto_pub -h localhost -t "home/weather/metrics" -r -n

```



