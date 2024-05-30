#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

#define WIFI_SSID "arduinoBike"
#define WIFI_PASS "passwork"

#ifdef UNO_BOARD
// Don't change the names of these #define's! they match the variant ones
#define SPIWIFI SPI
#define SPIWIFI_SS 10  // Chip select pin
#define SPIWIFI_ACK 7  // a.k.a BUSY or READY pin
#define ESP32_RESETN 5 // Reset pin
#define ESP32_GPIO0 -1 // Not connected

// DC Motor pins
#define ENA A5
#define IN1 11
#define IN2 12

// Servo Motor pins
#define LEFT_ARM 3
#define MID_ARM 5
#define RIGHT_ARM 9
#else

// DC Motor pins
#define ENA 14
#define IN1 12
#define IN2 13

// Servo Motor pins
#define LEFT_ARM 23
#define RIGHT_ARM 25
#define MID_ARM 26

#endif

#define BIKE_RUN_TIME 1 // In seconds

char webCSS[] = R"=====(
* {
font-family: sans-serif;
}

body {
padding: 1em;
font-size: 1em;
text-align: left;
}

.header {
display: flex;
align-items: center;
justify-content: space-between;
padding: 20px;
}

.custom-button {
padding: 8px 10px;
font-size: 30px;
background-color: #04AA6D;
color: white;
border: none;
border-radius: 15px;
cursor: pointer;
margin: 2px;
}

.custom-blue {
background-color: blue;
})=====";

enum
{
    SERVO_LEFT,
    SERVO_MID,
    SERVO_RIGHT
};

#endif
