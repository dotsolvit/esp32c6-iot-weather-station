# 🐳 Docker & Home Assistant Setup Guide / Налаштування Docker та Home Assistant



This guide explains how to deploy Home Assistant Core inside a Docker container on Raspberry Pi OS.

Цей гайд пояснює, як розгорнути Home Assistant Core всередині Docker-контейнера на Raspberry Pi OS.



---



## 🛠️ 1. Install Docker / Встановлення Docker



Run the official installation script and configure user permissions:

Запустіть офіційний скрипт встановлення та налаштуйте права користувача:



```bash

# Download and install Docker / Завантаження та інсталяція Docker

curl -fsSL https://docker.com -o get-docker.sh && sudo sh get-docker.sh



# Add your user to the docker group / Додаємо вашого юзера до групи docker

# Replace 'oleksandr' with your actual username / Замініть 'oleksandr' на вашого юзера

sudo usermod -aG docker oleksandr

```



⚠️ **Important / Важливо:** Reboot your Raspberry Pi (`sudo reboot`) after this step to apply group permissions.

Обов'язково перезавантажте малинку (`sudo reboot`) після цього кроку, щоб права групи набули чинності.



---



## 🚀 2. Deploy Home Assistant / Запуск Home Assistant



Create a directory for configuration persistent storage and run the container:

Створіть директорію для постійного зберігання конфігурації та запустіть контейнер:



```bash

# Create directory / Створюємо папку

mkdir -p /home/oleksandr/homeassistant/config



# Run Home Assistant Core container / Запуск контейнера Home Assistant

docker run -d '\'

 --name homeassistant '\'

 --privileged '\'

 --restart unless-stopped '\'

 -v /home/oleksandr/homeassistant/config:/config '\'

 -v /etc/localtime:/etc/localtime:ro '\'

 --net=host '\'

 ghcr.io/home-assistant/home-assistant:stable

```



---



## 🌐 3. Web Interface & Configuration / Веб-інтерфейс та інтеграція



1. Open your browser and navigate to / Відкрийте браузер за адресою: `http://<YOUR_RPI_IP>:8123` (e.g., `http://192.168.1.4:8123`).

2. Create an admin account / Створіть обліковий запис адміністратора.

3. Go to **Settings -> Devices & Integrations** and add the **MQTT** integration using `localhost` as the broker address.

&#x20;  Перейдіть у **Налаштування -> Пристрої та інтеграції** та додайте інтеграцію **MQTT**, вказавши `localhost` як адресу брокера.

4. Open the `configuration.yaml` file in your Pi's terminal to add your custom sensors:

&#x20;  Відкрийте файл `configuration.yaml` у терміналі вашої малинки, щоб додати кастомні сенсори:

&#x20;  ```bash

&#x20;  sudo nano /home/oleksandr/homeassistant/config/configuration.yaml

&#x20;  ```

5. Paste the contents from the `home-assistant/configuration_snippet.yaml` file from this repository into the end of the file.

&#x20;  Вставте вміст файлу `home-assistant/configuration_snippet.yaml` з цього репозиторію в кінець файлу.

6. Restart Home Assistant manually configured MQTT items via **Developer Tools -> YAML** interface.

&#x20;  Перезапустіть налаштування MQTT через інтерфейс **Інструменти розробника -> YAML**.



