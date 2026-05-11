/**
 * 时间类
 */

#pragma once
#include <cstdint>
#include <string>
namespace cmuduo
{
    namespace base
    {
        class TimeStamp
        {
        public:
            TimeStamp();
            explicit TimeStamp(int64_t microsecondssinceEpoch);
            static TimeStamp now();
            std::string toString() const;

        private:
            int64_t microsecondssinceEpoch_;
        };

    } // namespace base

} // namespace cmuduo