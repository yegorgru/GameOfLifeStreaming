#include "Config.h"
#include "Print.h"
#include "Utils.h"

#include <iostream>
#include <sstream>
#include <boost/asio/ip/address.hpp>

namespace GameOfLife::Client {

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
    : mDescription("Game of Life Client Options") {
    namespace po = boost::program_options;
    mDescription.add_options()
        ("help,h", "produce help message")
        ("server-port,p", po::value<int>()->default_value(9090)->notifier(Config::validatePort), "multicast port (0-65535)")
        ("cell-size,c", po::value<int>()->default_value(20)->notifier(Config::validateCellSize), "size of each cell in pixels (5-50)")
        ("fps,f", po::value<int>()->default_value(30)->notifier(Config::validateFps), "target frames per second (1-60)")
        ("multicast-address,m", po::value<std::string>()->default_value("239.255.0.1")->notifier(Config::validateMulticastAddress), "multicast group address")
        ("log-level,l", po::value<std::string>()->default_value("info")->notifier(Config::validateLogLevel), "log level (trace, debug, info, warning, error, fatal)")
        ("log-file", po::value<std::string>()->default_value(""), "path to log file (if empty, logs to console)");
}

bool Config::parseCommandLine(int argc, char* argv[]) {
    namespace po = boost::program_options;

    try {
        po::store(po::parse_command_line(argc, argv, mDescription), mVariablesMap);
        po::notify(mVariablesMap);
    }
    catch (const po::error& e) {
        Print::PrintLine("Failed to parse command line arguments: " + std::string(e.what()));
        std::ostringstream oss;
        oss << mDescription;
        Print::PrintLine(oss.str());
        return false;
    }

    if (mVariablesMap.count("help")) {
        std::ostringstream oss;
        oss << mDescription;
        Print::PrintLine(oss.str());
        return false;
    }

    showCurrentConfig();
    return true;
}

int Config::getServerPort() const {
    return mVariablesMap["server-port"].as<int>();
}

int Config::getCellSize() const {
    return mVariablesMap["cell-size"].as<int>();
}

int Config::getTargetFps() const {
    return mVariablesMap["fps"].as<int>();
}

const std::string& Config::getMulticastAddress() const {
    return mVariablesMap["multicast-address"].as<std::string>();
}

const std::string& Config::getLogFilename() const {
    return mVariablesMap["log-file"].as<std::string>();
}

LogLevel Config::getLogLevel() const {
    const std::string logLevelStr = mVariablesMap["log-level"].as<std::string>();
    return logLevelMap.at(logLevelStr);
}

void Config::validatePort(int port) {
    namespace po = boost::program_options;
    if (!isValidPort(port)) {
        throw po::validation_error(po::validation_error::invalid_option_value, "server-port", std::to_string(port));
    }
}

void Config::validateCellSize(int size) {
    namespace po = boost::program_options;
    if (size < 5 || size > 50) {
        throw po::validation_error(po::validation_error::invalid_option_value, "cell-size", std::to_string(size));
    }
}

void Config::validateFps(int fps) {
    namespace po = boost::program_options;
    if (fps < 1 || fps > 60) {
        throw po::validation_error(po::validation_error::invalid_option_value, "fps", std::to_string(fps));
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

void Config::validateLogLevel(const std::string& level) {
    namespace po = boost::program_options;
    if (logLevelMap.find(level) == logLevelMap.end()) {
        throw po::validation_error(po::validation_error::invalid_option_value, "log-level", level);
    }
}

void Config::showCurrentConfig() const {
    Print::PrintLine("\nClient Configuration:");
    Print::PrintLine("---------------------");
    Print::PrintLine(Print::composeMessage("Multicast Port:", getServerPort()));
    Print::PrintLine(Print::composeMessage("Multicast Address:", getMulticastAddress()));
    Print::PrintLine(Print::composeMessage("Cell Size:", getCellSize()));
    Print::PrintLine(Print::composeMessage("Target FPS:", getTargetFps()));
    Print::PrintLine("Log level: " + mVariablesMap["log-level"].as<std::string>());
    Print::PrintLine(Print::composeMessage("Log File:", getLogFilename().empty() ? "<Console>" : getLogFilename()));
    Print::PrintLine("---------------------");
}

} // namespace GameOfLife::Client
