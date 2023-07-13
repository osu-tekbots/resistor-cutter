/*
 * Defines what each connected pin on the ESP32 is for 
 *
 * Nathaniel Baird
 * bairdn@oregonstate.edu
 *
 * Started:      07/11/2023
 * Last updated: 07/11/2023
 */

// Nokia 5110 LCD screen pins -- connect VCC to 3.3V; LIGHT and GND to GND
#define CLK_PIN 17
#define DIN_PIN 16
#define DC_PIN  4
#define CE_PIN  0
#define RST_PIN 2

// Smarkn analog joystick pins
#define SW_PIN  32
#define VRx_PIN 34
#define VRy_PIN 35
