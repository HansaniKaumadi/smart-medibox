# 💊 Medibox Enhancement 

This repository contains the implementation for enhancing the **Medibox**, a device designed to assist users in managing their medication schedules effectively. The enhanced Medibox monitors environmental conditions and dynamically regulates storage conditions for light-sensitive and temperature-sensitive medicines.

## 📜 Overview

The project involves:

- Monitoring **light intensity** and **temperature** inside the Medibox using an **LDR** and a **DHT11** sensor.
- Dynamically adjusting a **servo-controlled shaded sliding window** based on environmental readings.
- Sending sensor data to a **Node-RED dashboard** via MQTT for visualization and parameter control.
- Allowing users to configure parameters like sampling intervals, sending intervals, ideal storage temperature, and servo control factors.

The system is implemented on an **ESP32 microcontroller** and simulated using **Wokwi**.

<p align="center">
  <img src="https://github.com/user-attachments/assets/c0e69fb4-1133-48e6-9d72-5ebe1c0a8a8a" width="60%"/>

 
</p>



<p align="center">
  <img src="https://github.com/user-attachments/assets/990819e3-34fd-4c7c-897b-698d9b33f81a" width="70%"/>
</p> 


---

## 🛠 Features Implemented

✅ **Light Monitoring (LDR)**  
- Samples light intensity every 5 seconds (default).  
- Averages readings over 2 minutes (default).  
- Sends averaged data to Node-RED via MQTT.  
- Configurable sampling and sending intervals.

✅ **Temperature Monitoring (DHT11)**  
- Reads ambient temperature in the storage area.  

✅ **Dynamic Light Regulation**  
- Controls a servo motor to adjust a shaded sliding window.  
- Uses the formula:  
θ = θoffset + (180 − θoffset) × I × γ × ln(ts/tu) × T/Tmed

where:
- θ = Servo angle
- θoffset = Minimum angle (user adjustable)
- I = Normalized light intensity (0–1)
- γ = Controlling factor (user adjustable)
- T = Measured temperature (°C)
- Tmed = Ideal medicine storage temperature (user adjustable)

✅ **Node-RED Dashboard**  
- Displays current average light intensity (0–1 scale).  
- Shows historical light intensity on a live chart.  
- Includes sliders for user-adjustable parameters:
- Sampling interval (ts)
- Sending interval (tu)
- Minimum servo angle (θoffset)
- Controlling factor (γ)
- Ideal storage temperature (Tmed)

✅ **MQTT Integration**  
- Uses `test.mosquitto.org` as the MQTT broker for data exchange.

---

## 🖥️ Node-RED Dashboard

The dashboard includes:  
- 📊 **Light Intensity Visualization:** Gauge and chart.  
- 🎚️ **User Controls:** Sliders for adjusting sampling rates, sending intervals, and environmental parameters.  
- 🌡️ **Temperature Display.**

---

