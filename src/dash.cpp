#include "dash.h"

#include "teensy_can.h"
#include "virtualTimer.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_RA8875.h"

#define RA8875_WAIT 7
#define RA8875_CS 10
#define RA8875_RESET 8

u_int16_t steering_angle;

//Adafruit_RA8875 tft = Adafruit_RA8875(RA8875_CS, RA8875_RESET);

void Dash::GetCAN()
{
    p_can_bus.Tick();
    g_can_bus.Tick();
}

void Dash::Initialize()
{
    p_can_bus.Initialize(ICAN::BaudRate::kBaud1M);
    g_can_bus.Initialize(ICAN::BaudRate::kBaud1M);
    
    p_can_bus.RegisterRXMessage(rx_motor);
    g_can_bus.RegisterRXMessage(rx_wheel);

    timer_group.AddTimer(1, [this](){ this->GetCAN(); });
}

//v_o = v_1*(r_2/(r_1+r_2))
//v_o <= 3.3 V
void Dash::SetSteeringAngle(u_int16_t curr_steering_angle)
{
    steering_angle = curr_steering_angle;
}

void Dash::UpdateDisplay()
{   
    float wheel_speed = static_cast<float>(wheel_speed_signal);
    float motor_temp = static_cast<float>(motor_temp_signal);



    timer_group.Tick(millis());
}