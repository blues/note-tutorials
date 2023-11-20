#pragma once

#include "Notecard.h"

extern Notecard notecard;

class NoopDFUUpdater2 {
public:
    bool setup() {
        notecard.logDebug("Host DFU updates are not supported in this platform.");
        return false;
    }

    bool begin(size_t length) {
        return false;
    }

    bool write(const uint8_t* data, size_t length) {
        return false;
    }

    bool end() {
        return false;
    }

    bool hasError() {
        return false;
    }

    String getError() {
        return String();
    }

    bool executeImage() {
        return false;
    }
};

class NoopDFUUpdater {
public:
    bool setup();
    bool begin(size_t length);

    bool write(const uint8_t* data, size_t length);
    bool end();

    bool hasError();

    String getError();

    bool executeImage();
};



#if 0
template <typename T>
inline constexpr bool _dfu_isSupported(T updater)
{
  return true;
}

template <>
inline constexpr bool _dfu_isSupported(NoopDFUUpdater noop)
{
    return false;
}

template<typename T>
inline constexpr void isPlatformSupported(T value) {
    if constexpr (_dfu_isSupported(value))
    {
    }
    else {
        static_assert(!sizeof(value), "Host DFU is not supported on this platform.");
    }
}
#endif

#if defined(ARDUINO_ARCH_ESP8266)
#include "dfu-esp8266.h"
#elif defined(ARDUINO_ARCH_ESP32)
#include "dfu-esp32.h"
#else
typedef NoopDFUUpdater DFUUpdater;
#endif