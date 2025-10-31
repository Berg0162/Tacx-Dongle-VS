# 🌀 Tacx-Dongle-VS

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)
[![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/Berg0162/Tacx-Dongle-VS?sort=semver)](https://github.com/Berg0162/Tacx-Dongle-VS/releases)
[![Platform: ESP32](https://img.shields.io/badge/platform-ESP32-orange)](https://www.espressif.com/en/products/socs/esp32)
[![GitHub issues](https://img.shields.io/github/issues/Berg0162/Tacx-Dongle-VS)](https://github.com/Berg0162/Tacx-Dongle-VS/issues)
[![GitHub Discussions](https://img.shields.io/github/discussions/Berg0162/Tacx-Dongle-VS)](https://github.com/Berg0162/Tacx-Dongle-VS/discussions)

**Virtual Shifting (VS) for older Tacx Smart trainers using the LilyGo T-Dongle-S3 and the [Tacx-Virtual-Shifting](https://github.com/Berg0162/Tacx-Virtual-Shifting) library.**

---

## 🏁 Overview
<img src="./media/LilyGo_T-Dongle-S3_Size.png" width="300" height="190" align= "left" alt="T-Dongle-S3 Size"> </br>

**Tacx-Dongle-VS** is a self-contained dongle that bridges **Zwift’s Virtual Shifting** system with **older Tacx Smart trainers** that never received Garmin’s 2025 firmware update.  
It combines a modern **ESP32-S3 microcontroller**, a **built-in ST7735 IPS display**, a **status LED** and when the project code is uploaded, it delivers **plug-and-ride compatibility**.
<br clear="left">

---

## 🚴 What is Virtual Shifting (VS)?
Virtual Shifting lets you *change gears digitally* while riding indoors and using the **Zwift Click device**.
Instead of mechanical chain movements, Zwift adjusts resistance and power targets in software, simulating gear ratios.  
Recent Tacx trainers implement this natively; older ones cannot — unless a small bridge device translates between Zwift’s BLE protocol and Tacx’s ANT+ FE-C control channel.

---

## 🔗 What is the Tacx-Virtual-Shifting library?
The [Tacx-Virtual-Shifting](https://github.com/Berg0162/Tacx-Virtual-Shifting) Arduino library:

- connects to **Zwift** via BLE (as a Smart Trainer peripheral)  
- connects to a **Tacx trainer** via ANT+ FE-C  
- interprets Zwift’s *virtual-gear*, *target-power*, or *gradient* messages  
- sends equivalent ANT+ commands to the older Tacx trainer  

The **Tacx-Dongle-VS** project wraps this library into a ready-to-use hardware package — the LilyGo T-Dongle-S3.

---

## 💡 Why the T-Dongle-S3?
- **ESP32-S3** – dual-core MCU with native BLE and plenty of flash/RAM  
- **Integrated display** – 0.96″ ST7735 IPS panel for connection & error feedback  
- **APA102 LED** – multicolor indicator for connection status  
- **USB-C** – single-cable power + firmware updates  

**These make the smallest and most affordable all-in-one bridge for Tacx VS.**

<img src="./media/LilyGo_T-Dongle-S3.png" width="761" height="241" ALIGN="left" alt="T-Dongle-S3">
<br clear="left">

For pricing (in the **EU**) see for example: [TinyTronics](https://www.tinytronics.nl/en/development-boards/microcontroller-boards/with-wi-fi/lilygo-t-dongle-s3-esp32-s3)

---

## 🔄 How it Works Together
<img src="./media/Schema_small.png" width="660" height="120" ALIGN="left" alt="Schema">
<br clear="left">
<br>
The dongle receives Zwift VS events via BLE, translates them using the Tacx-Virtual-Shifting library, and re-emits trainer-compatible ANT+ commands — effectively adding VS to older hardware.

---

## 🧩 Dependencies
| Library | Purpose | Source |
|----------|----------|---------|
| **Tacx-Virtual-Shifting** | BLE ↔ ANT+ bridge core | [GitHub Repo](https://github.com/Berg0162/Tacx-Virtual-Shifting) |
| **Arduino-GFX** | ST7735 display driver | Arduino Library Manager |
| **Adafruit_DotStar** | APA102 LED control | Arduino Library Manager |

---

## 🚀 Getting Started

1. **Clone** or download this repository.  
2. Unzip/copy the `Tacx-Dongle-Virtual-Shifting` folder to your Arduino **sketch folder**.  
3. Install **all required libraries** (see [Dependencies](#-dependencies)).  
4. Select **Board → ESP32S3 Dev Module**.  
5. Set all options in **`Tools → Menu`** as shown below:  

   ![T-Dongle-S3 Tools Settings](media/LilyGo_T-Dongle-S3_Tools.png)  

6. Connect the dongle via **USB-C**.  
7. Open the `Tacx-Dongle-Virtual-Shifting.ino` sketch and **upload**.  
8. On boot you’ll see:  
   - a **splash screen**,  
   - Tacx and Zwift connection rows with text + color cues,  
   - the **APA102 LED** mirroring the same connection states.  

<img src="./media/Screen_6.png" width="275" height="125" align= "left" alt="Screen #6">
When both devices show *Connected* (🟢 green), you’re ready to ride!
<br clear="left">
---

## 🧠 Color Feedback Legend
| Color | Meaning |
|:------|:---------|
| 🟡 Yellow | Scanning & Advertising |
| 🟢 Green | Connected |
| 🔴 Red | Connection Lost |
| 🔵 Led only | Scanning for Tacx |
| 🟠 Led only | Advertising for Zwift |

---

## 🖼️ Screenshots

<img src="./media/Screen_1.png" width="275" height="125" align= "left" alt="Screen #1">
<img src="./media/Screen_2.png" width="275" height="125" alt="Screen #2">
<br clear="left">

**Tacx-Dongle-VS** shows splash screen (during 3 seconds) and starts **scanning** (for Tacx trainer), device status Led is **turquoise blinking**!<br>

---

<img src="./media/Screen_3.png" width="275" height="125" align= "left" alt="Screen #3">
<img src="./media/Screen_4.png" width="275" height="125" alt="Screen #4">
<br clear="left">

**Tacx-Dongle-VS** shows (after 10 seconds) a scanning-timeout, device status Led is **red blinking**. Finally it keeps scanning (for Tacx trainer) and starts also **advertising** (for notifying Zwift app), device status Led is now **yellow blinking**!<br>

---

## 🧾 License
This project is licensed under the **GNU General Public License v3.0 (GPL-3.0)**.  
You may freely use, modify, and distribute this project, provided that any derivative work is also licensed under GPL-3.0.

---

## ✍️ Future Ideas
- OTA (Wi-Fi) firmware updates  
- OLED or e-paper variant for low-power builds  
- Optional web-UI configuration  

---

## ❤️ Contributing
This project is just starting!
If you’re interested in testing, coding, writing docs, or just giving feedback, contributions are welcome in [Discussions](https://github.com/Berg0162/Tacx-Dongle-VS/discussions).

---

## 🔧[Troubleshooting](docs/Troubleshooting.md)


