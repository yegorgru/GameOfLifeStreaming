#pragma once

#include <string>
#include <boost/program_options.hpp>
#include <map>
#include "Log.h"

namespace GameOfLife::Client {

class Config {
public:
    Config();
public:
    bool parseCommandLine(int argc, char* argv[]);
public:
    int getServerPort() const;
    int getCellSize() const;
    int getTargetFps() const;
    const std::string& getMulticastAddress() const;
    const std::string& getLogFilename() const;
    LogLevel getLogLevel() const;
private:
    void showCurrentConfig() const;
private:
    static void validatePort(int port);
    static void validateCellSize(int size);
    static void validateFps(int fps);
    static void validateMulticastAddress(const std::string& address);
    static void validateLogLevel(const std::string& level);
private:
    using VariablesMap = boost::program_options::variables_map;
    using Description = boost::program_options::options_description;
private:
    VariablesMap mVariablesMap;
    Description mDescription;
};

} // namespace GameOfLife::Client
