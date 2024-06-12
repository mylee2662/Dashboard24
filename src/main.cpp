#include <Arduino.h>

#include <Dash.h>
#include "Adafruit_GFX.h"
#include "Adafruit_RA8875.h"
#include <iostream>

// Library only supports hardware SPI at this time
// Connect SCLK to UNO Digital #13 (Hardware SPI clock)
// Connect MISO to UNO Digital #12 (Hardware SPI MISO)
// Connect MOSI to UNO Digital #11 (Hardware SPI MOSI)
#define RA8875_INT 3
#define RA8875_CS 10
#define RA8875_RESET 8

#define SP28_ANGLE 21

u_int8_t SP28_input;
float SP28_voltage;

Adafruit_RA8875 tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);
uint16_t tx, ty;

Dash dashboard;

void setup()
{
  Serial.begin(9600);

  pinMode(SP28_ANGLE, INPUT);

  // RA8875 Setup
  Serial.println("RA8875 start");

  /* Initialize the display using 'RA8875_480x80', 'RA8875_480x128', 'RA8875_480x272' or 'RA8875_800x480' */
  if (!tft.begin(RA8875_800x480))
  {
    Serial.println("RA8875 Not Found!");
    while (1)
      ;
  }

  Serial.println("Found RA8875");

  tft.displayOn(true);
  tft.GPIOX(true);                              // Enable TFT - display enable tied to GPIOX
  tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
  tft.PWM1out(255);
  // Initialize dashboard
  dashboard.Initialize();

  // With hardware accelleration this is instant
  dashboard.DrawBackground(tft);
}

void loop()
{
  // put your main code here, to run repeatedly:
  dashboard.GetCAN();
  dashboard.UpdateDisplay(tft);

  // Resistance: 2.6k
  // 180 degrees range of motion: divot side rotating over PF2C1

  // SP28_input = analogRead(SP28_ANGLE);
  // SP28_voltage = (float) SP28_input * 0.00322581f;
  // Serial.println(SP28_input);
  // Serial.println(SP28_voltage);
  // Serial.println("****************************************");
}