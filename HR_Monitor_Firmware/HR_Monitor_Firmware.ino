#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MAX30105.h>
#include <heartRate.h>

// OLED Display Settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// MAX30102 Heart Rate Sensor
MAX30105 particleSensor;

// Button Pins (based on schematic)
#define SW1_BTN 2  // SW1 connected to  D2
#define SW2_BTN 3    // SW2 connected to D3 
#define SW3_BTN 4    // SW3 connected to D4
#define LED_PIN 12   // LED connected to D12

// Button mapping
#define MENU_BTN SW1_BTN
#define UP_BTN SW2_BTN
#define DOWN_BTN SW3_BTN

// System Variables
const byte RATE_ARRAY_SIZE = 4;
long rates[RATE_ARRAY_SIZE];
long rateArray[RATE_ARRAY_SIZE];
byte rateArrayIndex = 0;
long lastBeat = 0;
int beatsPerMinute;
byte ledBrightness = 60;
byte sampleAverage = 4;
byte ledMode = 2;
int sampleRate = 400;
int pulseWidth = 411;
int adcRange = 4096;

// Menu System
enum MenuState {
  MAIN_SCREEN,
  SETTINGS_MENU,
  ALARM_SETTINGS,
  SENSOR_CALIBRATION
};

MenuState currentMenu = MAIN_SCREEN;
int menuSelection = 0;

// Heart Rate Thresholds
int minHeartRate = 60;
int maxHeartRate = 100;
bool alarmEnabled = true;
bool isAlarmTriggered = false;

// Timing Variables
unsigned long lastDisplayUpdate = 0;
unsigned long lastSensorRead = 0;
unsigned long alarmStartTime = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 500;
const unsigned long SENSOR_READ_INTERVAL = 20;
const unsigned long ALARM_DURATION = 5000;

// Button States
bool menuPressed = false;
bool upPressed = false;
bool downPressed = false;
bool ledState = false;

// Heart Rate Statistics
long totalBeats = 0;
long sessionStart = 0;
int minRecordedBPM = 999;
int maxRecordedBPM = 0;

void setup() {
  Serial.begin(115200);
  
  // Initialize OLED Display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Heart Rate Monitor"));
  display.println(F("Initializing..."));
  display.display();
  
  // Initialize MAX30102 Sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println(F("MAX30105 not found"));
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Sensor Error!"));
    display.println(F("Check connections"));
    display.display();
    while (1);
  }
  
  // Configure sensor
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
  
  // Initialize buttons and LED
  pinMode(SW1_BTN, INPUT_PULLUP);
  pinMode(SW2_BTN, INPUT_PULLUP);
  pinMode(SW3_BTN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Initialize session
  sessionStart = millis();
  
  delay(1000);
  display.clearDisplay();
  display.display();
}

void loop() {
  unsigned long currentTime = millis();
  
  // Read buttons
  readButtons();
  
  // Handle menu navigation
  handleMenu();
  
  // Read heart rate sensor
  if (currentTime - lastSensorRead >= SENSOR_READ_INTERVAL) {
    readHeartRate();
    lastSensorRead = currentTime;
  }
  
  // Update display
  if (currentTime - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    updateDisplay();
    lastDisplayUpdate = currentTime;
  }
  
  // Check alarms
  checkAlarms();
  
  delay(10);
}

void readButtons() {
  // Debounced button reading for analog pins
  static unsigned long lastButtonRead = 0;
  if (millis() - lastButtonRead < 50) return;
  lastButtonRead = millis();
  
  // Read analog values and convert to digital (buttons pull to GND when pressed)
  menuPressed = (analogRead(MENU_BTN) < 512);  // SW1
  upPressed = (analogRead(UP_BTN) < 512);      // SW2  
  downPressed = (analogRead(DOWN_BTN) < 512);  // SW3
}

void handleMenu() {
  static bool prevMenuPressed = false;
  static bool prevUpPressed = false;
  static bool prevDownPressed = false;
  
  // Menu button pressed
  if (menuPressed && !prevMenuPressed) {
    switch (currentMenu) {
      case MAIN_SCREEN:
        currentMenu = SETTINGS_MENU;
        menuSelection = 0;
        break;
      case SETTINGS_MENU:
        if (menuSelection == 0) currentMenu = ALARM_SETTINGS;
        else if (menuSelection == 1) currentMenu = SENSOR_CALIBRATION;
        else currentMenu = MAIN_SCREEN;
        break;
      case ALARM_SETTINGS:
      case SENSOR_CALIBRATION:
        currentMenu = SETTINGS_MENU;
        break;
    }
  }
  
  // Navigation buttons
  if (upPressed && !prevUpPressed) {
    if (currentMenu == SETTINGS_MENU) {
      menuSelection = (menuSelection > 0) ? menuSelection - 1 : 2;
    } else if (currentMenu == ALARM_SETTINGS) {
      if (menuSelection == 0) minHeartRate = min(minHeartRate + 5, 120);
      else if (menuSelection == 1) maxHeartRate = min(maxHeartRate + 5, 220);
      else if (menuSelection == 2) alarmEnabled = !alarmEnabled;
    }
  }
  
  if (downPressed && !prevDownPressed) {
    if (currentMenu == SETTINGS_MENU) {
      menuSelection = (menuSelection < 2) ? menuSelection + 1 : 0;
    } else if (currentMenu == ALARM_SETTINGS) {
      if (menuSelection == 0) minHeartRate = max(minHeartRate - 5, 30);
      else if (menuSelection == 1) maxHeartRate = max(maxHeartRate - 5, 80);
      else if (menuSelection == 2) alarmEnabled = !alarmEnabled;
    }
  }
  
  prevMenuPressed = menuPressed;
  prevUpPressed = upPressed;
  prevDownPressed = downPressed;
}

void readHeartRate() {
  long irValue = particleSensor.getIR();
  
  if (checkForBeat(irValue)) {
    // Calculate time between beats
    long delta = millis() - lastBeat;
    lastBeat = millis();
    
    // Store valid heart rate
    if (delta > 300 && delta < 2000) { // Valid heart rate range
      beatsPerMinute = 60 / (delta / 1000.0);
      
      if (beatsPerMinute < 255 && beatsPerMinute > 20) {
        // Store this reading in the array
        rates[rateArrayIndex++] = beatsPerMinute;
        rateArrayIndex %= RATE_ARRAY_SIZE;
        
        // Calculate average of readings
        long total = 0;
        for (byte i = 0; i < RATE_ARRAY_SIZE; i++) {
          total += rates[i];
        }
        beatsPerMinute = total / RATE_ARRAY_SIZE;
        
        // Update statistics
        totalBeats++;
        if (beatsPerMinute > maxRecordedBPM) maxRecordedBPM = beatsPerMinute;
        if (beatsPerMinute < minRecordedBPM) minRecordedBPM = beatsPerMinute;
      }
    }
  }
}

void updateDisplay() {
  display.clearDisplay();
  
  switch (currentMenu) {
    case MAIN_SCREEN:
      displayMainScreen();
      break;
    case SETTINGS_MENU:
      displaySettingsMenu();
      break;
    case ALARM_SETTINGS:
      displayAlarmSettings();
      break;
    case SENSOR_CALIBRATION:
      displaySensorCalibration();
      break;
  }
  
  display.display();
}

void displayMainScreen() {
  // Header
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("Heart Rate Monitor"));
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  // Main BPM Display
  display.setTextSize(2);
  display.setCursor(10, 20);
  if (beatsPerMinute > 0) {
    display.print(beatsPerMinute);
    display.println(F(" BPM"));
  } else {
    display.println(F("-- BPM"));
  }
  
  // Status indicators
  display.setTextSize(1);
  display.setCursor(0, 45);
  
  // Heart rate status
  if (beatsPerMinute > 0) {
    if (beatsPerMinute < minHeartRate) {
      display.print(F("LOW"));
    } else if (beatsPerMinute > maxHeartRate) {
      display.print(F("HIGH"));
    } else {
      display.print(F("NORMAL"));
    }
  } else {
    display.print(F("DETECTING"));
  }
  
  // Alarm status
  display.setCursor(70, 45);
  if (alarmEnabled) {
    display.print(F("ALARM:ON"));
  } else {
    display.print(F("ALARM:OFF"));
  }
  
  // Session info
  display.setCursor(0, 55);
  unsigned long sessionTime = (millis() - sessionStart) / 1000;
  display.print(F("Time: "));
  display.print(sessionTime / 60);
  display.print(F("m "));
  display.print(sessionTime % 60);
  display.print(F("s"));
  
  // Visual heartbeat indicator
  if (millis() - lastBeat < 200 && beatsPerMinute > 0) {
    display.fillCircle(120, 20, 4, SSD1306_WHITE);
  } else {
    display.drawCircle(120, 20, 4, SSD1306_WHITE);
  }
  
  // Alarm indicator
  if (isAlarmTriggered) {
    display.fillRect(100, 0, 28, 8, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(102, 1);
    display.print(F("ALARM"));
    display.setTextColor(SSD1306_WHITE);
  }
}

void displaySettingsMenu() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("Settings Menu"));
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  const char* menuItems[] = {"Alarm Settings", "Sensor Config", "Back to Main"};
  
  for (int i = 0; i < 3; i++) {
    display.setCursor(10, 20 + i * 12);
    if (i == menuSelection) {
      display.print(F("> "));
    } else {
      display.print(F("  "));
    }
    display.println(menuItems[i]);
  }
  
  display.setCursor(0, 58);
  display.print(F("MENU:Select UP/DOWN:Nav"));
}

void displayAlarmSettings() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("Alarm Settings"));
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  display.setCursor(0, 20);
  display.print(F("Min BPM: "));
  display.println(minHeartRate);
  
  display.setCursor(0, 32);
  display.print(F("Max BPM: "));
  display.println(maxHeartRate);
  
  display.setCursor(0, 44);
  display.print(F("Alarm: "));
  display.println(alarmEnabled ? F("Enabled") : F("Disabled"));
  
  // Selection indicator
  display.setCursor(110, 20 + menuSelection * 12);
  display.print(F("<"));
  
  display.setCursor(0, 58);
  display.print(F("UP/DOWN:Change/Toggle"));
}

void displaySensorCalibration() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("Sensor Status"));
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  display.setCursor(0, 20);
  display.print(F("IR Signal: "));
  display.println(particleSensor.getIR());
  
  display.setCursor(0, 32);
  display.print(F("Min BPM: "));
  display.println(minRecordedBPM == 999 ? 0 : minRecordedBPM);
  
  display.setCursor(0, 44);
  display.print(F("Max BPM: "));
  display.println(maxRecordedBPM);
  
  display.setCursor(0, 58);
  display.print(F("MENU:Back"));
}

void checkAlarms() {
  if (!alarmEnabled) {
    digitalWrite(LED_PIN, LOW);
    return;
  }
  
  if (beatsPerMinute > 0) {
    if (beatsPerMinute < minHeartRate || beatsPerMinute > maxHeartRate) {
      if (!isAlarmTriggered) {
        isAlarmTriggered = true;
        alarmStartTime = millis();
        Serial.println(F("ALARM: Heart rate out of range!"));
      }
      
      // Blink LED during alarm
      digitalWrite(LED_PIN, (millis() / 250) % 2);
    } else {
      digitalWrite(LED_PIN, LOW);
      isAlarmTriggered = false;
    }
  } else {
    // Slow blink when no signal detected
    digitalWrite(LED_PIN, (millis() / 1000) % 2);
  }
  
  // Auto-silence alarm after duration
  if (isAlarmTriggered && (millis() - alarmStartTime > ALARM_DURATION)) {
    isAlarmTriggered = false;
    digitalWrite(LED_PIN, LOW);
  }
}
