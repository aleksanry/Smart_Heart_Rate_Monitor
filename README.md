# Smart Heart Rate Monitor 💓

A comprehensive Arduino-based heart rate monitoring system featuring real-time BPM detection, customizable alarms, and an intuitive OLED interface.

![Heart Rate Monitor](https://img.shields.io/badge/Arduino-Heart%20Rate%20Monitor-blue?style=for-the-badge&logo=arduino)
<img width="853" height="618" alt="image" src="https://github.com/user-attachments/assets/365e926e-640f-475f-b4e7-026230bbd5fc" />

## ✨ Features

- **📊 Real-time Heart Rate Monitoring** - Continuous BPM detection with rolling average
- **🖥️ OLED Display Interface** - Clear, intuitive menu system
- **🚨 Smart Alarm System** - Customizable high/low BPM thresholds
- **📈 Session Statistics** - Track min/max BPM and session duration
- **💡 Visual Indicators** - LED status feedback and heartbeat animation
- **⚙️ User-Friendly Controls** - Three-button navigation system
- **🔧 Sensor Calibration** - Built-in sensor status monitoring

## 🛠️ Hardware Requirements

### Components
- **Arduino Nano v3.x** - Main microcontroller
- **MAX30102** - Heart rate and pulse oximetry sensor
- **SSD1306 OLED Display** (128x64) - User interface
- **3x Push Buttons** (SW1, SW2, SW3) - User input
- **1x LED** (D1) - Status indicator
- **3x Pull-up Resistors** (1kΩ) - Button debouncing
- **3x Capacitors** (100nF) - Power filtering

### Pin Connections

| Component | Arduino Pin | Notes |
|-----------|-------------|-------|
| MAX30102 SDA | A4 | I2C Data |
| MAX30102 SCL | A5 | I2C Clock |
| OLED SDA | A4 | I2C Data |
| OLED SCL | A5 | I2C Clock |
| SW1 (Menu) |pin D2 | Analog input with pull-up |
| SW2 (Up) | pin D3 | Analog input with pull-up |
| SW3 (Down) | pin D4 | Analog input with pull-up |
| Status LED | Pin D12 | Digital output |
| Power | 5V/3.3V | VCC connection |
| Ground | GND | Common ground |

## 📚 Software Requirements

### Arduino Libraries
Install these libraries through the Arduino Library Manager:

```
Adafruit SSD1306        // OLED display driver
Adafruit GFX Library    // Graphics library
MAX30105lib             // SparkFun MAX3010x sensor library
```

### Library Installation
1. Open Arduino IDE
2. Go to **Sketch → Include Library → Manage Libraries**
3. Search and install each library listed above
4. Restart Arduino IDE

## 🚀 Installation & Setup

### 1. Hardware Assembly
- Follow the pin connection table above
- Ensure proper power connections (3.3V or 5V)
- Connect I2C devices to the same bus (SDA/SCL)
- Add pull-up resistors to button connections

### 3. Sensor Calibration
1. Power on the device
2. Navigate to **Settings Menu → Sensor Config**
3. Ensure IR signal shows values > 10000 when finger is placed
4. If values are low, check sensor connections

## 🎮 User Interface Guide

### Button Controls
- **SW1 (Menu)** - Navigate between screens, confirm selections
- **SW2 (Up)** - Navigate up, increase values
- **SW3 (Down)** - Navigate down, decrease values

### Screen Navigation
```
Main Screen
    ↓ (SW1)
Settings Menu
    ↓ (SW1)
┌─ Alarm Settings
└─ Sensor Calibration
```

### Main Display Elements
- **Large BPM Reading** - Current heart rate
- **Status Indicator** - LOW/NORMAL/HIGH
- **Alarm Status** - ON/OFF indication
- **Session Timer** - Minutes and seconds elapsed
- **Heartbeat Animation** - Visual pulse indicator
- **Alarm Banner** - Flashing when triggered

## ⚙️ Configuration Options

### Alarm Settings
- **Minimum BPM**: Default 60, Range 30-120
- **Maximum BPM**: Default 100, Range 80-220
- **Alarm Enable/Disable**: Toggle alarm functionality

### Sensor Parameters
- **LED Brightness**: 60 (adjustable in code)
- **Sample Rate**: 400 Hz
- **Sample Average**: 4 readings
- **Pulse Width**: 411 µs

## 🔍 Troubleshooting

### Common Issues

**No Heart Rate Reading**
- Check sensor placement (fingertip on sensor)
- Verify MAX30102 connections
- Ensure adequate lighting conditions
- Check I2C address (default 0x57)

**Display Not Working**
- Verify OLED connections (SDA/SCL)
- Check I2C address (default 0x3C)
- Ensure adequate power supply

**Buttons Not Responding**
- Check analog pin connections
- Verify pull-up resistors are installed
- Test button continuity

**Inaccurate Readings**
- Keep finger steady during measurement
- Avoid excessive pressure on sensor
- Wait for stabilization (10-15 seconds)

### Debug Mode
Enable serial monitoring at 115200 baud for diagnostic information:
```cpp
Serial.begin(115200);  // Already included in code
```

## 📊 Technical Specifications

### Performance
- **Update Rate**: 20ms sensor reading, 500ms display refresh
- **Accuracy**: ±2 BPM (typical)
- **Range**: 20-220 BPM
- **Response Time**: 3-5 seconds for stable reading

### Power Consumption
- **Active Mode**: ~50mA @ 5V
- **Display On**: ~30mA additional
- **Sensor Active**: ~15mA additional

### Memory Usage
- **Program Memory**: ~75% of Arduino Nano
- **Dynamic Memory**: ~60% of available RAM

## 🔧 Customization

### Modifying Thresholds
```cpp
// Default alarm thresholds (in setup())
int minHeartRate = 60;    // Lower limit
int maxHeartRate = 100;   // Upper limit
```

### Display Update Rate
```cpp
// Update intervals (in milliseconds)
const unsigned long DISPLAY_UPDATE_INTERVAL = 500;  // 500ms
const unsigned long SENSOR_READ_INTERVAL = 20;      // 20ms
```

### LED Behavior
```cpp
// Alarm LED patterns
digitalWrite(LED_PIN, (millis() / 250) % 2);  // Fast blink (250ms)
digitalWrite(LED_PIN, (millis() / 1000) % 2); // Slow blink (1000ms)
```


### Code Style Guidelines
- Use descriptive variable names
- Comment complex algorithms
- Follow Arduino naming conventions
- Test on actual hardware before submitting



## 🙏 Acknowledgments

- **SparkFun** for the excellent MAX30105 library
- **Adafruit** for the comprehensive display libraries
- **Arduino Community** for continuous support and inspiration


## 🔮 Future Enhancements

- [ ] Bluetooth connectivity for data logging
- [ ] SD card storage for long-term monitoring
- [ ] Mobile app integration
- [ ] Multiple user profiles
- [ ] Advanced analytics and trends
- [ ] Voice alerts
- [ ] Battery level monitoring

---

**⚠️ Medical Disclaimer**: This device is for educational and recreational purposes only. It is not intended for medical diagnosis or treatment. Always consult healthcare professionals for medical advice.

---

Made with ❤️ by Nermin | Arduino Heart Rate Monitor v1.0
