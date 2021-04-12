#include <map>
#include <string>
#include "NuerteyDHT11Device.h"

static constexpr uint32_t DHT11_DEVICE_STABLE_STATUS_DELAY(1000); // 1 second.
static constexpr uint32_t DHT11_DEVICE_SAMPLING_PERIOD(3000);     // 3 seconds.

// DHT11 Sensor Interfacing with ARM MBED. Data communication is single-line
// serial. Note that for STM32 Nucleo-144 boards, the ST Zio connectors 
// are designated by [CN7, CN8, CN9, CN10].
//
// Connector: CN7 
// Pin      : 13 
// Pin Name : D22
// STM32 Pin: PB5
// Signal   : SPI_B_MOSI
NuerteyDHT11Device<DHT11_t> g_DHT11(PB_5); // TBD Nuertey Odzeyem; Pin Name or STM32 Pin??? If fixed here, fix in original project too

// =========================================================
// Free-Floating General Helper Functions To Be Used By All:
// =========================================================
using DHTStatusCodesMap_t = std::map<SensorStatus_t, std::string>;
using IndexElement_t = DHTStatusCodesMap_t::value_type;

inline static auto make_dht_error_codes_map()
{
    DHTStatusCodesMap_t eMap;
    
    eMap.insert(IndexElement_t(SensorStatus_t::SUCCESS, std::string("\"Communication success\"")));
    eMap.insert(IndexElement_t(SensorStatus_t::ERROR_BUS_BUSY, std::string("\"Communication failure - bus busy\"")));
    eMap.insert(IndexElement_t(SensorStatus_t::ERROR_NOT_DETECTED, std::string("\"Communication failure - sensor not detected on bus\"")));
    eMap.insert(IndexElement_t(SensorStatus_t::ERROR_ACK_TOO_LONG, std::string("\"Communication failure - ack too long\"")));
    eMap.insert(IndexElement_t(SensorStatus_t::ERROR_SYNC_TIMEOUT, std::string("\"Communication failure - sync timeout\"")));
    eMap.insert(IndexElement_t(SensorStatus_t::ERROR_DATA_TIMEOUT, std::string("\"Communication failure - data timeout\"")));
    eMap.insert(IndexElement_t(SensorStatus_t::ERROR_BAD_CHECKSUM, std::string("\"Checksum error\"")));
    eMap.insert(IndexElement_t(SensorStatus_t::ERROR_TOO_FAST_READS, std::string("\"Communication failure - too fast reads\"")));
 
    return eMap;
}

static DHTStatusCodesMap_t gs_DHTStatusCodesMap = make_dht_error_codes_map();

inline std::string ToString(const SensorStatus_t & key)
{
    return (gs_DHTStatusCodesMap.at(key));
}

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
        if (result == SensorStatus_t::SUCCESS)
        {
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
                  ToUnderlyingType(result), ToString(result).c_str());
        }

        // Per datasheet/device specifications:
        //
        // "Sampling period：Secondary Greater than 2 seconds"
        ThisThread::sleep_for(DHT11_DEVICE_SAMPLING_PERIOD);
    }
}
