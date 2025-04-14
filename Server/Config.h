#pragma once

#include "Log.h"
#include <boost/program_options.hpp>
#include <functional>
#include <map>
#include <string>

namespace GameOfLife::Server {

class Config
{
public:
    Config();
public:
    bool parseCommandLine(int argc, char* argv[]);
public:
    const std::string& getLogFilename() const;
    LogLevel getLogLevel() const;    
    int getPort() const;
    int getFps() const;
    std::pair<int, int> getGridSize() const;
    float getFillRatio() const;
    int getThreadCount() const;
    const std::string& getMulticastAddress() const;
private:
    void showCurrentConfig() const;
private:
    static void validateLogLevel(const std::string& input);    
    static void validateFps(int fps);
    static void validateGridSize(const std::string& input);
    static void validateFillRatio(float ratio);
    static void validateThreadCount(int count);
    static void validateMulticastAddress(const std::string& address);
private:
    using VariablesMap = boost::program_options::variables_map;
    using Description = boost::program_options::options_description;
    using ConfigHelpMap = std::map<std::string, std::string>;
private:
    VariablesMap mVariablesMap;
    Description mDescription;
    ConfigHelpMap mConfigHelp;
};

} // namespace GameOfLife::Server