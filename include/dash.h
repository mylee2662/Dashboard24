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

    enum Error
    {
        NO_ERROR,
        BMS_FAULT,
        IMD_FAULT
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
    void DrawError(Adafruit_RA8875 tft, std::string error_message, int startX, int startY, Error type);
    void DrawString(Adafruit_RA8875 tft, std::string message, int startX, int startY, int size, int16_t color, int16_t backgroundColor, Direction dir = LEFT_TO_RIGHT);
    void HandleBMSFaults(Adafruit_RA8875 tft, int startX, int startY);

private:
    TeensyCAN<2> p_can_bus{};
    TeensyCAN<1> g_can_bus{};
    VirtualTimerGroup timer_group{};

    // Coolant Signals
    CANSignal<float, 0, 16, CANTemplateConvertFloat(1), CANTemplateConvertFloat(-40), false> coolant_temp_signal;
    CANSignal<float, 16, 16, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> coolant_flow_signal;
    CANRXMessage<2> rx_coolant_state{g_can_bus, 0x420, coolant_temp_signal, coolant_flow_signal};

    // Wheel Speed Signals
    CANSignal<float, 16, 16, CANTemplateConvertFloat(0.01), CANTemplateConvertFloat(0), false> fl_wheel_speed_signal;
    CANSignal<float, 0, 16, CANTemplateConvertFloat(0.01), CANTemplateConvertFloat(0), false> fr_wheel_speed_signal;
    CANRXMessage<2> rx_wheel_speeds{g_can_bus, 0x411, fl_wheel_speed_signal, fr_wheel_speed_signal};

    // VCU Signals
    CANSignal<int, 0, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> drive_state_signal;
    CANRXMessage<1> rx_drive_state{p_can_bus, 0x000, drive_state_signal};

    // IMD Signals
    CANSignal<int16_t, 0, 16, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0)> imd_resistance_signal;
    CANSignal<bool, 16, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0)> imd_status_signal;
    CANRXMessage<2> rx_imd{g_can_bus, 0x300, imd_resistance_signal, imd_status_signal};

    // BMS Signals
    // Fault signals
    CANSignal<bool, 0, 1, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> bms_fault_summary_signal;
    CANSignal<bool, 1, 1, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> bms_fault_under_voltage_signal;
    CANSignal<bool, 2, 1, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> bms_fault_over_voltage_signal;
    CANSignal<bool, 3, 1, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> bms_fault_under_temperature_signal;
    CANSignal<bool, 4, 1, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> bms_fault_over_temperature_signal;
    CANSignal<bool, 5, 1, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> bms_fault_over_current_signal;
    CANSignal<bool, 6, 1, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> bms_fault_external_kill_signal;
    CANSignal<bool, 7, 1, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> bms_fault_open_wire_signal;
    CANRXMessage<8> rx_bms_faults{g_can_bus, 0x250, [this]() {
                                    RecordBMSFaults();
                                  },
                                  bms_fault_summary_signal, bms_fault_under_voltage_signal, bms_fault_over_voltage_signal, bms_fault_under_temperature_signal, bms_fault_over_temperature_signal, bms_fault_over_current_signal, bms_fault_external_kill_signal, bms_fault_open_wire_signal};

    float prev_wheel_speed = -1;
    float prev_fr_wheel_speed = -1;
    Error error = NO_ERROR;
    uint8_t bms_faults = 0;
    int16_t backgroundColor = NORTHWESTERN_PURPLE;
    std::map<std::string, BarData> bars;

    int CalcBarHeight(float value, float min, float max, int maxHeight);
    void RecordBMSFaults();
};