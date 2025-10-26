#pragma once
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "FreeSansBold9pt7b.h"

// Orientation: true = USB connector is pointing to the RIGHT (rotate 180°)
#define USB_TO_THE_RIGHT false

// ================== DISPLAY CONSTANTS ==================
#define DISP_W 160
#define DISP_H 80

// ========== HARDWARE PIN DEFINITIONS ==========
// Based on LilyGo T-Dongle-S3 documentation
#define PIN_TFT_CS   4
#define PIN_TFT_MOSI 3
#define PIN_TFT_SCLK 5
#define PIN_TFT_DC   2
#define PIN_TFT_RST  1
#define PIN_TFT_BL   38

// ================== COLORS ==================
#define CLR_BLACK     0x0000
#define CLR_WHITE     0xFFFF
#define CLR_TURQUOISE 0x07FD  // Tacx Turquoise blue
#define CLR_ORANGE    0xFCC9  // Zwift Orange
#define CLR_RED       0xF800  // Red
#define CLR_GREEN     0x07E0  // Electric green
#define CLR_YELLOW    0xFFF3  // Canary Yellow

// ================== STATE DEFINITIONS ==================
enum ConnState { ST_SCANNING, ST_CONNECTED, ST_LOST, ST_ADVERTISING };

struct RowState {
  ConnState state = ST_LOST; // Default
  uint16_t color = CLR_RED;  // Default
};

/* Convert RGB to RGB565
uint16_t rgb565( const unsigned long rgb)
{
  uint16_t R = (rgb >> 16) & 0xFF;
  uint16_t G = (rgb >>  8) & 0xFF;
  uint16_t B = (rgb      ) & 0xFF;

  uint16_t ret  = (R & 0xF8) << 8;  // 5 bits
           ret |= (G & 0xFC) << 3;  // 6 bits
           ret |= (B & 0xF8) >> 3;  // 5 bits
       
  return( ret);
}
*/

// ================== CLASS DEFINITION ==================
class TacxDisplay {
public:
  TacxDisplay();
  void begin();
  void showSplash();
  void showError(const String &txt);

  void setTacxState(ConnState s);
  void setZwiftState(ConnState s);

private:
  Arduino_DataBus *bus;
  Arduino_ST7735 *tft;

  RowState tacxRow, zwiftRow;
  ConnState tacxNewState, zwiftNewState;

  TaskHandle_t displayTaskHandle;

  void drawStatusRow(uint8_t rowIndex, const char *label,
                     uint16_t iconColor, const RowState &row);
  uint16_t getColorForState(ConnState s);
  const char* getTextForState(ConnState s);
  void showTacxStatus(ConnState state);
  void showZwiftStatus(ConnState state);
  static void displayTaskThunk(void *pvParameters);
  void displayTask(void *pvParameters);
};

// ================== IMPLEMENTATION ==================
TacxDisplay::TacxDisplay() {
  // Initialize SPI bus and display
  bus = new Arduino_ESP32SPI(PIN_TFT_DC, PIN_TFT_CS, PIN_TFT_SCLK, PIN_TFT_MOSI, -1, PIN_TFT_RST);
  // Corrected constructor for ST7735S 0.96" (T-Dongle-S3)
  tft = new Arduino_ST7735(bus, PIN_TFT_RST, 3, true, DISP_H, DISP_W, 26, 1, 26, 1);
}

void TacxDisplay::begin() {
  // Set backlight pin
  pinMode(PIN_TFT_BL, OUTPUT);
  tacxNewState = ST_LOST;
  zwiftNewState = ST_LOST;

  tft->begin();
  // Change orientation if USB connector points to the right
  if (USB_TO_THE_RIGHT)
    tft->setRotation(1); // rotate 180°
  // Basic text setup
  tft->fillScreen(CLR_BLACK);
  tft->setFont(&FreeSansBold9pt7b);

  // Enable backlight
  digitalWrite(PIN_TFT_BL, 0);
  // Start display task on core 1 (BLE typically runs on 0)
  xTaskCreatePinnedToCore(displayTaskThunk, "DisplayTask", 4096, this, 1, &displayTaskHandle, 1);
}

void TacxDisplay::showSplash() {
  tft->setTextColor(CLR_TURQUOISE, CLR_BLACK);
  tft->setCursor(1, 25);
  tft->print("Tacx Dongle-S3");
  tft->setTextColor(CLR_ORANGE, CLR_BLACK);
  tft->setCursor(6, 60);
  tft->print("Virtual Shifting");
  delay(3000); 
  tft->fillScreen(CLR_BLACK);
}

void TacxDisplay::showError(const String &txt) {
  tft->fillScreen(CLR_BLACK);
  tft->drawRect(0, 0, DISP_W, DISP_H, CLR_RED);
  tft->setTextColor(CLR_RED, CLR_BLACK);
  int16_t x = (DISP_W / 2) - (txt.length()+1) * 5;
  if (x < 3) x = 2;
  tft->setCursor(x, DISP_H / 2);
  tft->print(txt.c_str());
  delay(3000);
  // Restore Pre-Error Status Rows
  tft->fillScreen(CLR_BLACK);
  drawStatusRow(1, "Tacx", CLR_TURQUOISE, tacxRow);
  drawStatusRow(2, "Zwift", CLR_ORANGE, zwiftRow);
}

// Draws one row: label + icon + state text + colored status rect
void TacxDisplay::drawStatusRow(uint8_t rowIndex, const char *label,
                                uint16_t iconColor, const RowState &row) {
  int y = rowIndex == 1 ? 5 : 45;
  int iconX = 1;
  int textX = iconX + 49;
  int rectX = DISP_W - 8;

  // clear row background
  tft->fillRect(textX, y, DISP_W, 28, CLR_BLACK);

  // Draw icon
  tft->fillRoundRect(iconX, y, 46, 30, 4, iconColor);
  tft->setTextColor(CLR_BLACK);
  tft->setCursor(iconX + 1, y + 20);
  tft->println(label);

  // Draw state text
  tft->setTextColor(CLR_WHITE);
  tft->setCursor(textX, y + 20);
  tft->println(getTextForState(row.state));

  // Draw status rectangle (colored cue)
  tft->fillRect(rectX, y + 12, 8, 8, row.color);
  //tft->fillCircle(rectX + 3, y + 15, 4, row.color);
}

void TacxDisplay::showTacxStatus(ConnState state) {
  tacxRow.state = state;
  tacxRow.color = getColorForState(state);
  drawStatusRow(1, "Tacx", CLR_TURQUOISE, tacxRow);
}

void TacxDisplay::showZwiftStatus(ConnState state) {
  zwiftRow.state = state;
  zwiftRow.color = getColorForState(state);
  drawStatusRow(2, "Zwift", CLR_ORANGE, zwiftRow);
}

uint16_t TacxDisplay::getColorForState(ConnState s) {
  switch (s) {
    case ST_SCANNING:    return CLR_YELLOW;
    case ST_CONNECTED:   return CLR_GREEN;
    case ST_LOST:        return CLR_RED;
    case ST_ADVERTISING: return CLR_YELLOW;
    default:             return CLR_WHITE;
  }
}

const char* TacxDisplay::getTextForState(ConnState s) {
  switch (s) {                   
    case ST_SCANNING:    return "Scanning";
    case ST_CONNECTED:   return "Connected";
    case ST_LOST:        return "Lost!";
    case ST_ADVERTISING: return "Advertising";
    default:             return "";
  }
}

// ===== State setters =====
void TacxDisplay::setTacxState(ConnState s) {
  tacxNewState = s;
}
void TacxDisplay::setZwiftState(ConnState s) {
  zwiftNewState = s;
}

// ===== Task handling =====
void TacxDisplay::displayTaskThunk(void *pvParameters) {
  TacxDisplay *self = static_cast<TacxDisplay*>(pvParameters);
  self->displayTask(pvParameters);
}

void TacxDisplay::displayTask(void *pvParameters) {
  showSplash();

  for (;;) {
    if (tacxNewState != tacxRow.state) {
      showTacxStatus(tacxNewState);
    }

    if (zwiftNewState != zwiftRow.state) {
      showZwiftStatus(zwiftNewState);
    }

    vTaskDelay(pdMS_TO_TICKS(200)); // 200ms refresh interval
  }
}
