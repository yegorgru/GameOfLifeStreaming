#include "Input.h"
#include "Print.h"

#include <iostream>
#include <syncstream>

namespace {
    std::string read(std::istream& stream) {
        std::string message;
        stream >> message;
        return message;
    }

    std::string getline(std::istream& stream) {
        std::string message;
        std::getline(stream, message);
        return message;
    }
}

void Input::initConsoleReader() {
	if (mReader) {
        Print::PrintLine("Reader will be reinialized. Previous reader will be destroyed.");
        destroyReader();
	}
    mReader = std::make_unique<ConsoleReader>();
}

void Input::initFileReader(const std::string& fileName) {
	if (mReader) {
        Print::PrintLine("Reader will be reinialized. Previous reader will be destroyed.");
        destroyReader();
	}
    mReader = std::make_unique<FileReader>(fileName);
}

void Input::destroyReader() {
    if (mReader) {
        mReader.reset();
    }
}

std::string Input::Read() {
	if (!mReader) {
        initConsoleReader();
        Print::PrintLine("Reader is not initialized. Default console reader will be used.");
	}
    auto input = mReader->readWord();
    if (mReader->eof()) {
        Print::PrintLine("End of file reached. Console reader will be used.");
        destroyReader();
        initConsoleReader();
    }
    return input;
}

std::string Input::ReadLine() {
    if (!mReader) {
        initConsoleReader();
        Print::PrintLine("Reader is not initialized. Default console reader will be used.");
    }
    auto input = mReader->readLine();
    if (mReader->eof()) {
        Print::PrintLine("End of file reached. Console reader will be used.");
        destroyReader();
        initConsoleReader();
    }
    return input;
}

Input::FileReader::FileReader(const std::string& fileName)
    : mFile(fileName, std::ios::in)
{
	if (!mFile.is_open()) {
		throw std::runtime_error("Failed to open file for reading: " + fileName);
	}
}

Input::FileReader::~FileReader() {
    if (mFile.is_open()) {
        mFile.close();
    }
}

std::string Input::FileReader::readWord() {
    return read(mFile);
}

std::string Input::FileReader::readLine() {
    return getline(mFile);
}

bool Input::FileReader::eof() {
    return mFile.eof();
}

std::string Input::ConsoleReader::readWord() {
    return read(std::cin);
}

std::string Input::ConsoleReader::readLine() {
    return getline(std::cin);
}

bool Input::ConsoleReader::eof() {
    return false;
}