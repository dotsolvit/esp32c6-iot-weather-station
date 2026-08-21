\# 📡 MQTT Mosquitto Setup Guide / Налаштування MQTT Mosquitto



This guide explains how to install, configure, and test the Mosquitto MQTT broker on a clean Raspberry Pi OS.

Цей гайд пояснює, як встановити, налаштувати та протестувати MQTT-брокер Mosquitto на чистій системі Raspberry Pi OS.



\---



\## 🛠️ 1. Installation / Встановлення



Open your Raspberry Pi terminal and run the following commands:

Відкрийте термінал вашої Raspberry Pi та виконайте такі команди:



```bash

\# Update package list / Оновлюємо список пакетів

sudo apt update



\# Install Mosquitto broker and clients / Встановлюємо брокер та клієнтські утиліти

sudo apt install -y mosquitto mosquitto-clients

```



\---



\## ⚙️ 2. Configuration / Налаштування



By default, external connections are blocked. To allow access for your ESP32-C6, modify the config:

За замовчуванням зовнішні підключення заблоковані. Щоб дозволити доступ вашій ESP32-C6, відредагуйте конфіг:



1\. Open the configuration file / Відкрийте файл конфігурації:

&#x20;  ```bash

&#x20;  sudo nano /etc/mosquitto/mosquitto.conf

&#x20;  ```

2\. Append the following lines to the end of the file / Додайте ці рядки в кінець файлу:

&#x20;  ```text

&#x20;  listener 1883 0.0.0.0

&#x20;  allow\_anonymous true

&#x20;  ```

3\. Save and exit (\*\*Ctrl+O\*\*, \*\*Enter\*\*, \*\*Ctrl+X\*\*) / Збережіть та вийдіть.

4\. Enable autostart and run the service / Увімкніть автозапуск та запустіть службу:

&#x20;  ```bash

&#x20;  sudo systemctl enable --now mosquitto

&#x20;  ```

5\. Check status / Перевірте статус служби:

&#x20;  ```bash

&#x20;  sudo systemctl status mosquitto

&#x20;  ```



\---



\## 🔍 3. Diagnostics \& Testing / Діагностика та тести



\### Subscribe to topics (Listen) / Підписка на топіки (Слухаємо ефір):

```bash

\# Listen to the weather station metrics / Слухати метрики метеостанції

mosquitto\_sub -h localhost -t "home/weather/metrics" -v



\# Listen to absolutely all topics / Слухати абсолютно всі топіки у системі

mosquitto\_sub -h localhost -t "#" -v

```



\### Publish test messages / Публікація повідомлень (Надсилаємо дані):

```bash

\# Simulate your station by sending a test JSON packet / Емуляція надсилання JSON-пакета

mosquitto\_pub -h localhost -t "home/weather/metrics" -m '{"boot":1,"temp\_aht":25.5,"humidity":42.0,"temp\_bmp":25.6,"pressure":752.1}'

```



\### Clear a stuck topic / Очищення застряглого топіка:

```bash

mosquitto\_pub -h localhost -t "home/weather/metrics" -r -n

```



