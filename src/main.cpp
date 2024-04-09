#include <Arduino.h>

#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_RA8875.h"


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

void setup()
{
  Serial.begin(9600);

  pinMode(SP28_ANGLE, INPUT);

  //RA8875 Setup
  Serial.println("RA8875 start");

  /* Initialize the display using 'RA8875_480x80', 'RA8875_480x128', 'RA8875_480x272' or 'RA8875_800x480' */
  if (!tft.begin(RA8875_800x480)) {
    Serial.println("RA8875 Not Found!");
    while (1);
  }

  Serial.println("Found RA8875");

  tft.displayOn(true);
  tft.GPIOX(true);      // Enable TFT - display enable tied to GPIOX
  tft.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
  tft.PWM1out(255);

  // With hardware accelleration this is instant
  tft.fillScreen(RA8875_WHITE);

  /* Draw bars */
  // Origin is top left corner
  tft.fillRect(0, 200, 100, 280, RA8875_RED);
  tft.fillRect(100, 280, 100, 200, RA8875_BLUE);
  
  tft.fillRect(700, 220, 100, 260, RA8875_GREEN);
  tft.fillRect(600, 360, 100, 120, RA8875_MAGENTA);
  
  /* Draw telemetry status */
  tft.fillRect(750, 0, 50, 50, RA8875_GREEN);

  //Drive States: Drive, Reverse, 
  // Draw D
  tft.fillRect(384, 80, 8, 72, RA8875_BLACK);
  tft.fillRect(384, 80, 32, 8, RA8875_BLACK);
  tft.fillRect(384, 152, 32, 8, RA8875_BLACK);
  tft.fillRect(416, 88, 8, 8, RA8875_BLACK);
  tft.fillRect(416, 144, 8, 8, RA8875_BLACK);
  tft.fillRect(424, 96, 8, 48, RA8875_BLACK);

  //Draw O

  // Draw 0
  tft.fillRect(384, 96, 8, 48, RA8875_BLACK);

  tft.fillRect(400, 80, 16, 8, RA8875_BLACK);
  tft.fillRect(400, 152, 16, 8, RA8875_BLACK);

  tft.fillRect(392, 88, 8, 8, RA8875_BLACK);
  tft.fillRect(416, 88, 8, 8, RA8875_BLACK);

  tft.fillRect(392, 144, 8, 8, RA8875_BLACK);
  tft.fillRect(416, 144, 8, 8, RA8875_BLACK);

  tft.fillRect(424, 96, 8, 48, RA8875_BLACK);
  
  /* Switch to text mode */
  tft.textMode();


  /* Set a solid for + bg color ... */

  /* ... or a fore color plus a transparent background */

  /* Render some text! */
  char speed[15] = "70";

  /* Change the cursor location and color ... */
  tft.textTransparent(RA8875_RED);
  tft.textSetCursor(375, 180);
  tft.textEnlarge(3);
  //tft.textWrite(speed);

  char drive_state[2] = "D";
  tft.textSetCursor(395, 100);
  //tft.textWrite(drive_state);

  //Sets text color to BG color to "get rid off" cursor blink
  tft.textTransparent(RA8875_WHITE);
}

void loop() {
  // put your main code here, to run repeatedly:

  //dashboard.UpdateDisplay();

  //Resistance: 2.6k
  //180 degrees range of motion: divot side rotating over PF2C1

  // SP28_input = analogRead(SP28_ANGLE);
  // SP28_voltage = (float) SP28_input * 0.00322581f;
  // Serial.println(SP28_input);
  // Serial.println(SP28_voltage);
  // Serial.println("****************************************");
  // delay(100);
}