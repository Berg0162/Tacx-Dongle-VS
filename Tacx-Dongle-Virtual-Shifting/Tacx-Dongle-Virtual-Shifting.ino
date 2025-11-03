/***************************************************************************************************

              This is programming code for ESP32-S3 Espressif boards, requirements:
 
              Hardware: LilyGo T-Dongle-S3 (ESP32-S3) with 0.96" IPS ST7735S (80x160)
              Library:  Tacx-Virtual-Shifting
              Library:  Arduino_GFX_Library
              Library:  Adafruit_DotStar
              IDE:      Arduino IDE 2.x

                 This project builds on the work of other open-source projects.  
                 Many have invested time and resources providing open-source code!
 
                       GPL-3.0 license, check LICENSE for more information!

                      Full attribution and license details are included in:  
               https://github.com/Berg0162/Tacx-Virtual-Shifting/blob/main/NOTICE.txt

****************************************************************************************************

Select and upload this code first to your T-Dongle-S3 and follow the procedure:

 1. Start/Power-on the Tacx-Dongle and inspect the display and led info
 2. Start/Power-On the Tacx Smart Trainer  
 3. Your Tacx-Dongle and Trainer will pair
 4. Start Zwift-App on your computer or tablet and wait....
 5. Search on the Zwift pairing screens for the Tacx-Dongle a.k.a. <TACXS>
 6. Pair: Power, Controllable and Cadence one after another with <TACXS>
 7. Pair: Controls, your Zwift Click device and optionally others
 8. Pair: Heartrate (optionally)
 9. Select any Zwift ride you like
10. Hop on the bike: Zwift Click gear up/down and feel resistance change during the ride
 
This device is identified with the name <TACXS>. You will see this only when connecting to Zwift on 
the pairing screens! Notice: Zwift extends device names with additional numbers for identification!

****************************************************************************************************/

// --------------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress Serial Monitor messages (USB Mode: "Hardware CDC and JTAG")
// Uncomment general "#define CDC_JTAG" to activate
#define CDC_JTAG

// --------------------------------------------------------------------------------------------------
#include <TacxVirtualShifting.h>

#include "TacxDisplay.h"
TacxDisplay display;

#include "StatusLed.h"
StatusLed statusLed;

/*----------------------------------------------------------------------------------------------------
* When ZWIFT_SAFE_MODE is defined in the code, the firmware configures the T-Dongle-S3’s USB interface
* to use the "TinyUSB HID stack" instead of the default Arduino "Hardware CDC and JTAG" connection.
* This makes the dongle appear to a computer as a harmless HID device rather than a serial port.
* ⚠️ Use with caution: ZWIFT_SAFE_MODE has drawbacks! Check background information first!
* 💡 See: https://github.com/Berg0162/Tacx-Dongle-VS/blob/main/docs/Troubleshooting.md
* ---------------------------------------------------------------------------------------------------*/
// COMPILER DIRECTIVE: Uncomment "#define ZWIFT_SAFE_MODE" to activate
//#define ZWIFT_SAFE_MODE

#if defined(ZWIFT_SAFE_MODE) && defined(CDC_JTAG)
  // ⚠️ Warning: ZWIFT_SAFE_MODE and CDC_JTAG are both defined! Defaulting to CDC_JTAG mode.
  #undef ZWIFT_SAFE_MODE
#endif

#if defined(ZWIFT_SAFE_MODE) && !defined(CDC_JTAG)
  // Create alternative TinyUSB HID interface
  #include <USB.h>
  #include <USBHID.h>
  USBHID HID;
#endif

void setup() {

#if defined(CDC_JTAG) 
  Serial.setRxBufferSize(96); // Increase RX buffer size
  Serial.begin(115200);       
  while ( !Serial ) delay(10); 
  Serial.flush();
  delay(1000); // Give Serial I/O time to settle
  Serial.println("---- Tacx Dongle Virtual Shifting ----");
  Serial.printf("Virtual-Shifting Library Version %s\n", CODE_VERSION);
  delay(100);
#ifdef TACXNEO_FIRSTGENERATION
  Serial.println(" -> Tacx Neo First Generation modifications active!");
#endif
#elif defined(ZWIFT_SAFE_MODE)
  // Start alternative USB HID stack
  USB.begin();
  HID.begin();
#endif

  // Initialize Display, IPS ST7735S (80x160)
  display.begin();
  // Initialize builtin APA102 Led
  statusLed.begin();

  // Init NimBLEManager 
  BLEmanager->init();
  // Start scanning for a trainer
  BLEmanager->startScanning();
  // Set Display and Led states accordingly
  statusLed.setState(StatusLed::LED_SCANNING);
  display.setTacxState(ST_SCANNING);

  // Wait until the Peripheral/Trainer is successfuly connected or has a timeout
  const long TIMEOUT = millis() + 10000; // Within 10 seconds it should have found a Tacx trainer!
  while(!BLEmanager->clientIsConnected) {
    delay(100);
    if(millis() > TIMEOUT) {
      statusLed.setState(StatusLed::LED_ERROR);
      display.showError("Tacx Timeout!");
      statusLed.off();
      break;
    }
  }
  if (BLEmanager->clientIsConnected) {
    display.setTacxState(ST_CONNECTED);
  }
  // TacxS is connected with the Trainer, start advertising TacxS for Zwift connection!
  statusLed.setState(StatusLed::LED_ADVERTISING);
  display.setZwiftState(ST_ADVERTISING);
  BLEmanager->startAdvertising();
}

void matchDisplayAndLedStates() {
  // Keep track of previous device states
  static bool prevServer = false;
  static bool prevClient = false;
  static bool prevScanning = false;
  static bool prevAdvertising = false;

  // Poll BLEmanager for device states
  bool server = BLEmanager->serverIsConnected;
  bool client = BLEmanager->clientIsConnected;
  bool scanning = BLEmanager->isScanning();
  bool advertising = BLEmanager->isAdvertising();

  // Only update when something changed!
  if (server != prevServer || client != prevClient ||
      scanning != prevScanning || advertising != prevAdvertising) {

    // Match display with device state logic
    display.setZwiftState(server ? ST_CONNECTED : advertising ? ST_ADVERTISING : ST_LOST);
    display.setTacxState(client ? ST_CONNECTED : scanning ? ST_SCANNING : ST_LOST);

    // Match LED with device state logic
    if (server && client) {
      statusLed.setState(StatusLed::LED_CONNECTED);
    } else if (scanning && advertising) {
      statusLed.setState(StatusLed::LED_PAIRING);
    } else if (scanning) {
      statusLed.setState(StatusLed::LED_SCANNING);
    } else if (advertising) {
      statusLed.setState(StatusLed::LED_ADVERTISING);
    } else {
      statusLed.setState(StatusLed::LED_ERROR);
    }

    // Save current device states as previous
    prevServer = server;
    prevClient = client;
    prevScanning = scanning;
    prevAdvertising = advertising;
  }
}

void loop() { 
  matchDisplayAndLedStates();
  delay(200);
}
