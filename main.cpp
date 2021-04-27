#include "NuerteyDHT11Device.h"

static constexpr uint32_t DHT11_DEVICE_STABLE_STATUS_DELAY(1000); // 1 second.
static constexpr uint32_t DHT11_DEVICE_SAMPLING_PERIOD(3000);     // 3 seconds.

// DHT11 Sensor Interfacing with ARM MBED. Data communication is single-line
// serial. Note that for STM32 Nucleo-144 boards, the ST Zio connectors 
// are designated by [CN7, CN8, CN9, CN10]. 
//
// From prior successful testing of DHT11 on Arduino Uno, and matching 
// up the specific pins on Arduino with the "Arduino Support" section of
// the STM32 Zio connectors, I isolated the following pin as the 
// Arduino-equivalent data pin:
//
// Connector: CN10 
// Pin      : 10 
// Pin Name : D3
// STM32 Pin: PE13
// Signal   : TIMER_A_PWM3
NuerteyDHT11Device<DHT11_t> g_DHT11(PE_13); // TBD Nuertey Odzeyem; Pin Name or STM32 Pin??? If fixed here, fix in original project too

int main()
{
    printf("\r\n\r\nDHT11-Mbed-Driver Application - Beginning... \r\n\r\n");

    // Per datasheet/device specifications:
    //
    // "When power is supplied to the sensor, do not send any instruction
    // to the sensor in within one second in order to pass the unstable
    // status phase."
    ThisThread::sleep_for(DHT11_DEVICE_STABLE_STATUS_DELAY);

    while (1)
    {
        auto result = g_DHT11.ReadData();
        if (!result)
        {
            // TBD: Nuertey Odzeyem; what does this do? Will message() crash?
            // Should the enum and message have a SUCCESS case?
            // Just for learning experience; eventually get rid of it.
            printf("\n[debug success result] %s :-> \"%s\" -> [%d]\n", 
               result.category().name(), result.message().c_str(), result.value());
               
            auto h = 0.0f, c = 0.0f, f = 0.0f, k = 0.0f, dp = 0.0f, dpf = 0.0f;

            c   = g_DHT11.GetTemperature(TemperatureScale_t::CELCIUS);
            f   = g_DHT11.GetTemperature(TemperatureScale_t::FARENHEIT);
            k   = g_DHT11.GetTemperature(TemperatureScale_t::KELVIN);
            h   = g_DHT11.GetHumidity();
            dp  = g_DHT11.CalculateDewPoint(c, h);
            dpf = g_DHT11.CalculateDewPointFast(c, h);

            printf("\nTemperature in Kelvin: %4.2fK, Celcius: %4.2f°C, Farenheit %4.2f°F\n", k, c, f);
            printf("Humidity is %4.2f, Dewpoint: %4.2f, Dewpoint fast: %4.2f\n", h, dp, dpf);
        }
        else
        {
            printf("Error! g_DHT11.ReadData() returned: [%d] -> %s\n", 
                  result.value(), result.message().c_str());
        }

        // Per datasheet/device specifications:
        //
        // "Sampling period：Secondary Greater than 2 seconds"
        ThisThread::sleep_for(DHT11_DEVICE_SAMPLING_PERIOD);
    }
}
