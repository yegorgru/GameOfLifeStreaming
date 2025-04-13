#pragma once

#include "Print.h"

#include <string>
#include <memory>
#include <stdexcept>
#include <chrono>
#include <source_location>
#include <fstream>

enum class LogLevel {
	Trace = 0,
	Debug,
	Info,
	Warning,
	Error,
	Throw
};

class Log {
public:
	static void Trace(const std::string& message, std::source_location location = std::source_location::current());
	static void Debug(const std::string& message, std::source_location location = std::source_location::current());
	static void Info(const std::string& message, std::source_location location = std::source_location::current());
	static void Warning(const std::string& message, std::source_location location = std::source_location::current());
	static void Error(const std::string& message, std::source_location location = std::source_location::current());
	static void Throw(const std::string& message, std::source_location location = std::source_location::current());
public:
	static void initConsoleLogger(LogLevel logLevel);
	static void initFileLogger(LogLevel logLevel, const std::string fileName);
	static void destroyLogger();
private:
	static void log(LogLevel messageLogLevel, const std::string& message, std::source_location location);
private:
	class Logger {
	public:
		Logger() = default;
		virtual ~Logger() = default;
	public:
		virtual void printMessage(LogLevel logLevel, const std::string& message) = 0;
	};

	class LoggerFile : public Logger
	{
	public:
		LoggerFile(const std::string& fileName);
		~LoggerFile();
	private:
		void printMessage(LogLevel logLevel, const std::string& message) override;
	private:
		std::ofstream mFile;
	};

	class LoggerConsole : public Logger
	{
	public:
		LoggerConsole() = default;
	private:
		void printMessage(LogLevel logLevel, const std::string& message) override;
	};
private:
	using LoggerPtr = std::unique_ptr<Logger>;
private:
	inline static LoggerPtr mLogger = nullptr;
	inline static LogLevel mLogLevel = LogLevel::Error;
};