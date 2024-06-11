

#include <string>
#include "virtualTimer.h"
#include "teensy_can.h"
#include "Adafruit_GFX.h"
#include "Adafruit_RA8875.h"

//VirtualTimerGroup read_timer;

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
        void UpdateDisplay(Adafruit_RA8875 tft);
        
        void DrawBar(Adafruit_RA8875 tft, int startX, int *prev_bar_y, int width, float value, int min_value, int max_value, int *prev_bar_level);
        float WheelSpeedAvg(float fl_wheel_speed, float fr_wheel_speed);
        void DrawWheelSpeed(Adafruit_RA8875 tft, float wheel_speed, int startX, int startY);
        void DrawDriveState(Adafruit_RA8875 tft, int startX, int startY, int curr_drive_state, int squareSize);
        void DrawDigit(Adafruit_RA8875 tft, int digit, int startX, int startY, int squareSize);
    private:
        TeensyCAN<2> p_can_bus{};
        TeensyCAN<1> g_can_bus{};
        VirtualTimerGroup timer_group{};
        CANSignal<float, 0, 16, CANTemplateConvertFloat(0.01), CANTemplateConvertFloat(0), false> fl1_wheel_temp_signal;
        CANSignal<float, 16, 16, CANTemplateConvertFloat(0.01), CANTemplateConvertFloat(0), false> fl2_wheel_temp_signal;
        CANSignal<float, 32, 16, CANTemplateConvertFloat(0.01), CANTemplateConvertFloat(0), false> fr1_wheel_temp_signal;
        CANSignal<float, 48, 16, CANTemplateConvertFloat(0.01), CANTemplateConvertFloat(0), false> fr2_wheel_temp_signal;
        CANSignal<float, 0, 16, CANTemplateConvertFloat(1), CANTemplateConvertFloat(-40), false> coolant_temp_signal;
        CANSignal<float, 16, 16, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> coolant_flow_signal;
        //CANSignal<float, 64, 16, CANTemplateConvertFloat(0.01), CANTemplateConvertFloat(0), false> fl_wheel_speed_signal;
        CANSignal<float, 16, 16, CANTemplateConvertFloat(0.01), CANTemplateConvertFloat(0), false> fl_wheel_speed_signal;
        CANSignal<float, 0, 16, CANTemplateConvertFloat(0.01), CANTemplateConvertFloat(0), false> fr_wheel_speed_signal;
        CANSignal<int, 0, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> drive_state_signal;
        CANRXMessage<2> rx_wheel_speeds{g_can_bus, 0x411, fl_wheel_speed_signal, fr_wheel_speed_signal};
        //CANRXMessage<4> rx_wheel_temps{g_can_bus, 0x411, fl1_wheel_temp_signal, fl2_wheel_temp_signal, fr1_wheel_temp_signal, fr2_wheel_temp_signal};
        CANRXMessage<1> rx_drive_state{p_can_bus, 0x000, drive_state_signal};
        CANRXMessage<2> rx_coolant_state{g_can_bus, 0x420, coolant_temp_signal, coolant_flow_signal};
};