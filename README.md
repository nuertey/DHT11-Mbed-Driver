# DHT11-Mbed-Driver
C++ header-only library for driving DHT11/DHT22 sensor module(s) on ARM Mbed-enabled targets in order to obtain temperature/humidity readings.

For ease of use, flexibility and readability of the code, the driver has been written in a modern C++ (C++17/C++20) templatized class idiom. As Bjarne Stroustrup is fond of saying, "C++ implementations obey the zero-overhead principle: What you don’t use, you don’t pay for."

## Sensor Peripheral (DHT11) Description/Details
The DHT11 sensor module measures and provides humidity and temperature values serially over a single wire. Its characteristics are as follows:
   
* It can measure relative humidity in percentages (20 to 90% RH) and temperature in degree Celsius in the range of 0 to 50°C.

* It has 4 pins; one of which is used for data communication in serial form.

* Pulses of different TON and TOFF are decoded as logic 1 or logic 0 or start pulse or end of frame.

* Power Supply: 3 to 5V DC, 2.5mA max current use during conversion (while requesting data).

* Operating range: Good for 20-80% humidity readings with 5% accuracy.

* Good for 0-50°C temperature readings ±2°C accuracy.

* No more than 1 Hz sampling rate (once every second).

Reference: https://www.mouser.com/datasheet/2/758/DHT11-Technical-Data-Sheet-Translated-Version-1143054.pdf

### Cautions
These cautions are key to successful sensor operation: 

* When the connecting cable to the data pin is shorter than 20 metres, a 5K pull-up resistor is recommended;
 
* When the connecting cable to the data pin is longer than 20 metres, choose an appropriate pull-up resistor as needed.

* When power is supplied to the sensor, do not send any instructions to the sensor in within one second in order to pass the unstable status phase. 

Reference: https://www.mouser.com/datasheet/2/758/DHT11-Technical-Data-Sheet-Translated-Version-1143054.pdf

## Tested Target (and peripheral) :
NUCLEO F767ZI 
DHT11 Sensor Module
5K pull-up resistor
(All properly connected via breadboard with appropriate power supply)

## Compiler 
GCC ARM 10.2.1
(GNU Arm Embedded Toolchain 10-2020-q4-major) 10.2.1 20201103 (release)
gcc-arm-none-eabi-10-2020-q4-major/bin

## Build Instructions :
Double-check mbed tool config to ensure you are picking up an up-to-date GCC ARM compiler:

```
mbed config -L
[mbed] Working path "/home/.../Workspace/DHT11-Mbed-Driver" (library)
[mbed] Program path "/home/.../Workspace/DHT11-Mbed-Driver"
[mbed] Global config:
GCC_ARM_PATH=/home/.../opt/gcc-arm-none-eabi-10-2020-q4-major/bin
PROTOCOL=ssh

[mbed] Local config (/home/.../Workspace/DHT11-Mbed-Driver):
No local configuration is set
```

Now build in whichever way you prefer whilst ensuring that your build profile file specifies C++20. See mine for an illustration. 

And as I prefer the command-line, I build in the following manner:

```
mbed compile -m nucleo_f767zi -t GCC_ARM --profile my_profile.json -v -c
```

Then deploy the following equivalent image of yours to your target under test and reboot:

```
./BUILD/NUCLEO_F767ZI/GCC_ARM-MY_PROFILE/DHT11-Mbed-Driver.bin
```

## Output Snippet

```
Coming soon as testing is not yet complete and time is scarce as one multitasks between projects.

```

## Usage Example
Consult the provided test case application, "main.cpp" for a comprehensive example of how to use each feature of the library.

Just for the sake of brevity, here is a simple illustration of the core usage pattern :

```c++
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
        ThisThread::sleep_for(DHT11_DEVICE_STABLE_STATUS_DELAY);
    }
```
The above is merely an illustration. For a comprehensive example that actually compiles, consult the aforementioned test application.

## A Note on Dependencies
The MbedOS version was baselined off of:

```
commit ecb3c8c837162c73537bd0f3592c6e2a42994045 (HEAD, tag: mbed-os-5.11.4, tag: latest)
Merge: a8d1d26fa7 33d779f540
Author: Martin Kojtal <martin.kojtal@arm.com>
Date:   Mon Feb 11 11:56:11 2019 +0100

    Merge pull request #9646 from ARMmbed/release-candidate
    
    Release candidate for mbed-os-5.11.4
```

## License
MIT License

Copyright (c) 2021 Nuertey Odzeyem

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
