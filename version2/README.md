# 💊 Medibox Enhancement 

This repository contains the implementation for enhancing the **Medibox**, a device designed to assist users in managing their medication schedules effectively. The enhanced Medibox monitors environmental conditions and dynamically regulates storage conditions for light-sensitive and temperature-sensitive medicines.

## 📜 Overview

The project involves:

- Monitoring **light intensity** and **temperature** inside the Medibox using an **LDR** and a **DHT11** sensor.
- Dynamically adjusting a **servo-controlled shaded sliding window** based on environmental readings.
- Sending sensor data to a **Node-RED dashboard** via MQTT for visualization and parameter control.
- Allowing users to configure parameters like sampling intervals, sending intervals, ideal storage temperature, and servo control factors.

The system is implemented on an **ESP32 microcontroller** and simulated using **Wokwi**.

---

## 📂 Repository Structure


