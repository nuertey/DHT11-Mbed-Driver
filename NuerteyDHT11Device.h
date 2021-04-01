/***********************************************************************
* @file      NuerteyDHT11Device.h
*
*    DHT11/DHT22 temperature and humidity sensor driver targetted for  
*    ARM Mbed platform. 
* 
*    For ease of use, flexibility and readability of the code, the driver
*    is written in a modern C++ (C++17/C++20) templatized class/idiom. 
* 
*    As Bjarne Stroustrup is fond of saying, "C++ implementations obey 
*    the zero-overhead principle: What you don’t use, you don’t pay for."
*
* @brief   Drive the DHT11/DHT22 sensor module(s) in order to obtain 
*          temperature/humidity readings.
* 
* @note    The Sensor Peripheral (DHT11) Component's Details Are As 
*          Follows:
* 
*   1) The DHT11 sensor measures and provides humidity and temperature 
*      values serially over a single wire. Its characteristics are as 
*      follows:
*      
*      - It can measure relative humidity in percentages (20 to 90% RH) 
*        and temperature in degree Celsius in the range of 0 to 50°C.
* 
*      - It has 4 pins; one of which is used for data communication in 
*        serial form.
* 
*      - Pulses of different TON and TOFF are decoded as logic 1 or 
*        logic 0 or start pulse or end of frame.
* 
*      - Power Supply: 3 to 5V DC, 2.5mA max current use during 
*        conversion (while requesting data).
* 
*      - Operating range: Good for 20-80% humidity readings with 5% 
*        accuracy.
* 
*      - Good for 0-50°C temperature readings ±2°C accuracy.
* 
*      - No more than 1 Hz sampling rate (once every second).
* 
*      - Body size: 15.5mm x 12mm x 5.5mm. 
*
* @warning    These warnings are key to successful sensor operation: 
* 
*   [1] When the connecting cable to the data pin is shorter than 20 
*       metres, a 5K pull-up resistor is recommended;
* 
*   [2] When the connecting cable to the data pin is longer than 20 
*       metres, choose an appropriate pull-up resistor as needed.
* 
*   [3] When power is supplied to the sensor, do not send any instructions
*       to the sensor in within one second in order to pass the unstable
*       status phase. 
*
* @author    Nuertey Odzeyem
* 
* @date      April 1, 2021
*
* @copyright Copyright (c) 2021 Nuertey Odzeyem. All Rights Reserved.
***********************************************************************/
#pragma once

#include <cstdint>
#include <array>
#include <time.h> 
#include "mbed.h"

#define PIN_HIGH  1
#define PIN_LOW   0

enum class SensorStatus_t : uint8_t
{
    SUCCESS = 0,
    ERROR_BUS_BUSY,
    ERROR_NOT_DETECTED,
    ERROR_BAD_START,
    ERROR_SYNC_TIMEOUT,
    ERROR_DATA_TIMEOUT,
    ERROR_BAD_CHECKSUM,
    ERROR_OVERLY_FAST_READS
};

enum class TemperatureScale_t : uint8_t
{
    CELCIUS = 0,
    FARENHEIT,
    KELVIN
};

template <typename T, typename U>
struct TrueTypesEquivalent : std::is_same<typename std::decay<T>::type, U>::type
{};

// Types to distinguish each sensor module type:
struct DHT11_t {};
struct DHT22_t {};

template <typename T>
class NuerteyDHT11Device
{
    static_assert(TrueTypesEquivalent<T, DHT11_t>::value
               || TrueTypesEquivalent<T, DHT22_t>::value,
    "Hey! NuerteyDHT11Device in its current form is only designed with DHT11, or DHT22 sensors in mind!!");

public:
    static constexpr uint8_t DHT11_MICROCONTROLLER_RESOLUTION_BITS = 8;
    static constexpr uint8_t SINGLE_BUS_DATA_FRAME_SIZE_BYTES      = 5;
    static constexpr uint8_t MAXIMUM_DATA_FRAME_SIZE_BITS          = 40; // 5x8
    static constexpr double  MINIMUM_SAMPLING_PERIOD_SECONDS       = 2;

    using DataFrame_t = std::array<uint8_t, SINGLE_BUS_DATA_FRAME_SIZE_BYTES>;

    NuerteyDHT11Device(PinName thePinName);

    NuerteyDHT11Device(const NuerteyDHT11Device&) = delete;
    NuerteyDHT11Device& operator=(const NuerteyDHT11Device&) = delete;
    // Note that as the copy constructor and assignment operators above 
    // have been designated 'deleted', automagically, the move constructor
    // and move assignment operators would likewise be omitted for us by
    // the compiler, as indeed, we do intend. For after all, this driver  
    // class is not intended to be copied or moved as it has ownership 
    // of a unique hardware pin. Indeed, the Compiler is our friend. We 
    // simply have to play by its stringent rules. Simple!

    virtual ~NuerteyDHT11Device();

    [[nodiscard]] SensorStatus_t ReadData();

    float GetHumidity() const;
    float GetTemperature(const TemperatureScale_t & Scale) const;
    float CalculateDewPoint(const float & celsius, const float & humidity) const;
    float CalculateDewPointFast(const float & celsius, const float & humidity) const;

protected:

private:
    [[nodiscard]] SensorStatus_t ValidateChecksum() const;

    float CalculateTemperature() const;
    float CalculateHumidity() const;
    float ConvertCelciusToFarenheit(const float & celcius);
    float ConvertCelciusToKelvin(const float & celcius);

    PinName              m_TheDataPinName;
    DataFrame_t          m_TheDataFrame;
    time_t               m_TheLastReadTime;
    SensorStatus_t       m_TheLastReadResult;
    float                m_TheLastTemperature;
    float                m_TheLastHumidity;
};

template <typename T>
NuerteyDHT11Device<T>::NuerteyDHT11Device(PinName thePinName)
    : m_TheDataPinName(thePinName)
{
    // Typically for POSIX, the following is enough:
    // Using this value ensures that time(NULL) - m_TheLastReadTime will
    // be >= MINIMUM_SAMPLING_PERIOD_SECONDS right away. Note that this 
    // assignment wraps around, but so will the subtraction.
    m_TheLastReadTime = time(NULL) - MINIMUM_SAMPLING_PERIOD_SECONDS; 
}

template <typename T>
NuerteyDHT11Device<T>::~NuerteyDHT11Device()
{
}

template <typename T>
SensorStatus_t NuerteyDHT11Device<T>::ReadData()
{
    SensorStatus_t result = SensorStatus_t::SUCCESS;

    // Check if sensor was read less than two seconds ago and return 
    // early to use last reading.
    currentTime = time(NULL);

    if (difftime(currentTime, m_TheLastReadTime) < MINIMUM_SAMPLING_PERIOD_SECONDS)
    {
        return m_TheLastReadResult; // return last correct measurement
    }

    m_TheLastReadTime = currentTime;

    // Reset 40 bits of previously received data to zero.
    m_TheDataFrame.fill(0);

    // DHT11 uses a simplified single-wire bidirectional communication protocol.
    // It follows a Master/Slave paradigm [NUCLEO-F767ZI=Master, DHT11=Slave] 
    // with the MCU observing these states:
    //
    // WAITING, READING.
    DigitalInOut theDigitalInOutPin(m_TheDataPinName);

    // MCU Sends out Start Signal to DHT:
    //
    // "Data Single-bus free status is at high voltage level. When the 
    // communication between MCU and DHT11 begins, the programme of MCU 
    // will set Data Single-bus voltage level from high to low."
    // 
    // https://www.mouser.com/datasheet/2/758/DHT11-Technical-Data-Sheet-Translated-Version-1143054.pdf
    theDigitalInOutPin.mode(PullUp);
    theDigitalInOutPin.output();
    theDigitalInOutPin = PIN_LOW;


    if constexpr (std::is_same<T, DHT11_t>::value)
    {
        ComposeAmazonMQTTConnectData<DHT11_t>();
    }
    else if constexpr (std::is_same<T, DHT22_t>::value)
    {
        ComposeGoogleMQTTConnectData<DHT22_t>();
    }

    
    return result;
}

template <typename T>
SensorStatus_t NuerteyDHT11Device<T>::ValidateChecksum()
{
    SensorStatus_t result = SensorStatus_t::ERROR_BAD_CHECKSUM;
    
    // Per the sensor device specs./data sheet:
    if (m_TheDataFrame[4] == ((m_TheDataFrame[0] + m_TheDataFrame[1] + m_TheDataFrame[2] + m_TheDataFrame[3]) & 0xFF))
    {
        result = SensorStatus_t::SUCCESS;
    }
    
    return result;
}


