📸 SCUBA CAM — ESP32-CAM Smart OLED Camera

SCUBA CAM is a DIY camera project based on the ESP32-CAM AI Thinker, integrated with an SSD1306 OLED display, microSD photo storage, and a custom animated startup UI.

This project combines embedded camera processing, real-time OLED visualization, and simple file management into a standalone mini digital camera system.

✨ Features
📺 Real-time grayscale live preview on OLED (128×64)
📷 JPEG photo capture using ESP32-CAM
💾 Save images directly to microSD card
🔘 Physical shutter button for capturing images
💡 Manual flash toggle button
🧠 EEPROM-based auto photo numbering system
🎬 Custom startup animation sequence:
"SCUBA CAM" splash screen
Loading spinner animation
Animated scuba cat GIF
🔁 Dual camera modes:
Preview mode (low-resolution grayscale stream)
Photo mode (high-quality JPEG capture)
⚡ Optimized PSRAM framebuffer handling for stability
🧰 Hardware Requirements
ESP32-CAM AI Thinker
SSD1306 OLED Display (128×64, I2C)
microSD card module (SD_MMC interface)
2x Push buttons (Shutter + Flash control)
Built-in or external flash LED (GPIO 4)
Jumper wires & stable 5V power supply
🔌 Pin Configuration
OLED (I2C)
OLED Pin	ESP32-CAM
SDA	GPIO 14
SCL	GPIO 15
VCC	3.3V
GND	GND
Buttons & Flash LED
Function	GPIO
Shutter Button	13
Flash Button	12
Flash LED	4
Camera Pins (AI Thinker Default)

Uses the standard ESP32-CAM AI Thinker camera pin configuration (Y2–Y9, VSYNC, HREF, PCLK, etc.).

⚙️ How It Works
1. Startup Sequence

When powered on, the system runs:

OLED initialization
SCUBA CAM splash screen
Loading spinner animation
Scuba cat GIF animation
Camera preview mode activation
2. Preview Mode
Camera runs in 96×96 grayscale mode
Frames are downscaled and rendered on OLED
Provides real-time live preview
3. Capture Mode

When the shutter button is pressed:

OLED display freezes
Camera switches to JPEG mode
Image is captured
File is saved to microSD
EEPROM increments photo counter
System returns to preview mode
4. Flash Control

A dedicated button toggles the flash LED manually (GPIO 4).

📁 File Output

Captured images are stored in the format:

/photo_1.jpg
/photo_2.jpg
/photo_3.jpg
...

Photo numbering is automatically managed using EEPROM memory.

🚀 Installation
1. Required Libraries

Install these libraries in Arduino IDE:

Adafruit GFX Library
Adafruit SSD1306
ESP32 Board Support Package
EEPROM Library
2. Board Settings

Configure Arduino IDE as follows:

Board: AI Thinker ESP32-CAM
Partition Scheme: Huge APP (3MB No OTA)
PSRAM: Enabled
3. Upload Method

Because this is ESP32-CAM:

Connect FTDI programmer
Connect GPIO 0 → GND during upload
Press RESET after flashing
⚠️ Important Notes
Ensure PSRAM is enabled for stable camera operation
microSD card must be formatted as FAT32
OLED address is typically 0x3C
Use a stable 5V 1A power supply
Avoid weak power sources to prevent camera reset issues
🧠 Project Purpose

This project was developed for learning and experimenting with:

Embedded vision systems
Real-time OLED rendering techniques
ESP32-CAM memory optimization
Dual camera mode switching
Hardware + IoT integration
📌 Future Improvements
WiFi image upload (Telegram / server integration)
Web-based gallery viewer
Battery-powered version
Auto flash based on light detection
Touch input support
Improved image compression tuning
👨‍💻 Author

DIY Embedded Systems Project
SCUBA CAM Edition — Built with ESP32-CAM + OLED + SD Card system
