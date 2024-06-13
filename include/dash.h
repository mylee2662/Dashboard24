#include <map>
#include <string>
#include "virtualTimer.h"
#include "teensy_can.h"
#include "Adafruit_GFX.h"
#include "Adafruit_RA8875.h"

#define rawRGB24toRGB565(r, g, b) uint16_t((r / 8) << 11) | ((g / 4) << 5) | (b / 8)
#define NORTHWESTERN_PURPLE rawRGB24toRGB565(78, 42, 132)

// VirtualTimerGroup read_timer;

class Dash
{
public:
    struct BarData
    {
        std::string displayName;
        float min;
        float max;
        float value;
        int x;
        int y;
        int width;
        int maxHeight;

        BarData() : displayName(""), min(0), max(0), value(0), x(0), y(0), width(20), maxHeight(100) {}

        BarData(std::string displayName, float min, float max, int startX, int startY, int width = 20, int maxHeight = 100)
            : displayName(displayName), min(min), max(max), value(min), x(startX), y(startY), width(width), maxHeight(maxHeight) {}
    };

    enum Direction
    {
        LEFT_TO_RIGHT,
        UP_TO_DOWN
    };

    char mode;
    uint16_t index;
    int error_banner;

    // MutableStringArray arr;

    // MutableStringArray *arr_ref = &arr;

    void GetCAN();
    void Initialize();
    void UpdateDisplay(Adafruit_RA8875 tft);

    void DrawBackground(Adafruit_RA8875 tft, int16_t color = NORTHWESTERN_PURPLE);
        void DrawBar(Adafruit_RA8875 tft, std::string barName, float newValue, int16_t barColor, int16_t backgroundColor);
        float WheelSpeedAvg(float fl_wheel_speed, float fr_wheel_speed);
        void DrawWheelSpeed(Adafruit_RA8875 tft, float wheel_speed, int startX, int startY);
        void DrawDriveState(Adafruit_RA8875 tft, int startX, int startY, int curr_drive_state, int squareSize);
        void DrawIMDStatus(Adafruit_RA8875 tft, int startX, int startY, int imd_status, int squareSize);
        void DrawError(Adafruit_RA8875 tft, std::string error_message, int startX, int startY);
        void DrawString(Adafruit_RA8875 tft, std::string message, int startX, int startY, int size, int16_t color, int16_t backgroundColor, Direction dir = LEFT_TO_RIGHT);
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

        // IMD Signals
        CANSignal<int16_t, 0, 16, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0)> imd_resistance_signal;
        CANSignal<bool, 16, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0)> imd_status_signal;
        CANRXMessage<2> rx_imd{g_can_bus, 0x300, imd_resistance_signal, imd_status_signal};

        float prev_wheel_speed = -1;
        float prev_fr_wheel_speed = -1;
        bool prev_dected_error = false;
        int16_t backgroundColor = NORTHWESTERN_PURPLE;
        std::map<std::string, BarData> bars;

        int CalcBarHeight(float value, float min, float max, int maxHeight);
};