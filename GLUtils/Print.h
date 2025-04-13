#pragma once

#include <string>
#include <memory>
#include <utility>
#include <ostream>
#include <fstream>
#include <iostream>

class Print {
public:
    static void PrintWord(const std::string& message);
    static void PrintLine(const std::string& message);
    static void PrintWord(const std::string& message, std::ostream& stream);
    static void PrintLine(const std::string& message, std::ostream& stream);
public:
    template <typename... Args>
    static std::string composeMessage(Args&&... args);
public:
    static void initConsolePrinter();
    static void initFilePrinter(const std::string& fileName);
    static void destroyPrinter();
private:
    class Printer {
    public:
        Printer() = default;
        virtual ~Printer() = default;
    public:
        virtual void printMessage(const std::string& message) = 0;
    };

    class FilePrinter : public Printer {
    public:
        FilePrinter(const std::string& fileName);
        ~FilePrinter();
    private:
        void printMessage(const std::string& message) override;
    private:
        std::ofstream mFile;
    };

    class ConsolePrinter : public Printer {
    public:
        ConsolePrinter() = default;
        ~ConsolePrinter() = default;
    private:
        void printMessage(const std::string& message) override;
    };

private:
    using PrinterPtr = std::unique_ptr<Printer>;
private:
    inline static PrinterPtr mPrinter = nullptr;
};

template <typename... Args>
std::string Print::composeMessage(Args&&... args) {
    std::ostringstream oss;
    ((oss << args << " "), ...);
    std::string message = oss.str();
    if (message.length() > 0) {
        message.pop_back();
    }
    return message;
}