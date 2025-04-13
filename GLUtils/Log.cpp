#include "Log.h"

#include <filesystem>
#include <syncstream>
#include <format>

void Log::initConsoleLogger(LogLevel logLevel) {
	if (mLogger) {
		throw std::runtime_error("Logger is already initialized");
	}
	mLogger = std::make_unique<LoggerConsole>();
	mLogLevel = logLevel;
}

void Log::initFileLogger(LogLevel logLevel, const std::string fileName) {
	if (mLogger) {
		throw std::runtime_error("Logger is already initialized");
	}
	mLogger = std::make_unique<LoggerFile>(fileName);
	mLogLevel = logLevel;
}

void Log::destroyLogger() {
    if (mLogger) {
        mLogger.reset();
        mLogLevel = LogLevel::Error;
    }
}

void Log::Trace(const std::string& message, std::source_location location) {
	log(LogLevel::Trace, message, location);
}

void Log::Debug(const std::string& message, std::source_location location) {
	log(LogLevel::Debug, message, location);
}

void Log::Info(const std::string& message, std::source_location location) {
	log(LogLevel::Info, message, location);
}

void Log::Warning(const std::string& message, std::source_location location) {
	log(LogLevel::Warning, message, location);
}

void Log::Error(const std::string& message, std::source_location location) {
	log(LogLevel::Error, message, location);
}

void Log::Throw(const std::string& message, std::source_location location) {
	log(LogLevel::Throw, message, location);
}

void Log::log(LogLevel messageLogLevel, const std::string& message, std::source_location location) {
	if (!mLogger) {
		Print::PrintLine(Print::composeMessage("(Logger is not initialized)", message));
	}
	if (messageLogLevel >= mLogLevel) {
		std::ostringstream oss;
		switch (messageLogLevel) {
		case LogLevel::Trace:
			oss << "[TRACE]\t";
			break;
		case LogLevel::Debug:
			oss << "[DEBUG]\t";
			break;
		case LogLevel::Info:
			oss << "[INFO]\t";
			break;
		case LogLevel::Warning:
			oss << "[WARN]\t";
			break;
		case LogLevel::Error:
			oss << "[ERROR]\t";
			break;
		case LogLevel::Throw:
			oss << "[THROW]\t";
			break;
		}
		std::filesystem::path filePath(location.file_name());
		oss << " " << filePath.filename().string() << ":" << location.line() << " | ";
		auto now = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
		oss << std::format("{:%Y-%m-%d %H:%M:%S}", now);
		oss << " | " << message;
		mLogger->printMessage(messageLogLevel, oss.str());
		if (messageLogLevel == LogLevel::Throw) {
			throw std::runtime_error(oss.str());
		}
	}
}

Log::LoggerFile::LoggerFile(const std::string& fileName) 
	: mFile(fileName, std::ios::out | std::ios::trunc)
{
	if (!mFile.is_open()) {
		throw std::runtime_error("Failed to open file for logging: " + fileName);
	}
}

Log::LoggerFile::~LoggerFile() {
	if (mFile.is_open()) {
		mFile.close();
	}
}

void Log::LoggerFile::printMessage(LogLevel logLevel, const std::string& message) {
	Print::PrintLine(message, mFile);
}

void Log::LoggerConsole::printMessage(LogLevel logLevel, const std::string& message) {
	if (logLevel <= LogLevel::Warning) {
		Print::PrintLine(message, std::cout);
	}
	else {
		Print::PrintLine(message, std::cerr);
	}
}