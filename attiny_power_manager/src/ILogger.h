#pragma once

class ILogger
{
   public:
    virtual void printf(const char* fmt, ...) = 0;
    virtual void tagged_printf(const char* tag, const char* fmt, ...) = 0;
};