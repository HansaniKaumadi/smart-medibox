#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <time.h>
#include <DHTesp.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

// NTP server settings for time synchronization
#define NTP_SERVER "pool.ntp.org" // NTP server for time sync
#define UTC_OFFSET 0 // UTC offset in seconds
#define UTC_OFFSET_DST 0 // Daylight saving time offset

// OLED display configuration
#define SCREEN_WIDTH 128 // OLED display width in pixels
#define SCREEN_HEIGHT 64 // OLED display height in pixels
#define OLED_RESET -1 // Reset pin (not used)
#define SCREEN_ADDRESS 0x3C // I2C address of OLED

// Pin definitions for hardware components
#define BUZZER 18 // Buzzer pin for alarms
#define LED_1 15 // LED for alarm indication
#define LED_2 2 // LED for environmental alerts
#define CANCEL 34 // Cancel button pin
#define UP 35 // Up button pin
#define DOWN 32 // Down button pin
#define OK 33 // OK button pin
#define DHT 12 // DHT22 sensor pin
#define LDR 39 // Light Dependent Resistor pin
#define SERVO 4 // Servo motor pin

// Musical notes for buzzer alarm
int n_notes = 8; // Number of musical notes
int C = 262; // Frequency for C note
int D = 294; // Frequency for D note
int E = 330; // Frequency for E note
int F = 349; // Frequency for F note
int G = 392; // Frequency for G note
int A = 440; // Frequency for A note
int B = 494; // Frequency for B note
int C_H = 523; // Frequency for high C note
int notes[] = {C, D, E, F, G, A, B, C_H}; // Array of note frequencies

// Time variables
int year = 0; // Current year
int month = 0; // Current month
int days = 0; // Current day
int hours = 0; // Current hour
int minutes = 0; // Current minute
int seconds = 0; // Current second

// Servo control variables
int tita = 0; // Current servo angle (degrees)
int titaoffset = 30; // Minimum servo angle (default)
float gammaVal = 0.75; // Control factor for servo equation (default)
float temp = 0; // Measured temperature (°C)
float tempmed = 30; // Ideal storage temperature (°C, default)

// Light intensity variables
int intensitycount = 0; // Count of intensity samples
float intensity = 0; // Current light intensity (0–1)
float cumuIntensity = 0; // Cumulative intensity for averaging

// Alarm system variables
bool alarm_enabled = false; // Global alarm enable flag
int n_alarms = 2; // Number of alarms
bool enable_Alrm[] = {false, false}; // Enable flags for alarms
int alarm_hours[] = {0, 0}; // Alarm hours
int alarm_minutes[] = {0, 0}; // Alarm minutes
bool alarm_triggered[] = {false, false}; // Alarm triggered flags
bool alarm_snoozed[] = {false, false}; // Alarm snoozed flags

// Timing variables
unsigned long timeNow = 0; // Current time in milliseconds
unsigned long timeLast = 0; // Last intensity sampling time
unsigned long timelastmin = 0; // Last intensity sending time

// Configurable intervals
int Ts = 5; // Sampling interval in seconds (default)
int Tu = 1; // Sending interval in minutes (default)

// Menu system variables
int current_mode = 1; // Current menu mode (1-based)
int max_modes = 4; // Total number of menu modes
String options[] = {"1 - Set Time", "2 - Set Alarm 1", "3 - Set Alrm 2", "4 - View Alarms"}; // Menu options

// Initialize hardware objects
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // OLED display object
DHTesp dhtSensor; // DHT22 sensor object
Servo servo; // Servo motor object

// MQTT configuration
const char* mqtt_server = "broker.hivemq.com"; // MQTT broker address
WiFiClient espClient; // WiFi client for MQTT
PubSubClient client(espClient); // MQTT client

// Callback function to handle incoming MQTT messages
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  // Print payload
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Convert payload to string
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';

  // Update parameters based on topic
  if (strcmp(topic, "Medibox735E/intensitySampling") == 0) {
    Ts = atoi(message); // Update sampling interval
  }
  if (strcmp(topic, "Medibox735E/intensitySending") == 0) {
    Tu = atoi(message); // Update sending interval
  }
  if (strcmp(topic, "Medibox735E/titaoffset") == 0) {
    titaoffset = atoi(message); // Update minimum servo angle
  }
  if (strcmp(topic, "Medibox735E/controlFact") == 0) {
    gammaVal = atof(message); // Update control factor
  }
  if (strcmp(topic, "Medibox735E/tempmed") == 0) {
    tempmed = atof(message); // Update ideal temperature
  }
}

// Reconnect to MQTT broker if connection is lost
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-"; // Generate unique client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect with will message
    if (client.connect(clientId.c_str(), "Medibox735E/status", 0, true, "ESP32 Disconnected")) {
      Serial.println("connected");
      client.publish("Medibox735E/status", "ESP32 Connected to MQTT", true); // Publish connection status
      // Subscribe to control topics
      client.subscribe("Medibox735E/intensitySampling");
      client.subscribe("Medibox735E/intensitySending");
      client.subscribe("Medibox735E/titaoffset");
      client.subscribe("Medibox735E/controlFact");
      client.subscribe("Medibox735E/tempmed");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000); // Wait before retrying
    }
  }
}

// Display text on OLED at specified position and size
void print_line(String text, int text_size, int row, int column) {
  display.setTextSize(text_size); // Set text size
  display.setTextColor(SSD1306_WHITE); // Set text color to white
  display.setCursor(column, row); // Set cursor position
  display.println(text); // Print text
  display.display(); // Update display
}

// Monitor temperature and humidity, trigger alerts if out of range
void check_temp() {
  TempAndHumidity data = dhtSensor.getTempAndHumidity(); // Read sensor data
  temp = data.temperature; // Store temperature
  Serial.println("Publishing Temperature: " + String(temp, 1));
  client.publish("Medibox735E/temperature", String(temp, 1).c_str(), true); // Publish temperature
  bool all_good = true; // Flag for environmental status

  // Check temperature thresholds
  if (data.temperature > 32) {
    all_good = false;
    digitalWrite(LED_2, HIGH); // Turn on alert LED
    print_line("TEMP HIGH ", 1, 55, 0); // Display alert
  } else if (data.temperature < 25) {
    all_good = false;
    digitalWrite(LED_2, HIGH);
    print_line("TEMP LOW ", 1, 55, 0);
  }
  // Check humidity thresholds
  if (data.humidity > 80) {
    all_good = false;
    digitalWrite(LED_2, HIGH);
    print_line("HUMD HIGH", 1, 45, 0);
  } else if (data.humidity < 65) {
    all_good = false;
    digitalWrite(LED_2, HIGH);
    print_line("HUMD LOW", 1, 45, 0);
  }
  // Display all good if no alerts
  if (all_good) {
    digitalWrite(LED_2, LOW); // Turn off alert LED
    print_line("All Good", 1, 50, 0);
  }
}

// Display current date and time on OLED
void print_time_now(void) {
  display.clearDisplay(); // Clear OLED
  // Format and display date
  String dateString = String(year) + "-" + (month < 10 ? "0" : "") + String(month) + "-" + (days < 10 ? "0" : "") + String(days);
  print_line(dateString, 1, 0, 60);
  // Format and display time
  String timeString = (hours < 10 ? "0" : "") + String(hours) + ":" +
                      (minutes < 10 ? "0" : "") + String(minutes) + ":" +
                      (seconds < 10 ? "0" : "") + String(seconds);
  print_line(timeString, 2, 25, 20);
  display.display(); // Update display
}

// Snooze an alarm for 5 minutes
void snoozeAlarm(int i) {
  noTone(BUZZER); // Stop buzzer
  delay(200);
  digitalWrite(LED_1, LOW); // Turn off alarm LED
  display.clearDisplay();
  print_line("Snoozed Alarm... " + String(i), 1, 40, 0); // Display snooze message
  Serial.println("Snoozed Alarm... " + String(i));
  // Calculate new alarm time (5 minutes later)
  int temphour = hours;
  int tempminute = minutes;
  if (tempminute + 5 >= 60) {
    temphour += 1;
    tempminute = (tempminute + 5) % 60;
  } else {
    tempminute += 5;
  }
  alarm_hours[i] = temphour; // Update alarm hour
  alarm_minutes[i] = tempminute; // Update alarm minute
  alarm_triggered[i] = false; // Reset triggered flag
  enable_Alrm[i] = true; // Keep alarm enabled
  alarm_enabled = true; // Enable global alarm
  alarm_snoozed[i] = true; // Mark as snoozed
  delay(1000);
  display.clearDisplay(); // Clear display
}

// Play alarm sound and handle user interaction
void ring_alarm(int alarm) {
  alarm_triggered[alarm] = true; // Mark alarm as triggered
  display.clearDisplay();
  print_line("Medicine Time", 1, 40, 0); // Display alarm message
  digitalWrite(LED_1, HIGH); // Turn on alarm LED
  int count = 0; // Count alarm cycles
  while (digitalRead(CANCEL) == HIGH) { // Loop until cancel pressed
    for (int i = 0; i < n_notes; i++) { // Play notes
      if (digitalRead(CANCEL) == LOW) { // Check for cancel
        Serial.println("Alarm off");
        alarm_enabled = false; // Disable global alarm
        enable_Alrm[alarm] = false; // Disable this alarm
        alarm_triggered[alarm] = true; // Keep triggered
        noTone(BUZZER); // Stop buzzer
        delay(200);
        digitalWrite(LED_1, LOW); // Turn off LED
        return;
      }
      tone(BUZZER, notes[i]); // Play note
      delay(500);
      noTone(BUZZER); // Stop note
      delay(2);
      if (!alarm_snoozed[alarm]) { // Auto-snooze if not already snoozed
        if (count > 5) { // After 5 cycles
          snoozeAlarm(alarm); // Snooze alarm
          digitalWrite(LED_1, LOW); // Turn off LED
          return;
        }
      }
    }
    Serial.println("Alarm on" + String(count));
    count++; // Increment cycle count
  }
}

// Update system time from NTP
void update_time(void) {
  struct tm timeinfo; // Time structure
  getLocalTime(&timeinfo); // Get current time
  // Convert time components to strings
  char year_str[8];
  char month_str[8];
  char day_str[8];
  char hour_str[8];
  char min_str[8];
  char sec_str[8];
  strftime(year_str, 8, "%Y", &timeinfo);
  strftime(month_str, 8, "%m", &timeinfo);
  strftime(day_str, 8, "%d", &timeinfo);
  strftime(sec_str, 8, "%S", &timeinfo);
  strftime(hour_str, 8, "%H", &timeinfo);
  strftime(min_str, 8, "%M", &timeinfo);
  // Update global time variables
  year = atoi(year_str);
  month = atoi(month_str);
  days = atoi(day_str);
  hours = atoi(hour_str);
  minutes = atoi(min_str);
  seconds = atoi(sec_str);
  // Adjust for overflow
  if (minutes >= 60) {
    minutes -= 60;
    hours += 1;
  }
  if (hours >= 24) {
    hours -= 24;
    days += 1;
  }
}

// Update time and check for alarm triggers
void update_time_with_check_alarm() {
  update_time(); // Update system time
  // Disable global alarm if no alarms are enabled
  if (enable_Alrm[0] == false && enable_Alrm[1] == false) {
    alarm_enabled = false;
  }
  if (alarm_enabled) { // Check for alarm triggers
    for (int i = 0; i < n_alarms; i++) {
      if (alarm_triggered[i] == false && hours == alarm_hours[i] && minutes == alarm_minutes[i]) {
        Serial.println("Alarm ON");
        ring_alarm(i); // Trigger alarm
      }
    }
  }
}

// Wait for button press and return button ID
int wait_for_button_press() {
  while (true) {
    if (digitalRead(UP) == LOW) { // Up button pressed
      delay(200); // Debounce
      return UP;
    } else if (digitalRead(DOWN) == LOW) { // Down button pressed
      delay(200);
      return DOWN;
    } else if (digitalRead(CANCEL) == LOW) { // Cancel button pressed
      delay(200);
      return CANCEL;
    } else if (digitalRead(OK) == LOW) { // OK button pressed
      delay(200);
      return OK;
    }
    update_time(); // Keep time updated
  }
}

// Set system time manually
void set_time() {
  int temp_hour = 5; // Default hour
  while (true) {
    display.clearDisplay();
    print_line("Enter hour: " + String(temp_hour), 0, 0, 2); // Display hour prompt
    int pressed = wait_for_button_press(); // Wait for input
    if (pressed == UP) {
      delay(20);
      temp_hour++; // Increment hour
      if (temp_hour > 12) {
        temp_hour = -12; // Wrap around
      }
    } else if (pressed == DOWN) {
      delay(20);
      temp_hour--; // Decrement hour
      if (temp_hour < -12) {
        temp_hour = 12;
      }
    } else if (pressed == OK) {
      delay(200);
      break; // Confirm hour
    } else if (pressed == CANCEL) {
      delay(200);
      return; // Exit
    }
  }
  int temp_minute = 30; // Default minute
  while (true) {
    display.clearDisplay();
    print_line("Enter minute: " + String(temp_minute), 0, 0, 2); // Display minute prompt
    int pressed = wait_for_button_press();
    if (pressed == UP) {
      delay(200);
      temp_minute++; // Increment minute
      temp_minute = temp_minute % 60; // Wrap at 60
    } else if (pressed == DOWN) {
      delay(200);
      temp_minute--; // Decrement minute
      if (temp_minute < 0) {
        temp_minute = 59;
      }
    } else if (pressed == OK) {
      delay(200);
      break; // Confirm minute
    } else if (pressed == CANCEL) {
      delay(200);
      return; // Exit
    }
  }
  // Calculate UTC offset in seconds
  int offSec = 0;
  if (temp_hour < 0) {
    offSec = temp_hour * 60 * 60 - temp_minute * 60;
  } else {
    offSec = temp_hour * 60 * 60 + temp_minute * 60;
  }
  configTime(offSec, UTC_OFFSET_DST, NTP_SERVER); // Set NTP time
  update_time(); // Update system time
  display.clearDisplay();
  print_line("Time is set", 0, 0, 2); // Confirm time set
  delay(1000);
  display.clearDisplay();
  print_time_now(); // Show new time
  delay(1000);
  display.clearDisplay();
}

// Set alarm time for a specific alarm
void set_alarm(int alarm) {
  int temp_hour = hours; // Start with current hour
  while (true) {
    display.clearDisplay();
    print_line("Enter hour: " + String(temp_hour), 0, 0, 2); // Display hour prompt
    int pressed = wait_for_button_press();
    if (pressed == UP) {
      delay(200);
      temp_hour++; // Increment hour
      temp_hour = temp_hour % 24; // Wrap at 24
    } else if (pressed == DOWN) {
      delay(200);
      temp_hour--; // Decrement hour
      if (temp_hour < 0) {
        temp_hour = 23;
      }
    } else if (pressed == OK) {
      delay(200);
      alarm_hours[alarm] = temp_hour; // Set alarm hour
      break;
    } else if (pressed == CANCEL) {
      display.clearDisplay();
      print_line("Set Alarm Canceled! ", 0, 0, 2); // Cancel message
      delay(200);
      return;
    }
  }
  int temp_minute = minutes; // Start with current minute
  while (true) {
    display.clearDisplay();
    print_line("Enter minute: " + String(temp_minute), 0, 0, 2); // Display minute prompt
    int pressed = wait_for_button_press();
    if (pressed == UP) {
      delay(200);
      temp_minute++; // Increment minute
      temp_minute = temp_minute % 60;
    } else if (pressed == DOWN) {
      delay(200);
      temp_minute--; // Decrement minute
      if (temp_minute < 0) {
        temp_minute = 59;
      }
    } else if (pressed == OK) {
      delay(200);
      alarm_minutes[alarm] = temp_minute; // Set alarm minute
      break;
    } else if (pressed == CANCEL) {
      display.clearDisplay();
      print_line("Set Time Canceled! ", 0, 0, 2); // Cancel message
      delay(200);
      return;
    }
  }
  enable_Alrm[alarm] = true; // Enable alarm
  alarm_enabled = true; // Enable global alarm
  alarm_triggered[alarm] = false; // Reset triggered flag
  alarm_snoozed[alarm] = false; // Reset snoozed flag
  display.clearDisplay();
  Serial.println("Alarm " + String(alarm + 1) + " " + alarm_hours[alarm] + ":" + alarm_minutes[alarm] + " SET");
  print_line("Alarm  " + String(alarm) + " " + alarm_hours[alarm] + ":" + alarm_minutes[alarm] + " SET", 0, 0, 2); // Confirm alarm
  delay(1000);
  display.clearDisplay();
}

// View and manage alarms
void viewAlarms() {
  while (true) {
    display.clearDisplay();
    for (int i = 0; i < n_alarms; i++) { // Display all alarms
      String alarmString;
      if (enable_Alrm[i]) {
        alarmString = "Alarm " + String(i + 1) + ": " + String(alarm_hours[i]) + ":" + String(alarm_minutes[i]);
      } else {
        alarmString = "Alarm " + String(i + 1) + ": OFF";
      }
      print_line(alarmString, 1, i * 10, 0);
    }
    if (digitalRead(OK) == LOW) { // Exit on OK
      delay(200);
      display.clearDisplay();
      print_line("Exiting Alarm View", 1, 40, 0);
      delay(1000);
      return;
    }
    if (digitalRead(CANCEL) == LOW) { // Enter delete mode
      delay(200);
      display.clearDisplay();
      print_line("Delete Alarms !", 1, 10, 0);
      if (enable_Alrm[0]) {
        print_line("Alarm 1 - Press Up to Delete", 1, 25, 0);
      }
      if (enable_Alrm[1]) {
        print_line("Alarm 2 - Press Down to Delete", 1, 45, 0);
      }
      if (!enable_Alrm[0] && !enable_Alrm[1]) {
        print_line("No Alarms to Delete.", 1, 40, 0);
      }
      int press = wait_for_button_press(); // Wait for input
      if (press == CANCEL) { // Exit delete mode
        display.clearDisplay();
        print_line("Exiting Alarm View", 1, 40, 0);
        return;
      }
      if (press == UP && enable_Alrm[0]) { // Delete alarm 1
        display.clearDisplay();
        enable_Alrm[0] = false;
        alarm_snoozed[0] = false;
        print_line("Alarm 1 Deleted", 1, 10, 0);
        delay(1000);
      }
      if (press == DOWN && enable_Alrm[1]) { // Delete alarm 2
        display.clearDisplay();
        enable_Alrm[1] = false;
        alarm_snoozed[1] = false;
        print_line("Alarm 2 Deleted", 1, 30, 0);
        delay(1000);
      }
    }
    delay(5); // Small delay for responsiveness
  }
}

// Execute selected menu mode
void run_mode(int mode) {
  if (mode == 0) {
    Serial.println("Set Time");
    set_time(); // Set system time
  } else if (mode == 1 || mode == 2) {
    set_alarm(mode - 1); // Set alarm 1 or 2
  } else if (mode == 3) {
    viewAlarms(); // View/delete alarms
  }
}

// Navigate main menu
void go_to_menu() {
  while (digitalRead(CANCEL) == HIGH) { // Stay in menu until cancel
    update_time_with_check_alarm(); // Keep time and alarms updated
    display.clearDisplay();
    print_line(options[current_mode - 1], 1, 0, 0); // Display current option
    int pressed = wait_for_button_press();
    if (pressed == UP) {
      current_mode++; // Next mode
      if (current_mode > max_modes) {
        current_mode = 1; // Wrap around
      }
      delay(20);
    } else if (pressed == DOWN) {
      current_mode--; // Previous mode
      if (current_mode < 1) {
        current_mode = max_modes;
      }
      delay(20);
    } else if (pressed == OK) {
      Serial.print("Selected Mode: ");
      Serial.println(current_mode);
      delay(200);
      run_mode(current_mode - 1); // Run selected mode
    }
  }
}

// Check if it's time to sample light intensity
bool Sample_intensity() {
  timeNow = millis(); // Current time
  if (timeNow - timeLast >= Ts * 1000) { // Check if Ts seconds elapsed
    timeLast = timeNow; // Update last sample time
    return true;
  }
  return false;
}

// Check if it's time to send average intensity
bool Send_avg_intensity() {
  timeNow = millis();
  int thresholdTime = Tu * 60000; // Convert Tu to milliseconds
  if (timeNow - timelastmin >= thresholdTime) { // Check if Tu minutes elapsed
    timelastmin = timeNow; // Update last send time
    Serial.println("Sending average intensity (Tu = " + String(Tu) + " min)");
    return true;
  }
  return false;
}

// Sample and send light intensity data
void check_intensity() {
  if (Send_avg_intensity()) { // Time to send average
    if (intensitycount > 0) { // If samples exist
      float avgIntensity = cumuIntensity / intensitycount; // Calculate average
      Serial.println("Publishing Intensity: " + String(avgIntensity, 2) + ", Samples: " + String(intensitycount));
      bool published = client.publish("Medibox735E/intensity", String(avgIntensity, 2).c_str()); // Publish average
      if (!published) {
        Serial.println("Failed to publish intensity");
      }
    } else {
      Serial.println("No intensity samples collected");
    }
    intensitycount = 0; // Reset sample count
    cumuIntensity = 0; // Reset cumulative intensity
  }
  if (Sample_intensity()) { // Time to sample
    int ldrValue = analogRead(LDR); // Read LDR
    intensity = float(ldrValue) / 4095.0; // Normalize to 0–1
    Serial.println("LDR Raw: " + String(ldrValue) + ", Intensity: " + String(intensity, 2));
    cumuIntensity += intensity; // Add to cumulative
    intensitycount++; // Increment sample count
  }
}

// Control servo angle based on environmental conditions
void control_servo() {
  // Calculate servo angle using given equation
  int rawtita = titaoffset + (180 - titaoffset) * intensity * gammaVal * log(float(Ts) / float(Tu * 60)) * (float(temp) / float(tempmed));
  tita = constrain(rawtita, 0, 180); // Limit to 0–180°
  servo.attach(SERVO); // Attach servo
  servo.write(tita); // Set angle
}

// Setup function to initialize hardware and connections
void setup() {
  Serial.begin(9600); // Start serial communication
  // Initialize pins
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(CANCEL, INPUT);
  pinMode(UP, INPUT);
  pinMode(DOWN, INPUT);
  pinMode(OK, INPUT);
  dhtSensor.setup(DHT, DHTesp::DHT22); // Initialize DHT22
  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Halt if failed
  }
  display.display(); // Show initial display
  delay(2000);
  display.clearDisplay();
  print_line("Welcome to Medibox", 1, 40, 10); // Welcome message
  delay(3000);
  display.clearDisplay();
  // Connect to WiFi
  WiFi.begin("Wokwi-GUEST", "");
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    print_line("Connecting to WiFi...", 1, 0, 0);
  }
  display.clearDisplay();
  print_line("WiFi Connected", 1, 0, 0); // Confirm WiFi
  delay(2000);
  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER); // Initialize NTP
  // Setup MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  client.publish("Medibox735E/intensity", String(0).c_str()); // Initial publish
  // Initialize servo
  servo.attach(SERVO);
  servo.write(tita);
}

// Main loop to run continuous tasks
void loop() {
  if (!client.connected()) {
    reconnect(); // Reconnect to MQTT if needed
  }
  update_time_with_check_alarm(); // Update time and check alarms
  print_time_now(); // Display time
  check_intensity(); // Handle LDR sampling and publishing
  check_temp(); // Monitor temperature/humidity
  control_servo(); // Update servo angle
  // Display alarm status
  if (enable_Alrm[0]) {
    print_line("Alarm 1 ON ", 1, 45, 60);
  }
  if (enable_Alrm[1]) {
    print_line("Alarm 2 ON", 1, 55, 60);
  }
  if (digitalRead(CANCEL) == LOW) { // Enter menu on cancel press
    Serial.println("MENU");
    delay(1000);
    go_to_menu();
  }
  delay(20); // Small delay for stability
  client.loop(); // Handle MQTT callbacks
}