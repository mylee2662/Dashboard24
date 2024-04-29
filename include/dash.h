#include <string>

#include "virtualTimer.h"
#include "teensy_can.h"
#include "Adafruit_GFX.h"
#include "Adafruit_RA8875.h"

VirtualTimerGroup read_timer;

class Dash
{
    public:
        char mode;
        uint16_t index;

        int error_banner;

        //MutableStringArray arr;

        //MutableStringArray *arr_ref = &arr;

        void GetCAN();
        void Initialize();
        void SetSteeringAngle(u_int16_t curr_steering_angle);
        void UpdateDisplay();
        void DrawDisplay(Adafruit_RA8875 tft, int startX, int startY);

        void DrawWheelSpeed(Adafruit_RA8875 tft, int startX, int startY);
        void DrawDriveState(Adafruit_RA8875 tft, int startX, int startY, int squareSize);
        void DrawChar(Adafruit_RA8875 tft, std::string input, int startX, int startY, int squareSize);
    private:
        TeensyCAN<1> p_can_bus{};
        TeensyCAN<2> g_can_bus{};
        VirtualTimerGroup timer_group{};
        CANSignal<float, 0, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(-40), false> wheel_speed_signal;
        CANSignal<float, 0, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(-40), false> motor_temp_signal;
        CANSignal<int, 0, 16, CANTemplateConvertFloat(0.1), CANTemplateConvertFloat(-40), false> drive_state_signal;

        CANRXMessage<1> rx_motor{p_can_bus, 0x420, motor_temp_signal};
        CANRXMessage<1> rx_wheel{g_can_bus, 0x420, wheel_speed_signal};
};