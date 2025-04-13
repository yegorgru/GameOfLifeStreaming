#include "Print.h"

#include <iostream>
#include <syncstream>

void Print::initConsolePrinter() {
	if (mPrinter) {
        PrintLine("Printer will be reinialized. Previous printer will be destroyed.");
        destroyPrinter();
	}
    mPrinter = std::make_unique<ConsolePrinter>();
}

void Print::initFilePrinter(const std::string& fileName) {
	if (mPrinter) {
        PrintLine("Printer will be reinialized. Previous printer will be destroyed.");
        destroyPrinter();
	}
    mPrinter = std::make_unique<FilePrinter>(fileName);
}

void Print::destroyPrinter() {
    if (mPrinter) {
        mPrinter.reset();
    }
}

void Print::PrintWord(const std::string& message) {
	if (!mPrinter) {
        initConsolePrinter();
        PrintLine("Printer is not initialized. Default console printer will be used.");
	}
    mPrinter->printMessage(message);
}

void Print::PrintLine(const std::string& message) {
    PrintWord(message + "\n");
}

void Print::PrintWord(const std::string& message, std::ostream& stream) {
	std::osyncstream out{ stream };
	out << message;
    out.flush();
}

void Print::PrintLine(const std::string& message, std::ostream& stream) {
    PrintWord(message + "\n", stream);
}

Print::FilePrinter::FilePrinter(const std::string& fileName)
    : mFile(fileName, std::ios::out | std::ios::trunc)
{
	if (!mFile.is_open()) {
		throw std::runtime_error("Failed to open file for printing: " + fileName);
	}
}

Print::FilePrinter::~FilePrinter() {
    if (mFile.is_open()) {
        mFile.close();
    }
}

void Print::FilePrinter::printMessage(const std::string& message) {
    Print::PrintWord(message, mFile);
}

void Print::ConsolePrinter::printMessage(const std::string& message) {
    Print::PrintWord(message, std::cout);
}