#include "Config.h"
#include "Log.h"

#include <iostream>
#include <sstream>
#include <regex>
#include <boost/program_options.hpp>
#include <boost/asio/ip/address.hpp>

namespace GameOfLife::Server {

namespace {
    const std::map<std::string, LogLevel> logLevelMap {
        {"throw", LogLevel::Throw},
        {"error", LogLevel::Error},
        {"warning", LogLevel::Warning},
        {"info", LogLevel::Info},
        {"debug", LogLevel::Debug},
        {"trace", LogLevel::Trace}
    };
}

Config::Config()
    : mDescription("Game of Life Server Options") {
    namespace po = boost::program_options;
    mDescription.add_options()
        ("help,h", "produce help message")
        ("port,p", po::value<int>()->default_value(9000), "server's port")
        ("log-level,l", po::value<std::string>()->default_value("info")->notifier(Config::validateLogLevel), "logging level: throw/error/warning/info/debug/trace")
        ("log-file,L", po::value<std::string>()->default_value(""), "logging file")        
        ("fps,f", po::value<int>()->default_value(1)->notifier(Config::validateFps), "frames per second (1-30)")
        ("grid-size,g", po::value<std::string>()->default_value("40x20")->notifier(Config::validateGridSize), "grid size in format WxH (e.g., 40x20)")
        ("fill-ratio,r", po::value<float>()->default_value(0.3f)->notifier(Config::validateFillRatio), "percentage of initially alive cells (0.0-1.0)")
        ("threads,t", po::value<int>()->default_value(2)->notifier(Config::validateThreadCount), "number of threads in the thread pool (1-64)")
        ("multicast-address,m", po::value<std::string>()->default_value("239.255.0.1")->notifier(Config::validateMulticastAddress), "multicast group address");
}

bool Config::parseCommandLine(int argc, char* argv[]) {
    namespace po = boost::program_options;

    try {
        po::store(po::parse_command_line(argc, argv, mDescription), mVariablesMap);
        po::notify(mVariablesMap);
    }
    catch (const po::error& e) {
        Print::PrintLine("Failed to parse command line arguments: " + std::string(e.what()));
        Print::PrintLine(Print::composeMessage(mDescription));
        return false;
    }

    if (mVariablesMap.count("help")) {
        Print::PrintLine(Print::composeMessage(mDescription));
        return false;
    }

    showCurrentConfig();
    return true;
}

const std::string& Config::getLogFilename() const {
    return mVariablesMap["log-file"].as<std::string>();
}

LogLevel Config::getLogLevel() const {
    const std::string logLevelStr = mVariablesMap["log-level"].as<std::string>();
    return logLevelMap.at(logLevelStr);
}

int Config::getPort() const {
    return mVariablesMap["port"].as<int>();
}

int Config::getFps() const {
    return mVariablesMap["fps"].as<int>();
}

std::pair<int, int> Config::getGridSize() const {
    std::string gridSizeStr = mVariablesMap["grid-size"].as<std::string>();
    std::regex gridSizeRegex("(\\d+)x(\\d+)");
    std::smatch matches;
    
    if (std::regex_match(gridSizeStr, matches, gridSizeRegex)) {
        int width = std::stoi(matches[1]);
        int height = std::stoi(matches[2]);
        return {width, height};
    }
    
    return {40, 20};
}

float Config::getFillRatio() const {
    return mVariablesMap["fill-ratio"].as<float>();
}

int Config::getThreadCount() const {
    return mVariablesMap["threads"].as<int>();
}

const std::string& Config::getMulticastAddress() const {
    return mVariablesMap["multicast-address"].as<std::string>();
}

void Config::validateLogLevel(const std::string& input) {
    namespace po = boost::program_options;
    if (logLevelMap.find(input) == logLevelMap.end()) {
        throw po::validation_error(po::validation_error::invalid_option_value, "log-level", input);
    }
}

void Config::validateFps(int fps) {
    namespace po = boost::program_options;
    if (fps < 1 || fps > 30) {
        throw po::validation_error(po::validation_error::invalid_option_value, "fps", std::to_string(fps));
    }
}

void Config::validateGridSize(const std::string& input) {
    namespace po = boost::program_options;
    std::regex gridSizeRegex("(\\d+)x(\\d+)");
    std::smatch matches;
    
    if (!std::regex_match(input, matches, gridSizeRegex)) {
        throw po::validation_error(po::validation_error::invalid_option_value, "grid-size", input);
    }
    
    int width = std::stoi(matches[1]);
    int height = std::stoi(matches[2]);
    
    if (width < 10 || width > 200 || height < 10 || height > 200) {
        throw po::validation_error(po::validation_error::invalid_option_value, "grid-size", input);
    }
}

void Config::validateFillRatio(float fillRatio) {
    namespace po = boost::program_options;
    if (fillRatio < 0.0f || fillRatio > 1.0f) {
        throw po::validation_error(po::validation_error::invalid_option_value, "fill-ratio", std::to_string(fillRatio));
    }
}

void Config::validateThreadCount(int count) {
    namespace po = boost::program_options;
    if (count < 1 || count > 64) {
        throw po::validation_error(po::validation_error::invalid_option_value, "threads", std::to_string(count));
    }
}

void Config::validateMulticastAddress(const std::string& address) {
    namespace po = boost::program_options;
    boost::system::error_code ec;
    auto ip_address = boost::asio::ip::make_address(address, ec);
    if (ec) {
        throw po::validation_error(po::validation_error::invalid_option_value, "multicast-address", address);
    }
}

void Config::showCurrentConfig() const {    
    Print::PrintLine("\nServer Configuration:");
    Print::PrintLine("--------------------");
    Print::PrintLine(Print::composeMessage("Port:", getPort()));
    Print::PrintLine(Print::composeMessage("Log level:", mVariablesMap["log-level"].as<std::string>()));
    
    const auto& logFile = getLogFilename();
    Print::PrintLine(Print::composeMessage("Log file:", (logFile.empty() ? "console" : logFile)));
    
    Print::PrintLine(Print::composeMessage("Fps:", getFps()));
    
    auto [width, height] = getGridSize();    
    Print::PrintLine(Print::composeMessage("Grid size:", width, "x", height));
    
    Print::PrintLine(Print::composeMessage("Fill ratio:", getFillRatio()));
    Print::PrintLine(Print::composeMessage("Thread count:", getThreadCount()));
    Print::PrintLine(Print::composeMessage("Multicast Address:", getMulticastAddress()));
    Print::PrintLine("--------------------");
}

} // namespace GameOfLife::Server