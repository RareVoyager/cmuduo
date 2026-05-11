#include <include/log.h>
#include <include/timestamp.h>
#include <iostream>

namespace cmuduo
{
    namespace base
    {
        void Logger::setLevel(LogLevel level)
        {
            loglevel_ = level;
        }

        void Logger::log(std::string msg)
        {
            std::cout << '[' << LeveltoString() << ']'
                      << TimeStamp::now().toString() << ": " << msg << std::endl;
        }

        Logger::Logger()
        {
            // 设置一个默认等级
            loglevel_ = LogLevel::INFO;
        }

        // 私有的转字符串方法
        std::string Logger::LeveltoString()
        {
            switch (loglevel_)
            {
            case LogLevel::INFO:
                return "info";
            case LogLevel::ERROR:
                return "error";
            case LogLevel::FATAL:
                return "fatal";
            case LogLevel::DEBUG:
                return "debug";
            default:
                return "info";
            }
        }
    }
}
