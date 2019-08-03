#pragma once

#include <stdarg.h>
#include "ILogger.h"
#include "circular_buffer.h"

class DebugLogger : public ILogger {
   public:
    DebugLogger(unsigned int bufsize) {
        uint8_t* buffer = malloc(bufsize * sizeof(uint8_t));
        _cbuf = circular_buf_init(buffer, bufsize);
    }

    unsigned long count(void) { return circular_buf_size(_cbuf); }

    void printf(const char* fmt, ...) {
        char tempbuf[64];
        va_list arg;
        va_start(arg, fmt);
        int written = vsnprintf(tempbuf, sizeof(tempbuf), fmt, arg);
        if (written > 0)
            write(tempbuf, written);
        va_end(arg);
        return written;
    }

    void tagged_printf(const char* tag, const char* fmt, ...) {
        char tempbuf[64];
        int written;

        written = snprintf(tempbuf, sizeof(tempbuf), "%lu %s: ", millis(), tag);
        if (written > 0) {
            write(tempbuf, strlen(tempbuf));
        }

        va_list arg;
        va_start(arg, fmt);
        written = vsnprintf(tempbuf, sizeof(tempbuf), fmt, arg);
        if (written > 0) {
            write(tempbuf, written);
        }
        va_end(arg);
        return written;
    }

    void write(const char* data, unsigned long len) {
        for (uint8_t i = 0; i < len; ++i) {
            circular_buf_put(_cbuf, data[i]);
        }
    }

    /* returns how many bytes actually read */
    unsigned int read(char* data, unsigned int len) {
        unsigned int readn = 0;
        while (!circular_buf_empty(_cbuf) && readn < len) {
            circular_buf_get(_cbuf, &data[readn]);
            readn++;
        }
        return readn;
    }

   private:
    cbuf_handle_t _cbuf;
};
