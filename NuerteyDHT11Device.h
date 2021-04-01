/***********************************************************************
* @file      NuerteyDHT11Device.h
*
*    DHT11/DHT22 temperature and humidity sensor driver targetted for  
*    ARM Mbed platform. 
*
* @brief   Drive the DHT11/DHT22 sensor module in order to obtain 
*          temperature/humidity readings.
* 
* @note    Written in a modern C++ (C++17/C++20) templatized class/idiom.
*
* @warning
*
* @author    Nuertey Odzeyem
* 
* @date      April 1, 2021
*
* @copyright Copyright (c) 2021 Nuertey Odzeyem. All Rights Reserved.
***********************************************************************/
#pragma once

#include "mbed.h"

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
    static constexpr int       JSON_WEB_TOKENS_BUFFER_SIZE = 6;

    NuerteyDHT11Device(PinName thePinName);

    NuerteyDHT11Device(const NuerteyDHT11Device&) = delete;
    NuerteyDHT11Device& operator=(const NuerteyDHT11Device&) = delete;

    virtual ~NuerteyDHT11Device();

    [[nodiscard]] bool        IsOpen() const;
    const std::string &       GetName() const;
    unsigned int              GetPeriodDurationMicroSecs() const;

    void DataPinRising();
    void DataPinFalling();

protected:
    // Keep protected to enforce RAII. No one should be explicitly calling these anyway.
    bool Open();
    bool Close();

private:
    PinName              m_TheDataPinName;      // Self-explanatory.

    int                  DHT_data[6];
    std::array<int, M>   rBuffer;

    time_t               _lastReadTime;
    float                _lastTemperature;
    float                _lastHumidity;

    enum _snd_pcm_stream  m_DeviceType;          // SND_PCM_STREAM_CAPTURE | SND_PCM_STREAM_PLAYBACK
    uint8_t               m_NumberOfChannels;    // Quality of the recorded audio.
    snd_pcm_uframes_t     m_FramesPerPeriod;     // Latency - lower numbers will decrease latency and increase CPU usage.
    uint32_t              m_SamplingRate;        // Quality of the recorded audio.
    snd_pcm_format_t      m_Format;              // Bit depth - Quality.
    snd_pcm_t*            m_pPCMHandle;          // Returned PCM handle.
    bool                  m_IsDeviceOpen;        // A way to track if device open and configuration truly succeeded.
    snd_pcm_hw_params_t*  m_pHardwareParameters; // Just as a cautionary step, keep this around until we are destructing the class.
    bool                  m_IsOperationErrorPresent; // A way to track if a read or write error exists.

};

template <typename T>
NuerteyDHT11Device<T>::NuerteyDHT11Device(PinName thePinName)
    : m_TheDataPinName(thePinName)
{
}

template <typename T>
NuerteyDHT11Device<T>::~NuerteyDHT11Device()
{
}
