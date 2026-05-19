#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "esp_camera.h"

#include "FS.h"
#include "SD_MMC.h"

#include <EEPROM.h>

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// =====================================================
// INCLUDE GIF HEADER
// =====================================================
#include "kucing_scuba.h"

// =====================================================
// EEPROM
// =====================================================
#define EEPROM_SIZE 1

// =====================================================
// OLED
// =====================================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// =====================================================
// BUTTON
// =====================================================
#define SHUTTER_BUTTON 13
#define FLASH_BUTTON   12

// =====================================================
// FLASH LED
// =====================================================
#define FLASH_LED 4

bool flashState = false;
bool lastFlashButton = HIGH;

// =====================================================
// CAMERA PINS AI THINKER
// =====================================================
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0

#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5

#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// =====================================================

int pictureNumber = 0;

// =====================================================
// OLED INIT
// =====================================================
void initOLED() {

  Wire.begin(14, 15);

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        SCREEN_ADDRESS)) {

    Serial.println("OLED FAIL");
    return;
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.display();
}

// =====================================================
// OLED MESSAGE (teks di tengah layar)
// =====================================================
void oledMessage(String text) {

  display.clearDisplay();

  display.setTextSize(1);

  // Hitung posisi X agar teks center horizontal
  // Setiap karakter lebar 6px (textSize=1)
  int16_t x = (SCREEN_WIDTH - (text.length() * 6)) / 2;
  if (x < 0) x = 0;

  // Center vertical: baris tunggal tinggi 8px, center di y=28
  display.setCursor(x, 28);

  display.println(text);

  display.display();
}

// =====================================================
// STOP OLED
// =====================================================
void stopOLED() {

  display.clearDisplay();
  display.display();

  Wire.end();
}

// =====================================================
// STARTUP: SCUBA CAM SPLASH
// =====================================================
void showSplashScreen() {

  display.clearDisplay();

  // Judul besar di tengah: textSize=2 → tiap char 12px wide, 16px tall
  display.setTextSize(2);

  String title = "SCUBA CAM";
  // Lebar: 9 karakter x 12px = 108px → x = (128-108)/2 = 10
  display.setCursor(10, 24);
  display.println(title);

  display.display();

  delay(2000);
}

// =====================================================
// STARTUP: LOADING SPINNER
// =====================================================
void showLoadingSpinner() {

  // Karakter spinner yang berputar
  const char spinner[] = {'|', '/', '-', '\\'};
  int spinCount = 16; // Berapa kali berputar

  for (int i = 0; i < spinCount; i++) {

    display.clearDisplay();

    // Teks "Loading..." di atas
    display.setTextSize(1);
    String loadText = "Loading...";
    int16_t lx = (SCREEN_WIDTH - (loadText.length() * 6)) / 2;
    display.setCursor(lx, 20);
    display.println(loadText);

    // Spinner karakter di tengah bawah
    display.setTextSize(2);
    display.setCursor(58, 36); // Center: (128 - 12) / 2 = 58
    display.print(spinner[i % 4]);

    display.display();

    delay(100); // Kecepatan spin
  }
}

// =====================================================
// STARTUP: TAMPILKAN GIF SCUBA CAT
// =====================================================
void showScubaCatGIF() {

  // Loop melalui setiap frame GIF
  for (uint8_t f = 0; f < scuba_cat.frame_count; f++) {

    display.clearDisplay();

    // Hitung offset agar GIF center di OLED
    int16_t offsetX = (SCREEN_WIDTH  - scuba_cat.width)  / 2;
    int16_t offsetY = (SCREEN_HEIGHT - scuba_cat.height) / 2;
    if (offsetX < 0) offsetX = 0;
    if (offsetY < 0) offsetY = 0;

    // Gambar frame satu pixel per satu pixel
    // Frame disimpan sebagai grayscale 8-bit (1024 byte = 32x32)
    for (uint16_t py = 0; py < scuba_cat.height; py++) {
      for (uint16_t px = 0; px < scuba_cat.width; px++) {

        // Ambil nilai pixel dari PROGMEM (flash memory)
        uint8_t pixelVal = pgm_read_byte(
          &scuba_cat.frames[f][py * scuba_cat.width + px]
        );

        // Threshold: pixel terang → nyalakan dot OLED
        if (pixelVal > 127) {
          display.drawPixel(
            offsetX + px,
            offsetY + py,
            SSD1306_WHITE
          );
        }
      }
    }

    display.display();

    // Gunakan delay dari array delays di PROGMEM
    uint16_t frameDelay = pgm_read_word(&scuba_cat.delays[f]);
    delay(frameDelay);
  }
}

// =====================================================
// CAMERA INIT PREVIEW MODE
// =====================================================
void initCameraPreview() {

  esp_camera_deinit();

  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk = XCLK_GPIO_NUM;

  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;

  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;

  // PREVIEW OLED MODE
  config.pixel_format = PIXFORMAT_GRAYSCALE;

  config.frame_size = FRAMESIZE_96X96;

  config.fb_count = 1;

  config.fb_location = CAMERA_FB_IN_PSRAM;

  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK) {

    Serial.println("PREVIEW INIT FAIL");
    return;
  }

  Serial.println("PREVIEW MODE");
}

// =====================================================
// CAMERA INIT PHOTO MODE
// =====================================================
void initCameraPhoto() {

  esp_camera_deinit();

  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk = XCLK_GPIO_NUM;

  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;

  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;

  // JPEG PHOTO MODE
  config.pixel_format = PIXFORMAT_JPEG;

  // LEBIH STABIL DARI UXGA
  config.frame_size = FRAMESIZE_SVGA;

  config.jpeg_quality = 12;

  config.fb_count = 2;

  config.fb_location = CAMERA_FB_IN_PSRAM;

  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK) {

    Serial.println("PHOTO INIT FAIL");
    return;
  }

  Serial.println("PHOTO MODE");
}

// =====================================================
// SAVE PHOTO
// =====================================================
void savePhoto() {

  // FREEZE SCREEN
  oledMessage("CAPTURING...");

  delay(1200);

  // STOP OLED
  stopOLED();

  // PHOTO MODE
  initCameraPhoto();

  delay(500);

  // INIT SD
  if (!SD_MMC.begin("/sdcard", true)) {

    initOLED();

    oledMessage("SD FAILED");

    delay(2000);

    initCameraPreview();

    return;
  }

  // TAKE PHOTO
  camera_fb_t * fb = esp_camera_fb_get();

  if (!fb) {

    SD_MMC.end();

    initOLED();

    oledMessage("CAPTURE FAIL");

    delay(2000);

    initCameraPreview();

    return;
  }

  EEPROM.begin(EEPROM_SIZE);

  pictureNumber = EEPROM.read(0) + 1;

  String path =
    "/photo_" +
    String(pictureNumber) +
    ".jpg";

  File file =
    SD_MMC.open(path.c_str(), FILE_WRITE);

  if (!file) {

    oledMessage("FILE ERROR");

  } else {

    file.write(fb->buf, fb->len);

    file.flush();

    file.close();

    EEPROM.write(0, pictureNumber);

    EEPROM.commit();

    Serial.println("PHOTO SAVED");
    Serial.println(path);
  }

  esp_camera_fb_return(fb);

  // RELEASE SD
  SD_MMC.end();

  // OLED ON
  initOLED();

  // SUCCESS MESSAGE
  display.clearDisplay();

  display.setTextSize(2);

  display.setCursor(15, 20);

  display.println("SAVED!");

  display.display();

  delay(2000);

  // BACK TO PREVIEW
  initCameraPreview();
}

// =====================================================
// SETUP
// =====================================================
void setup() {

  // DISABLE BROWNOUT DETECTOR
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);

  pinMode(SHUTTER_BUTTON, INPUT_PULLUP);

  pinMode(FLASH_BUTTON, INPUT_PULLUP);

  pinMode(FLASH_LED, OUTPUT);

  digitalWrite(FLASH_LED, LOW);

  EEPROM.begin(EEPROM_SIZE);

  // ===================================================
  // STARTUP SEQUENCE
  // ===================================================

  // 1. Inisialisasi OLED dulu
  initOLED();

  // 2. Tampilkan "SCUBA CAM" di tengah selama 2 detik
  showSplashScreen();

  // 3. Animasi loading spinner
  showLoadingSpinner();

  // 4. Tampilkan animasi GIF scuba cat
  showScubaCatGIF();

  // 5. Inisialisasi kamera preview
  initCameraPreview();

  // ===================================================
}

// =====================================================
// LOOP
// =====================================================
void loop() {

  // ===================================================
  // FLASH MANUAL TOGGLE
  // ===================================================

  bool currentFlashButton =
    digitalRead(FLASH_BUTTON);

  if (lastFlashButton == HIGH &&
      currentFlashButton == LOW) {

    flashState = !flashState;

    digitalWrite(FLASH_LED, flashState);

    delay(250);
  }

  lastFlashButton = currentFlashButton;

  // ===================================================
  // SHUTTER BUTTON
  // ===================================================

  if (digitalRead(SHUTTER_BUTTON) == LOW) {

    delay(50);

    if (digitalRead(SHUTTER_BUTTON) == LOW) {

      savePhoto();

      while(digitalRead(SHUTTER_BUTTON) == LOW);

      delay(500);
    }
  }

  // ===================================================
  // LIVE PREVIEW OLED
  // ===================================================

  camera_fb_t *fb = esp_camera_fb_get();

  if (!fb) return;

  display.clearDisplay();

  float x_ratio =
    (float)fb->width / SCREEN_WIDTH;

  float y_ratio =
    (float)fb->height / SCREEN_HEIGHT;

  for (int y = 0; y < SCREEN_HEIGHT; y++) {

    for (int x = 0; x < SCREEN_WIDTH; x++) {

      int px = (int)(x * x_ratio);

      int py = (int)(y * y_ratio);

      uint8_t pixel =
        fb->buf[py * fb->width + px];

      if (pixel > 127) {

        display.drawPixel(
          x,
          y,
          SSD1306_WHITE
        );
      }
    }
  }

  display.display();

  esp_camera_fb_return(fb);

  delay(10);
}
