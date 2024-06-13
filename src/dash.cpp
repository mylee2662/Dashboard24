#include "dash.h"

#include <iostream>
#include <map>
#include <cmath>
#include <string>
#include "teensy_can.h"
#include "virtualTimer.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_RA8875.h"

#define RA8875_WAIT 7
#define RA8875_CS 10
#define RA8875_RESET 8
#define IMD_ERR_PIN 1000

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 480
#define CENTER SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2
#define CENTER_OFFSET(x, y) SCREEN_WIDTH / 2 + x, SCREEN_HEIGHT / 2 + y
#define BAND_HEIGHT 50
#define BAR_HEIGHT SCREEN_HEIGHT - BAND_HEIGHT * 2
#define BAR_WIDTH 50
#define BAR_SPACING 15

int drive_state_startX = SCREEN_WIDTH / 2;
int drive_state_startY = SCREEN_HEIGHT / 2 - 160;
int wheel_speed_startX = SCREEN_WIDTH / 2;
int wheel_speed_startY = SCREEN_HEIGHT / 2;
int bar_max_size = 480;

#define DEBUG

float motor_temp;
int drive_state = -1;
bool drive_state_drawn = false;

void Dash::GetCAN()
{
    p_can_bus.Tick();
    g_can_bus.Tick();
}

void Dash::Initialize()
{
    Serial.println("Initializing Dashboard");

    p_can_bus.Initialize(ICAN::BaudRate::kBaud1M);
    g_can_bus.Initialize(ICAN::BaudRate::kBaud1M);

    p_can_bus.RegisterRXMessage(rx_drive_state);
    g_can_bus.RegisterRXMessage(rx_wheel_speeds);

    timer_group.AddTimer(10, [this]()
                         { this->GetCAN(); });

    pinMode(IMD_ERR_PIN, OUTPUT);
    // ake the pin low
    digitalWrite(IMD_ERR_PIN, LOW);

    // create bars
    this->bars["coolant_temp"] = BarData("co", 0, 100, 0, SCREEN_HEIGHT - BAND_HEIGHT, BAR_WIDTH, BAR_HEIGHT);
    this->bars["inverter_temp"] = BarData("iv", 0, 100, BAR_WIDTH + BAR_SPACING, SCREEN_HEIGHT - BAND_HEIGHT, BAR_WIDTH, BAR_HEIGHT);
    this->bars["motor_temp"] = BarData("mo", 0, 100, 2 * BAR_WIDTH + 2 * BAR_SPACING, SCREEN_HEIGHT - BAND_HEIGHT, BAR_WIDTH, BAR_HEIGHT);  

    this->bars["pack_voltage"] = BarData("pv", 0, 100, SCREEN_WIDTH - BAR_WIDTH, SCREEN_HEIGHT - BAND_HEIGHT, BAR_WIDTH, BAR_HEIGHT);
    this->bars["min_voltage"] = BarData("nv", 0, 100, SCREEN_WIDTH - 2 * BAR_WIDTH - BAR_SPACING, SCREEN_HEIGHT - BAND_HEIGHT, BAR_WIDTH, BAR_HEIGHT);
    this->bars["max_cell_temp"] = BarData("mt", 0, 100, SCREEN_WIDTH - 3 * BAR_WIDTH - 2 * BAR_SPACING, SCREEN_HEIGHT - BAND_HEIGHT, BAR_WIDTH, BAR_HEIGHT);

}

void Dash::DrawBackground(Adafruit_RA8875 tft, int16_t color)
{
    this->backgroundColor = color;
    // black out the screen
    tft.fillScreen(color);

    int border = 20;
    int rect_height = 200;
    int rect_border_height = rect_height + 2 * border;
    // draw outlines
    tft.fillRect(SCREEN_WIDTH / 3.75, 0, SCREEN_HEIGHT * 0.75, SCREEN_HEIGHT, RA8875_BLACK);
    tft.fillCircle(CENTER, SCREEN_HEIGHT / 2.2, RA8875_BLACK);

    // draw the error band
    tft.fillRect(0, 0, SCREEN_WIDTH, BAND_HEIGHT, RA8875_BLACK);
    // draw the bottom band
    tft.fillRect(0, SCREEN_HEIGHT - BAND_HEIGHT, SCREEN_WIDTH, BAND_HEIGHT, RA8875_BLACK);
    // fill in main circle white
    tft.fillCircle(CENTER, SCREEN_HEIGHT / 2.2 - border, RA8875_WHITE);

    // write text beneath the bars
    // iterate
    for (auto &bar : this->bars)
    {
        BarData &data = bar.second;
        DrawString(tft, data.displayName, data.x, data.y + 10, 4, RA8875_WHITE, RA8875_BLACK);
    }

    if (prev_dected_error)
    {
        return;
    }

    // draw info in top left cornder
    DrawString(tft, "Temperatures", 8, 2, 5, RA8875_WHITE, RA8875_BLACK);
    // draw info on the top right
    DrawString(tft, "Battery", SCREEN_WIDTH - 8 * 28, 2, 5, RA8875_WHITE, RA8875_BLACK);
}

float Dash::WheelSpeedAvg(float fl_wheel_speed, float fr_wheel_speed)
{
    return (fl_wheel_speed + fr_wheel_speed) / 2;
}

void Dash::UpdateDisplay(Adafruit_RA8875 tft)
{
#ifndef DEBUG
    float fl_wheel_speed = static_cast<float>(fl_wheel_speed_signal);
    float fr_wheel_speed = static_cast<float>(fr_wheel_speed_signal);
    int curr_drive_state = static_cast<int>(drive_state_signal);
    int imd_status = static_cast<int>(imd_status_signal);
#else
    // we should change the drive state for testing
    // cycle based on time
    int fl_wheel_speed = (millis() / 200) % 200;
    int fr_wheel_speed = (millis() / 200) % 200;
    int curr_drive_state = (millis() / 1000) % 3;
    int imd_status = millis() > 50000 ? -10 : 0;
#endif
    float avg_wheel_speed = fl_wheel_speed + fr_wheel_speed / 2;

    // motor_temp = static_cast<float>(motor_temp_signal);
    drive_state = static_cast<int>(drive_state_signal);

    DrawDriveState(tft, drive_state_startX, drive_state_startY, curr_drive_state, 8);
    if (this->prev_wheel_speed != avg_wheel_speed)
        DrawWheelSpeed(tft, avg_wheel_speed, wheel_speed_startX, wheel_speed_startY);
    this->prev_wheel_speed = avg_wheel_speed;
    // draw IMD status
    DrawIMDStatus(tft, 8, 2, imd_status, 32);

    // draw the test bar
    this->DrawBar(tft, "coolant_temp", (millis() / 100) % 100, RA8875_GREEN, this->backgroundColor);
    this->DrawBar(tft, "inverter_temp", (millis() / 20) % 100, RA8875_YELLOW, this->backgroundColor);
    this->DrawBar(tft, "motor_temp", (millis() / 10) % 100, RA8875_BLUE, this->backgroundColor);

    this->DrawBar(tft, "pack_voltage", (millis() / 100) % 100, RA8875_GREEN, this->backgroundColor);
    this->DrawBar(tft, "min_voltage", (millis() / 20) % 100, RA8875_YELLOW, this->backgroundColor);
    this->DrawBar(tft, "max_cell_temp", (millis() / 10) % 100, RA8875_BLUE, this->backgroundColor);

    timer_group.Tick(millis());
}

void Dash::DrawBar(Adafruit_RA8875 tft, std::string barName, float newValue, int16_t barColor, int16_t backgroundColor)
{
    // get the bar if any
    if (this->bars.find(barName) == this->bars.end())
    {
        Serial.println("Bar not found");
        return;
    }

    BarData &bar = this->bars[barName];

    // are the values the same?
    if (bar.value == newValue)
    {
        return;
    }

    // calculate the height of the bar
    int newHeight = CalcBarHeight(newValue, bar.min, bar.max, bar.maxHeight);
    int oldHeight = CalcBarHeight(bar.value, bar.min, bar.max, bar.maxHeight);

    if (newHeight == oldHeight)
    {
        return;
    }

    // draw the bar
    int diff = newHeight - oldHeight;
    bar.value = newValue;

    // std::cout << "Drawing bar " << barName << " with value " << newValue << " and height " << newHeight << std::endl;

    if (diff > 0)
    {
        // if the new height is greater than the old height, we need to fill in the difference
        // we will draw the bar to go upwards
        // the top left of the screen is 0,0
        tft.fillRect(bar.x, bar.y - newHeight, bar.width, diff, barColor);
    }
    else
    {
        // if the new height is less than the old height, we need to clear the difference
        // we will draw the bar to go downwards
        tft.fillRect(bar.x, bar.y - oldHeight, bar.width, -diff, backgroundColor);
    }

    // write the value at the bottom of the
}

void Dash::DrawWheelSpeed(Adafruit_RA8875 tft, float wheel_speed, int startX, int startY)
{
    // Serial.println("Drawing Wheel Speed");

    // fill in the space
    tft.fillRect(startX - 150, startY, 300, 128, RA8875_WHITE);

    int rounded_wheel_speed = round(wheel_speed);

    Serial.println(rounded_wheel_speed);

    int digit_spacing = 8;
    int char_width = 80;

    startX -= char_width / 2;

    // Making a naive assumption that 0 <= wheel speed < 100
    if (wheel_speed > 99)
    {
        startX += char_width;
    }
    else if (wheel_speed > 9)
    {
        // Digits must be off center for double digit numbers
        startX += char_width / 2;
    }

    // Draw the digits
    while (rounded_wheel_speed > 0)
    {
        int digit = rounded_wheel_speed % 10;
        tft.drawChar(startX, startY, digit + '0', RA8875_BLACK, RA8875_WHITE, 16);
        startX -= char_width + digit_spacing;
        rounded_wheel_speed /= 10;
    }
}

// Draws drive state on screen based on CAN signal
void Dash::DrawDriveState(Adafruit_RA8875 tft, int startX, int startY, int curr_drive_state, int squareSize)
{
    // if (curr_drive_state == drive_state)
    // {
    //     return;
    // }

    char state;
    switch (curr_drive_state)
    {
    case 0:
        state = '-';
        break;
    case 1:
        state = 'N';
        break;
    case 2:
        state = 'D';
        break;
    }

    tft.drawChar(startX - 32, startY, state, RA8875_BLACK, RA8875_WHITE, 16);
    drive_state = curr_drive_state;
}

void Dash::DrawIMDStatus(Adafruit_RA8875 tft, int startX, int startY, int imd_status, int squareSize)
{
    std::string status;
    switch (imd_status)
    {
    case -10:
        status = "IMD: Short Circuit";
        break;
    case -5:
        status = "IMD: Loading";
        break;
    case -25:
        status = "IMD: Connection Fault";
    case -20:
        status = "IMD: Device Error";
        break;
    default:
        return;
    }

    // pull the pin high
    digitalWrite(IMD_ERR_PIN, HIGH);
    DrawError(tft, status, startX, startY);
}

void Dash::DrawError(Adafruit_RA8875 tft, std::string error_message, int startX, int startY)
{
    if (prev_dected_error)
        return;

    prev_dected_error = true;
    this->DrawBackground(tft, RA8875_RED);
    DrawString(tft, error_message, startX, startY, 5, RA8875_WHITE, RA8875_BLACK);
}

void Dash::DrawString(Adafruit_RA8875 tft, std::string message, int startX, int startY, int size, int16_t color, int16_t backgroundColor, Direction dir)
{
    for (int i = 0; i < message.length(); i++)
    {
        switch (dir)
        {
        case LEFT_TO_RIGHT:
            tft.drawChar(startX + i * size * 6, startY, message[i], color, backgroundColor, size);
            break;
        case UP_TO_DOWN:
            tft.drawChar(startX, startY + i * size * 8, message[i], color, backgroundColor, size);
            break;
        default:
            break;
        }
    }
}

int Dash::CalcBarHeight(float value, float min, float max, int maxHeight)
{
    int lerp = (value - min) / (max - min) * maxHeight;
    // clamp the value between 0 and maxHeight
    return lerp > maxHeight ? maxHeight : lerp < 0 ? 0
                                                   : lerp;
}