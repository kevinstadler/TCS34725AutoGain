# `TCS34725AutoGain` 

[![arduino-library-badge](https://www.ardu-badge.com/badge/TCS34725AutoGain.svg)](https://www.ardu-badge.com/TCS34725AutoGain) [![GitHub release](https://img.shields.io/github/tag/kevinstadler/TCS34725AutoGain.svg)](https://github.com/kevinstadler/TCS34725AutoGain/releases) [![Build Status](https://github.com/kevinstadler/TCS34725AutoGain/workflows/Arduino%20library%20build/badge.svg)](https://github.com/kevinstadler/TCS34725AutoGain/actions)

This is a fork of hideakitai's TCS34725 library, which adds support for:

* automatic setting of gain and integration time based on a clear sensor count target
* fine-grained control of all sensor modes (busy reading vs single readouts, power saving/wait times, interrupt thresholds)
* blocking and non-blocking sensor readout methods
* all configuration setter methods double as getters: when called without arguments, methods will return their current configuration setting based on freshly read information from the device register

### Basic usage

This fork is fully compatible with the original `TCS34725` library, meaning any existing code will continue to work. Just swap `TCS34725.h` for `TCS34725AutoGain.h` and you can start using the additional methods.

```cpp
//#include "TCS34725.h"
#include "TCS34725AutoGain.h"

TCS34725 tcs;

void setup(void)
{
    Wire.begin();
    if (!tcs.attach(Wire)) {
        // handle error
    }

...
```

### Automatic setting of gain and integration time

The following method performs some measurements to determine the best gain and integration time for hitting the desired minimum clear channel count. It returns `true` if such a setting was found, or `false` if the final setting fails to produce the desired count (usually indicating that light conditions are too dark).

```cpp
    bool autoGain(uint16_t minClearCount = 100, Gain initGain = Gain::X01);
```

After returning from the method the sensor will be set to the lowest gain and integration time that is capable of reliably producing the given minimum count, and a first measurement using these new settings is instantly available by calling `raw()`.

(A minimum clear channel count of at least 100 is recommended for reliable lux calculation in the [DN40 Lux and CCT Calculation Application Note](https://ams.com/documents/20143/36005/ColorSensors_AN000166_1-00.pdf/d1290c78-4ef1-5b88-bff0-8e80c2f92b6b).)

### Sensor modes

The sensor passes through internal system states depending on settings of the enable register (see the state diagrams on pages 7 and 10 of the [data sheet](https://cdn-shop.adafruit.com/datasheets/TCS34725.pdf)). Most libraries directly enter the mode in which the sensor repeatedly takes measurements as fast as possible (referred to as `RGBC`), but for power saving or less frequent readout intervals, the following other modes are available (they are *modes* rather than states because each mode actually describes a specific transition pattern through the internal sensor states).

```cpp
    enum class Mode : uint8_t {
        Undefined, // unknown/undefined/error state
        Sleep,    // !PON: in sleep state
        Idle,     //  PON & !AEN: in idle state
        RGBC,     //  PON &  AEN & !WEN: repeatedly taking RGBC measurements
        WaitRGBC  //  PON &  AEN &  WEN: taking RGBC measurements with waits in between
    };

    // get current state
    Mode mode();

    // set the sensor mode -- this method only alters those bits of the enable register
    // required to set the target mode, other bits are left unchanged
    Mode mode(Mode m);
```

In the `WaitRGBC` mode, the sensor will pause in between reads to save power. The wait time can be controlled using the following methods (see the [TCS34725 Modes Full Demo](examples/TCS34725_Modes_Full_Demo/TCS34725_Modes_Full_Demo.ino) sketch for an example).

```cpp
    // unless the second argument is true, this method only updates the wait time configuration,
    // but does not automatically switch the sensor to WaitRGBC mode
    float wait(float ms, bool enterWaitRGBC = false); /* between 2.4ms and 256*28.8 = 7372.8ms */
```

### Blocking and non-busy reads

```cpp
    bool available(float timeoutMs);
    bool singleRead();
```

### New methods for setting interrupt thresholds

See the [TCS34725 Interrupt Thresholds](examples/TCS34725_Interrupt_Thresholds/TCS34725_Interrupt_Thresholds.ino) sketch for an example.

```cpp
    bool interrupt();
    uint8_t persistence();

    uint16_t lowInterruptThreshold();
    uint16_t highInterruptThreshold();

    void interruptThresholds(uint16_t low, uint16_t high);
    void lowInterruptThreshold(uint16_t lowThreshold);
    void highInterruptThreshold(uint16_t highThreshold);
```

### Other (non-breaking) changes/enhancements of hideakitai's API

* A number of configuration *setter* methods return information about the effective value written.
* The same configuration setter methods can now also be called omitting the argument, which makes them read and return their configuration setting based on freshly read information from the device register.

```cpp
    float integrationTime(float ms); // 2.4 - 614.4 ms
    float integrationTime();

    int16_t integrationCycles(int16_t nCycles); // 1 - 256
    int16_t integrationCycles();

    // returns a numeric value representing the gain (i.e. 1, 4, 16 or 60)
    float gain(Gain g);
    float gain();
```

```

============================= original library documentation below =============================

```

# TCS34725

Arduino library for TCS34725 RGB Color Sensor

## Description

This library is partially ported from this [circuitpython library](https://github.com/adafruit/Adafruit_CircuitPython_TCS34725).

- optimized performance (no suspend caused by ```delay()```)
- easily check if measurement cycle has done
- lux and color temperature calculation (ported from [here](https://github.com/adafruit/Adafruit_CircuitPython_TCS34725))
- interrupt feature is automatically enabled
- various parameter adjustment through APIs

## Usage

``` C++
#include "TCS34725.h"
TCS34725 tcs;

void setup(void)
{
    Serial.begin(115200);

    Wire.begin();
    if (!tcs.attach(Wire))
        Serial.println("ERROR: TCS34725 NOT FOUND !!!");

    tcs.integrationTime(33); // ms
    tcs.gain(TCS34725::Gain::X01);

    // set LEDs...
}

void loop(void)
{
    if (tcs.available()) // if current measurement has done
    {
        TCS34725::Color color = tcs.color();
        Serial.print("Color Temp : "); Serial.println(tcs.colorTemperature());
        Serial.print("Lux        : "); Serial.println(tcs.lux());
        Serial.print("R          : "); Serial.println(color.r);
        Serial.print("G          : "); Serial.println(color.g);
        Serial.print("B          : "); Serial.println(color.b);
    }
}
```

## Parameter Adjustment

### Integration Time

Integration time for RGBC measurement can be set in millisecond.
The parameter range is from 2.4ms to 614.4ms.

``` C++
    void integrationTime(float ms) // 2.4 - 614.4 ms
```

### Gain

The gain of TCS can be set like:

``` C++
    tcs.gain(TCS34725::Gain::X01);
```

and you can choose gains as follows.

``` C++
    enum class Gain : uint8_t { X01, X04, X16, X60 };
```

### RGB Scaling

When RGB color is calculated, simple scaling by pow() is applied.

``` C++
    rgb_clr = pow(raw_rgb / raw_clear, scaling) * 255.f;
```

Default scaling parameter is 2.4, and if you want to disable it, please set to 1.0.

``` C++
    tcs.scale(2.5); // default
    tcs.scale(1.0); // disable scaling
```

### Glass Attenuation

This parameter is applied when lux and color temperature is calculated.
Please see [this library](https://github.com/adafruit/Adafruit_CircuitPython_TCS34725) for detail.

``` C++
    // The Glass Attenuation (FA) factor used to compensate for lower light
    // levels at the device due to the possible presence of glass. The GA is
    // the inverse of the glass transmissivity (T), so GA = 1/T. A transmissivity
    // of 50% gives GA = 1 / 0.50 = 2. If no glass is present, use GA = 1.
    // See Application Note: DN40-Rev 1.0 Ã¢â‚¬â€œ Lux and CCT Calculations using
    // ams Color Sensors for more details.
    tcs.glassAttenuation(1.0); // default: no glass
    tcs.glassAttenuation(0.5); // glass factor 50%
```

## APIs

``` C++
    struct Color { float r, g, b; };
    union RawData
    {
        struct
        {
            uint16_t c;
            uint16_t r;
            uint16_t g;
            uint16_t b;
        };
        uint8_t raw[sizeof(uint16_t) * 4];
    };

    bool attach(WireType& w = Wire)
    void power(bool b)
    void enableColorTempAndLuxCalculation(bool b)

    void integrationTime(float ms) // 2.4 - 614.4 ms
    void gain(Gain g)
    void scale(float s)
    void glassAttenuation(float v)
    void persistence(uint16_t data)

    bool available()

    const RawData& raw() const
    const Color& color() const
    float lux() const
    float colorTemperature() const

    void interrupt(bool b)
    void clearInterrupt()

    // to manipulate registers, please use these APIs
    void write8(Reg reg, uint8_t value)
    uint8_t read8(Reg reg)
    uint16_t read16(Reg reg)
```

## Raw Register Manipulation

``` C++
    enum class Reg : uint8_t
    {
        ENABLE = 0x00,
        ATIME = 0x01,
        WTIME = 0x03,
        AILTL = 0x04,
        AILTH = 0x05,
        AIHTL = 0x06,
        AIHTH = 0x07,
        PERS = 0x0C,
        CONFIG = 0x0D,
        CONTROL = 0x0F,
        ID = 0x12,
        STATUS = 0x13,
        CDATAL = 0x14,
        CDATAH = 0x15,
        RDATAL = 0x16,
        RDATAH = 0x17,
        GDATAL = 0x18,
        GDATAH = 0x19,
        BDATAL = 0x1A,
        BDATAH = 0x1B,
    };

    enum class Mask : uint8_t
    {
        ENABLE_AIEN = 0x10,
        ENABLE_WEN = 0x08,
        ENABLE_AEN = 0x02,
        ENABLE_PON = 0x01,
        STATUS_AINT = 0x10,
        STATUS_AVALID = 0x01
    };
```


## License

MIT
