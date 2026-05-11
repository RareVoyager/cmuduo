/**
 * 日志打印
 */

#pragma once

#include <string>

#include "noncopyable.h"
#include "singleton.h"

namespace cmuduo
{
	namespace base
	{
		// 日志级别 INFO / ERROR / FATAL / DEBUG
		enum class LogLevel
		{
			INFO,
			ERROR,
			FATAL,
			DEBUG
		};

		class Logger : noncopyable
		{
		public:
			Logger();
			// 设置日志等级
			void setLevel(LogLevel level);

			// 打印日志
			void log(std::string msg);

		private:
			LogLevel loglevel_;

			std::string LeveltoString();
		};
	}// namespace base
}// namespace cmuduo


#define logger (::Singleton<::cmuduo::base::Logger>::GetInstance())

#define LOG(msg, ...)                                        \
	do                                                       \
	{                                                        \
		char buf[1024];                                      \
		std::snprintf(buf, sizeof(buf), msg, ##__VA_ARGS__); \
		logger->log(buf);                                    \
	} while (0)

#define LOG_INFO(msg, ...)                                \
	do                                                    \
	{                                                     \
		logger->setLevel(::cmuduo::base::LogLevel::INFO); \
		LOG(msg, ##__VA_ARGS__);                          \
	} while (0)

#define LOG_ERROR(msg, ...)                                \
	do                                                     \
	{                                                      \
		logger->setLevel(::cmuduo::base::LogLevel::ERROR); \
		LOG(msg, ##__VA_ARGS__);                           \
	} while (0)

#define LOG_FATAL(msg, ...)                                \
	do                                                     \
	{                                                      \
		logger->setLevel(::cmuduo::base::LogLevel::FATAL); \
		LOG(msg, ##__VA_ARGS__);                           \
		exit(-1);                                          \
	} while (0)

#define LOG_DEBUG(msg, ...)                                \
	do                                                     \
	{                                                      \
		logger->setLevel(::cmuduo::base::LogLevel::DEBUG); \
		LOG(msg, ##__VA_ARGS__);                           \
	} while (0)