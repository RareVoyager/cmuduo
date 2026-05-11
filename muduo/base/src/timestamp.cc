#include <include/timestamp.h>
#include <time.h>

#pragma once
namespace cmuduo
{
    namespace base
    {
        TimeStamp::TimeStamp() : microsecondssinceEpoch_(0)
        {
        }

        TimeStamp::TimeStamp(int64_t microsecondssinceEpoch)
            : microsecondssinceEpoch_(microsecondssinceEpoch)
        {
        }

        TimeStamp TimeStamp::now()
        {
            return TimeStamp(time(NULL));
        }

        std::string TimeStamp::toString() const
        {
            char buf[128] = {0};
            tm *ti_time = localtime(&microsecondssinceEpoch_);
            snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d",
                     ti_time->tm_year + 1900,
                     ti_time->tm_mon + 1,
                     ti_time->tm_mday,
                     ti_time->tm_hour,
                     ti_time->tm_min,
                     ti_time->tm_sec);
            return buf;
        }
    }
}