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
* @note    The sensor peripheral (DHT11) component's details are as 
*          follows:
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
    ERROR_TOO_FAST_READS
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
    static constexpr uint8_t DHT11_MICROCONTROLLER_RESOLUTION_BITS  = 8;
    static constexpr uint8_t SINGLE_BUS_DATA_FRAME_SIZE_BYTES       = 5;
    static constexpr uint8_t MAXIMUM_DATA_FRAME_SIZE_BITS           = 40; // 5x8
    static constexpr double  MINIMUM_SAMPLING_PERIOD_SECONDS        = 2;
    static constexpr float   ZERO_DEGREES_CELCIUS_EQUIVALENT_KELVIN = 273.15; // Freezing point of water.

    using DataFrameBytes_t = std::array<uint8_t, SINGLE_BUS_DATA_FRAME_SIZE_BYTES>;
    using DataFrameBits_t  = std::array<uint8_t, MAXIMUM_DATA_FRAME_SIZE_BITS>;

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
    [[nodiscard]] SensorStatus_t ExpectPulse(DigitalInOut & theIO, const int & level, const int & max_time);
    [[nodiscard]] SensorStatus_t ValidateChecksum() const;

    float CalculateTemperature() const;
    float CalculateHumidity() const;
    float ConvertCelciusToFarenheit(const float & celcius);
    float ConvertCelciusToKelvin(const float & celcius);

    PinName              m_TheDataPinName;
    DataFrameBytes_t     m_TheDataFrame;
    time_t               m_TheLastReadTime;
    SensorStatus_t       m_TheLastReadResult;
    float                m_TheLastTemperature;
    float                m_TheLastHumidity;
};

template <typename T>
NuerteyDHT11Device<T>::NuerteyDHT11Device(PinName thePinName)
    : m_TheDataPinName(thePinName)
{
    // Using this value ensures that time(NULL) - m_TheLastReadTime will
    // be >= MINIMUM_SAMPLING_PERIOD_SECONDS the first time. Note that  
    // this assignment does wrap around, but so will the subtraction.

    // Typically for POSIX, the following formulation is enough since
    // time_t is denoted in seconds:
    m_TheLastReadTime = time(NULL) - MINIMUM_SAMPLING_PERIOD_SECONDS; 
}

template <typename T>
NuerteyDHT11Device<T>::~NuerteyDHT11Device()
{
}

template <typename T>
SensorStatus_t NuerteyDHT11Device<T>::ReadData()
{
    auto result = SensorStatus_t::SUCCESS;

    // Check if sensor was read less than two seconds ago and return 
    // early to use last reading.
    auto currentTime = time(NULL);

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
    
    // Just to allow things to stabilize:
    ThisThread::sleep_for(1);
    
    theDigitalInOutPin.output();
    theDigitalInOutPin = PIN_LOW;

    // As an alternative to SFINAE template techniques:
    if constexpr (std::is_same<T, DHT11_t>::value)
    {
        // "...and this process must take at least 18ms to ensure DHT’s 
        // detection of MCU's signal", so err on the side of caution.
        ThisThread::sleep_for(20);
    }
    else if constexpr (std::is_same<T, DHT22_t>::value)
    {
        // The data sheet specifies, "at least 1ms", so err on the side 
        // of caution by doubling the amount. Per Mbed docs, spinning
        // with wait_us() on milliseconds here is not recommended as it
        // would affect multi-threaded performance.
        ThisThread::sleep_for(2);
    }

    uint8_t i = 0, j = 0, b = 0;
    DataFrameBits_t bitValue = {}; // Initialize to zeros.

    // "...then MCU will pull up voltage and wait 20-40us for DHT’s response."
    theDigitalInOutPin.mode(PullUp);

    // End the start signal by setting data line high for 30 microseconds.
    theDigitalInOutPin = PIN_HIGH;
    wait_us(30);
    theDigitalInOutPin.input();

    // Wait till the sensor grabs the bus.
    if (SensorStatus_t::SUCCESS != ExpectPulse(theDigitalInOutPin, 1, 40))
    {
        result = SensorStatus_t::ERROR_NOT_DETECTED;
        m_TheLastReadResult = result;
        return result;
    }

    // Sensor should signal low 80us and then hi 80us.
    if (SensorStatus_t::SUCCESS != ExpectPulse(theDigitalInOutPin, 0, 100))
    {
        result = SensorStatus_t::ERROR_SYNC_TIMEOUT;
        m_TheLastReadResult = result;
        return result;
    }

    if (SensorStatus_t::SUCCESS != ExpectPulse(theDigitalInOutPin, 1, 100))
    {
        result = SensorStatus_t::ERROR_TOO_FAST_READS;
        m_TheLastReadResult = result;
        return result;
    }
    else
    {
        // Timing critical code.
        {
            // TBD; Nuertey Odzeyem: We CANNOT use the CriticalSectionLock
            // here as ExpectPulse() calls wait_us(). As the Mbed docs 
            // further clarifies:
            //
            // "Note: You must not use time-consuming operations, standard 
            // library and RTOS functions inside critical section."
            //CriticalSectionLock  lock;

            // capture the data
            for (i = 0; i < SINGLE_BUS_DATA_FRAME_SIZE_BYTES; i++)
            {
                for (j = 0; j < DHT11_MICROCONTROLLER_RESOLUTION_BITS; j++)
                {
                    if (SensorStatus_t::SUCCESS != ExpectPulse(theDigitalInOutPin, 0, 75))
                    {
                        result = SensorStatus_t::ERROR_DATA_TIMEOUT;
                        m_TheLastReadResult = result;
                        return result;
                    }
                    // logic 0 is 28us max, 1 is 70us
                    wait_us(40);
                    bitValue[i*DHT11_MICROCONTROLLER_RESOLUTION_BITS + j] = theDigitalInOutPin;
                    if (SensorStatus_t::SUCCESS != ExpectPulse(theDigitalInOutPin, 1, 50))
                    {
                        result = SensorStatus_t::ERROR_DATA_TIMEOUT;
                        m_TheLastReadResult = result;
                        return result;
                    }
                }
            }
        } // End of timing critical code.

        // store the data
        for (i = 0; i < SINGLE_BUS_DATA_FRAME_SIZE_BYTES; i++)
        {
            b = 0;
            for (j = 0; j < DHT11_MICROCONTROLLER_RESOLUTION_BITS; j++)
            {
                if (bitValue[i*DHT11_MICROCONTROLLER_RESOLUTION_BITS + j] == 1)
                {
                    b |= (1 << (7-j));
                }
            }
            m_TheDataFrame[i] = b;
        }
        result = ValidateChecksum();
    }

    m_TheLastReadResult = result;
    
    return result;
}

template <typename T>
SensorStatus_t NuerteyDHT11Device<T>::ExpectPulse(DigitalInOut & theIO, const int & level, const int & max_time)
{
    auto result = SensorStatus_t::SUCCESS;
 
    // This method essentially spins in a loop (i.e. polls) for every 
    // microsecond until the expected pulse arrives or we timeout.   
    int count = 0;
    while (level == theIO)
    {
        if (count > max_time)
        {
            result = SensorStatus_t::ERROR_TOO_FAST_READS;
            break;
        }
        count++;
        wait_us(1);
    }
    
    return result;
}

template <typename T>
SensorStatus_t NuerteyDHT11Device<T>::ValidateChecksum()
{
    auto result = SensorStatus_t::ERROR_BAD_CHECKSUM;
    
    // Per the sensor device specs./data sheet:
    if (m_TheDataFrame[4] == ((m_TheDataFrame[0] + m_TheDataFrame[1] + m_TheDataFrame[2] + m_TheDataFrame[3]) & 0xFF))
    {
        m_TheLastTemperature = CalculateTemperature();
        m_TheLastHumidity = CalculateHumidity();
        result = SensorStatus_t::SUCCESS;
    }
    
    return result;
}

template <typename T>
float NuerteyDHT11Device<T>::CalculateTemperature() const
{}

template <typename T>
float NuerteyDHT11Device<T>::CalculateHumidity() const
{}

template <typename T>
float NuerteyDHT11Device<T>::ConvertCelciusToFarenheit(const float & celcius)
{
    return ((celsius * 9/5) + 32);
}

template <typename T>
float NuerteyDHT11Device<T>::ConvertCelciusToKelvin(const float & celcius)
{
    return (celsius + 273.15);
}

template <typename T>
float NuerteyDHT11Device<T>::GetHumidity() const
{
    return m_TheLastHumidity;
}

template <typename T>
float NuerteyDHT11Device<T>::GetTemperature(const TemperatureScale_t & Scale) const
{
    auto result = 0.0;

    if (Scale == TemperatureScale_t::FARENHEIT)
    {
        result = ConvertCelciusToFarenheit(m_TheLastTemperature);
    }
    else if (Scale == TemperatureScale_t::KELVIN)
    {
        result = ConvertCelciusToKelvin(m_TheLastTemperature);
    }
    else
    {
        result = m_TheLastTemperature;
    }

    return result;
}

template <typename T>
float NuerteyDHT11Device<T>::CalculateDewPoint(const float & celsius, const float & humidity) const
{}

template <typename T>
float NuerteyDHT11Device<T>::CalculateDewPointFast(const float & celsius, const float & humidity) const
{}
