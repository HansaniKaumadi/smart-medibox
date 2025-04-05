# MediBox: ESP32-based Medicine Reminder System

## Overview
**MediBox** is a smart medication reminder system built using an ESP32 microcontroller.  
The system features:
- Time-based medication reminders  
- Environmental monitoring (temperature and humidity)  
- A user-friendly interface  

It helps users manage their medication schedule while ensuring medications are stored under proper environmental conditions.

---

## Hardware Components

- **ESP32 DevKit V1 microcontroller**  
- **SSD1306 OLED display** (128x64 pixels)  
- **DHT22 temperature and humidity sensor**  
- **Buzzer** for alarm notifications  
- **LED indicator** for visual alerts  
- **4 pushbuttons** for user interaction:
  - UP button (blue)
  - DOWN button (yellow)
  - OK/MENU button (green)
  - CANCEL button (red)  
- **Resistors** (100Ω, 220Ω, 1000Ω)  
- **Breadboard** for component connections  

---

## Features

### 1. Medication Reminders
- Set up to **2 daily medication alarms**  
- **Snooze functionality** (5 minutes)  
- **Visual and audible notifications**  

### 2. Environmental Monitoring
- Real-time **temperature and humidity display**  
- **Configurable thresholds**:
  - Temperature: 24°C - 32°C  
  - Humidity: 65% - 80%  
- **Alerts** when environment exceeds healthy ranges  

### 3. Time Management
- Real-time clock synchronized via **NTP**  
- Configurable **time zone settings**  
- **Date and time** display  

### 4. User Interface
- **Menu-driven interface** for all settings  
- OLED display for clear visual feedback  
- **Button navigation system**  

---

## Software Dependencies

Include the following libraries in your Arduino IDE:

- `Wire.h`  
- `Adafruit_GFX.h`  
- `Adafruit_SSD1306.h`  
- `DHTesp.h`  
- `WiFi.h`  
- `time.h`  

---

## Installation

1. Connect all hardware components as per the circuit diagram  
2. Install the **Arduino IDE**  
3. Install the required libraries via **Library Manager**:
   - Adafruit GFX Library  
   - Adafruit SSD1306  
   - DHT sensor library for ESPx  
   - WiFi library (comes with ESP32 board package)  
4. Select your **ESP32 board** in the Arduino IDE  
5. Upload the code to your ESP32  

---

## Usage Instructions

### Basic Operation
- Main screen displays:
  - Current time
  - Date
  - Time zone
  - Environmental readings  
- Press **OK/MENU** to enter the settings menu  
- Temperature and humidity are continuously monitored  

### Menu Navigation
- **UP/DOWN**: Navigate menu options  
- **OK**: Select menu option  
- **CANCEL**: Return to previous screen  

### Available Menu Options
- **Set Time Zone**: Configure your local UTC offset  
- **Set Alarm 1**: Set time for first medication reminder  
- **Set Alarm 2**: Set time for second medication reminder  
- **View Alarms**: Check configured alarms  
- **Delete Alarm**: Remove an alarm setting  

### When an Alarm Triggers
- Buzzer will sound and LED will light up  
- Display will show "**Medicine Time!**" with alarm number  

**Options:**
- Press **CANCEL** to stop the alarm  
- Press **OK** to snooze for 5 minutes  

---

## Environmental Monitoring

- Temperature and humidity are displayed on the main screen  
- Visual alerts for thresholds:
  - `TEMP HIGH!` (above 32°C)  
  - `TEMP LOW!` (below 24°C)  
  - `HUM HIGH!` (above 80%)  
  - `HUM LOW!` (below 65%)  

---

## Circuit Connections

| Component      | ESP32 Pin |
|----------------|-----------|
| DHT22          | 12        |
| OLED SDA       | 21        |
| OLED SCL       | 22        |
| Buzzer         | 5         |
| LED            | 15        |
| CANCEL Button  | 34        |
| OK/MENU Button | 32        |
| UP Button      | 33        |
| DOWN Button    | 35        |

---

## Customization

You can modify the following parameters in the code:

- `TEMP_MIN` and `TEMP_MAX`: Temperature thresholds  
- `HUM_MIN` and `HUM_MAX`: Humidity thresholds  
- `UTC_OFFSET`: Default time zone offset (in seconds)  
- `n_alarms`: Number of supported alarms (default: 2)  

---

## Troubleshooting

- **Display not initializing?** Check I2C connections  
- **Time not updating?** Verify WiFi connection  
- **Environmental readings inaccurate?** Check DHT22 connections  

---

## Future Enhancements

- Support for additional alarm slots  
- EEPROM-based persistent storage for settings  
- Battery level monitoring  
- Medication tracking and history logging  
- Bluetooth integration for mobile app support  
