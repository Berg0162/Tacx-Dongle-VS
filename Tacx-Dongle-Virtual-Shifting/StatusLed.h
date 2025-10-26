#pragma once
#include <Arduino.h>
#include <Adafruit_DotStar.h>

// LilyGo T-Dongle-S3 built-in APA102 pins
#define LED_DI_PIN 40
#define LED_CI_PIN 39

class StatusLed {
public:
  enum State {
    LED_OFF,
    LED_SCANNING,
    LED_ADVERTISING,
    LED_PAIRING,
    LED_CONNECTED,
    LED_ERROR
  };

  // Constructor
  StatusLed(const uint8_t dataPin = LED_DI_PIN, const uint8_t clockPin = LED_CI_PIN) : \
                                    led(1, dataPin, clockPin, DOTSTAR_BGR) {}

  // Initialize LED hardware
  void begin() {
    led.begin();
    setColor(COLOR_OFF); // No show
    off(); // Led State is off
  }

  // Set current state
  void setState(State newState) {
    if (currentState == newState) return;
    stopBlinkTask();  // stop any existing task
    currentState = newState;

    switch (newState) {
      case LED_PAIRING:
        startBlink(COLOR_YELLOW, 400, 0.3); // slow blink
        break;
      case LED_SCANNING:
        startBlink(COLOR_TURQUOISE, 400, 0.3); // slow blink
        break;
      case LED_ADVERTISING:
        startBlink(COLOR_ORANGE, 400, 0.3); // slow blink
        break;
      case LED_CONNECTED:
        setColor(COLOR_GREEN, 0.1); // steady dim
        break;
      case LED_ERROR:
        startBlink(COLOR_RED, 200, 0.5); // fast blink
        break;
      default:
        setColor(COLOR_OFF);
        break;
    }
  }

  // Set Led direct off
  void off() { setState(LED_OFF); }

private:
  // === Internal members ===
  Adafruit_DotStar led;
  TaskHandle_t blinkTaskHandle = nullptr;
  volatile bool blinkRunning = false;
  State currentState = LED_OFF;

  // === Colors ===
  static constexpr uint32_t COLOR_RED       = 0xFF0000; // Red
  static constexpr uint32_t COLOR_GREEN     = 0x00FF00; // Electric Green
  static constexpr uint32_t COLOR_YELLOW    = 0xFFDF00; // Golden Yellow
  static constexpr uint32_t COLOR_ORANGE    = 0xFC6719; // Zwift Orange
  static constexpr uint32_t COLOR_TURQUOISE = 0x00FFEF; // Tacx Turquoise
  static constexpr uint32_t COLOR_OFF       = 0x000000;

  struct BlinkConfig {
    uint32_t color;
    int delayMs;
    float brightness;
    StatusLed* self;
  };

  // === Helpers ===
  static void colorToRGB(uint32_t color, float brightness, uint8_t &r, uint8_t &g, uint8_t &b) {
    brightness = constrain(brightness, 0.0f, 1.0f);
    r = ((color >> 16) & 0xFF) * brightness;
    g = ((color >> 8) & 0xFF) * brightness;
    b = (color & 0xFF) * brightness;
  }

  void setColor(uint32_t color, float brightness = 1.0) {
    uint8_t r, g, b;
    colorToRGB(color, brightness, r, g, b);
    led.setPixelColor(0, r, g, b);
    led.show();
  }

  // === Blink task ===
  static void blinkTask(void *param) {
    BlinkConfig cfg = *(BlinkConfig*)param;
    free(param);
    StatusLed* self = cfg.self;

    while (self->blinkRunning) {
      self->setColor(cfg.color, cfg.brightness);
      vTaskDelay(pdMS_TO_TICKS(cfg.delayMs));
      self->setColor(COLOR_OFF);
      vTaskDelay(pdMS_TO_TICKS(cfg.delayMs));
    }

    // Mark task as done before deleting
    TaskHandle_t handle = self->blinkTaskHandle;
    self->blinkTaskHandle = nullptr;
    vTaskDelete(handle); // deletes current task safely
  }

  // === Task control ===
  void startBlink(uint32_t color, int delayMs, float brightness = 1.0) {

    stopBlinkTask(); // stop any previous task safely

    BlinkConfig *cfg = (BlinkConfig*)malloc(sizeof(BlinkConfig));
    if (!cfg) return;
    cfg->color = color;
    cfg->delayMs = delayMs;
    cfg->brightness = brightness;
    cfg->self = this;

    blinkRunning = true;
    xTaskCreatePinnedToCore(blinkTask, "StatusLedBlink", 2048, cfg, 1, &blinkTaskHandle, 1);
  }

  void stopBlinkTask() {
    if (!blinkRunning) return;
    blinkRunning = false;

    // Wait until the task sets handle to nullptr (meaning it deleted itself)
    for (int i = 0; i < 20 && blinkTaskHandle; i++) {
      vTaskDelay(pdMS_TO_TICKS(10));
    }

    // If still not deleted, forcefully kill it as a last resort
    if (blinkTaskHandle) {
      vTaskDelete(blinkTaskHandle);
      blinkTaskHandle = nullptr;
    }
    
    setColor(COLOR_OFF);
  }
};
