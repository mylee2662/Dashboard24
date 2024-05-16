#include "dash.h"

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

int drive_state_startX = 384;
int drive_state_startY = 80;
int wheel_speed_startX = 384;
int wheel_speed_startY = 216;

//float fl_wheel_speed;
//float fr_wheel_speed;
float motor_temp;
int drive_state = 2; //Just a temp value to test wheel board

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

    timer_group.AddTimer(10, [this](){ this->GetCAN(); });
}

float Dash::WheelSpeedAvg(float fl_wheel_speed, float fr_wheel_speed){
    return (fl_wheel_speed + fr_wheel_speed) / 2;
}

void Dash::UpdateDisplay(Adafruit_RA8875 tft)
{   
    float fl_wheel_speed = static_cast<float>(fl_wheel_speed_signal);
    float fr_wheel_speed = static_cast<float>(fr_wheel_speed_signal);

    Serial.println("FRONT LEFT");
    Serial.println(fl_wheel_speed);
    Serial.println("FRONT RIGHT");
    Serial.println(fr_wheel_speed);

    //float avg_wheel_speed = WheelSpeedAvg(fl_wheel_speed, fr_wheel_speed);

    //motor_temp = static_cast<float>(motor_temp_signal);
    //drive_state = static_cast<int>(drive_state_signal);

    //White out the screen
    tft.fillScreen(RA8875_WHITE);

    DrawDriveState(tft, drive_state_startX, drive_state_startY, 8);
    //DrawWheelSpeed(tft, avg_wheel_speed, wheel_speed_startX, wheel_speed_startY);
    DrawWheelSpeed(tft, fr_wheel_speed, wheel_speed_startX, wheel_speed_startY);

    timer_group.Tick(millis());
}

void Dash::DrawWheelSpeed(Adafruit_RA8875 tft, float wheel_speed, int startX, int startY){
    //Serial.println("Drawing Wheel Speed");
    
    int rounded_wheel_speed = round(wheel_speed);

    Serial.println(rounded_wheel_speed);
    // TODO

    //Making a naive assumption that 0 <= wheel speed < 100
     if(wheel_speed > 9){
        //Digits must be off center for double digit numbers
        startX += 32;
     }
     else if(wheel_speed == 0){
        DrawDigit(tft, 0, startX, startY, 8);
     }

    while(rounded_wheel_speed > 0){
        int digit = rounded_wheel_speed % 10;
        rounded_wheel_speed /= 10;

        DrawDigit(tft, digit, startX, startY, 8);
        startX -= 80;
    }
}

//Draws drive state on screen based on CAN signal
void Dash::DrawDriveState(Adafruit_RA8875 tft, int startX, int startY, int squareSize){
    switch(drive_state){
        case 0:
            //Off
            tft.fillRect(startX, startY + squareSize, squareSize, squareSize * 8, RA8875_BLACK);

            tft.fillRect(startX + squareSize, 80, squareSize * 4, squareSize, RA8875_BLACK);
            tft.fillRect(startX + squareSize, 152, squareSize * 4, squareSize, RA8875_BLACK);

            tft.fillRect(424, 88, squareSize, squareSize * 8, RA8875_BLACK);
            
            break;
        case 1:
            //Neutral
            // Draw the left vertical line of 'N'
            tft.fillRect(startX, startY, squareSize, squareSize * 9, RA8875_BLACK);
            // Draw the right vertical line of 'N'
            tft.fillRect(startX + 5 * squareSize, startY, squareSize, squareSize * 9, RA8875_BLACK);
            // Draw the staggered diagonal that goes down 2 squares for each step to the right
            for (int x = 1, y = 0; x < 6 && y < 9; x++, y += 2) {
                // Ensure the diagonal does not extend beyond the bottom of the grid
                if(y < 9) {
                    tft.fillRect(startX + x * squareSize, startY + y * squareSize, squareSize, squareSize, RA8875_BLACK);
                }
                if (y + 1 < 9) { // Check if there's room to move down one more row
                    tft.fillRect(startX + x * squareSize, startY + (y + 1) * squareSize, squareSize, squareSize, RA8875_BLACK);
                }
            }

            break;
        case 2:
            //Drive
            tft.fillRect(startX + 8, 80, squareSize, squareSize * 9, RA8875_BLACK);
            tft.fillRect(startX + 8, 80, squareSize * 4, squareSize, RA8875_BLACK);
            tft.fillRect(startX + 8, 152, squareSize * 4, squareSize, RA8875_BLACK);
            tft.fillRect(startX + 40, 88, squareSize, squareSize, RA8875_BLACK);
            tft.fillRect(startX + 40, 144, squareSize, squareSize, RA8875_BLACK);
            tft.fillRect(startX + 48, 96, squareSize, squareSize * 6, RA8875_BLACK);

            break;
        default:
            Serial.print("Unknown drive state");
    }
}

void Dash::DrawDigit(Adafruit_RA8875 tft, int digit, int startX, int startY, int squareSize){
    switch(digit){
        case 0:
            tft.fillRect(startX + 8, startY + 16, 8, 40, RA8875_BLACK);

            tft.fillRect(startX + 24, startY, 16, 8, RA8875_BLACK);
            tft.fillRect(startX + 24, startY + 64, 16, 8, RA8875_BLACK);

            tft.fillRect(startX + 16, startY + 8, 8, 8, RA8875_BLACK);
            tft.fillRect(startX + 40, startY + 8, 8, 8, RA8875_BLACK);

            tft.fillRect(startX + 16, startY + 56, 8, 8, RA8875_BLACK);
            tft.fillRect(startX + 40, startY + 56, 8, 8, RA8875_BLACK);

            tft.fillRect(startX + 48, startY + 16, 8, 40, RA8875_BLACK);

            break;
        case 1:
            tft.fillRect(startX + 12, startY + 16, 8, 8, RA8875_BLACK);
            tft.fillRect(startX + 20, startY + 8, 8, 8, RA8875_BLACK);
            tft.fillRect(startX + 28, startY, 8, 72, RA8875_BLACK);

            break;
        case 2:
            tft.fillRect(startX, startY + 16, 8, 8, RA8875_BLACK);
            tft.fillRect(startX + 8, startY + 8, 8, 8, RA8875_BLACK);
            tft.fillRect(startX + 16, startY, 32, 8, RA8875_BLACK);
            tft.fillRect(startX + 48, startY + 8, 8, 8, RA8875_BLACK);
            tft.fillRect(startX + 56, startY + 16, 8, 16, RA8875_BLACK);
            tft.fillRect(startX + 48, startY + 32, 8, 8, RA8875_BLACK);
            tft.fillRect(startX + 24, startY + 40, 24, 8, RA8875_BLACK);
            tft.fillRect(startX + 8, startY + 48, 16, 8, RA8875_BLACK);
            tft.fillRect(startX, startY + 56, 8, 8, RA8875_BLACK);
            tft.fillRect(startX, startY + 64, 64, 8, RA8875_BLACK);

            break;
        case 3:
            tft.fillRect(startX, startY + 8, 8, 8, RA8875_BLACK);
            tft.fillRect(startX + 8, startY, 48, 8, RA8875_BLACK);
            tft.fillRect(startX + 56, startY + 8, 8, 24, RA8875_BLACK);
            tft.fillRect(startX + 24, startY + 32, 32, 8, RA8875_BLACK);
            tft.fillRect(startX + 56, startY + 40, 8, 24, RA8875_BLACK);
            tft.fillRect(startX + 8, startY + 64, 48, 8, RA8875_BLACK);
            tft.fillRect(startX, startY + 56, 8, 8, RA8875_BLACK);

            break;
        case 4:
            tft.fillRect(startX, startY, 8, 40, RA8875_BLACK);
            tft.fillRect(startX + 40, startY, 8, 72, RA8875_BLACK);
            tft.fillRect(startX, startY + 32, 64, 8, RA8875_BLACK);

            break;
        case 5:
            tft.fillRect(startX, startY, 64, 8, RA8875_BLACK);
            tft.fillRect(startX, startY, 8, 24, RA8875_BLACK);
            tft.fillRect(startX, startY + 24, 48, 8, RA8875_BLACK);
            tft.fillRect(startX + 48, startY + 32, 8, 8, RA8875_BLACK);
            tft.fillRect(startX + 56, startY + 40, 8, 16, RA8875_BLACK);
            tft.fillRect(startX + 48, startY + 56, 8, 8, RA8875_BLACK);
            tft.fillRect(startX + 8, startY + 64, 40, 8, RA8875_BLACK);
            tft.fillRect(startX, startY + 56, 8, 8, RA8875_BLACK);

            break;
        case 6:
            tft.fillRect(startX + 56, startY + 16, 8, 8, RA8875_BLACK);
            tft.fillRect(startX + 48, startY + 8, 8, 8, RA8875_BLACK);
            tft.fillRect(startX + 16, startY, 32, 8, RA8875_BLACK);
            tft.fillRect(startX + 8, startY + 8, 8, 8, RA8875_BLACK);
            tft.fillRect(startX, startY + 16, 8, 48, RA8875_BLACK);
            tft.fillRect(startX + 8, startY + 64, 48, 8, RA8875_BLACK);
            tft.fillRect(startX + 56, startY + 48, 8, 16, RA8875_BLACK);
            tft.fillRect(startX + 48, startY + 40, 8, 8, RA8875_BLACK);
            tft.fillRect(startX, startY + 32, 48, 8, RA8875_BLACK);

            break;
        case 7:
            tft.fillRect(startX, startY, 64, 8, RA8875_BLACK);
            tft.fillRect(startX + 56, startY, 8, 24, RA8875_BLACK);
            tft.fillRect(startX + 48, startY + 24, 8, 8, RA8875_BLACK);
            tft.fillRect(startX + 40, startY + 32, 8, 8, RA8875_BLACK);
            tft.fillRect(startX + 24, startY + 40, 16, 8, RA8875_BLACK);
            tft.fillRect(startX + 16, startY + 48, 8, 24, RA8875_BLACK);

            break;
        case 8:
            tft.fillRect(startX + 8, startY, 48, 8, RA8875_BLACK);
            tft.fillRect(startX + 8, startY + 32, 48, 8, RA8875_BLACK);
            tft.fillRect(startX + 8, startY + 64, 48, 8, RA8875_BLACK);

            tft.fillRect(startX, startY + 8, 8, 24, RA8875_BLACK);
            tft.fillRect(startX, startY + 40, 8, 24, RA8875_BLACK);
            tft.fillRect(startX + 56, startY + 8, 8, 24, RA8875_BLACK);
            tft.fillRect(startX + 56, startY + 40, 8, 24, RA8875_BLACK);

            break;
        case 9:
            tft.fillRect(startX + 8, startY, 48, 8, RA8875_BLACK);
            tft.fillRect(startX, startY + 8, 8, 24, RA8875_BLACK);
            tft.fillRect(startX + 8, startY + 32, 56, 8, RA8875_BLACK);
            tft.fillRect(startX + 56, startY + 8, 8, 56, RA8875_BLACK);
            tft.fillRect(startX + 8, startY + 64, 48, 8, RA8875_BLACK);
            tft.fillRect(startX, startY + 56, 8, 8, RA8875_BLACK);

            break;
        default:
            Serial.print("INPUT ERROR");
            break;
    }
}