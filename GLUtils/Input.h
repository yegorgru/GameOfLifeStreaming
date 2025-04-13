#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <memory>

class Input {
public:
    static std::string Read();
    static std::string ReadLine();
public:
    static void initConsoleReader();
    static void initFileReader(const std::string& fileName);
    static void destroyReader();
private:
    class Reader {
    public:
        Reader() = default;
        virtual ~Reader() = default;
    public:
        virtual std::string readWord() = 0;
        virtual std::string readLine() = 0;
        virtual bool eof() = 0;
    };
    using ReaderPtr = std::unique_ptr<Reader>;

    class FileReader : public Reader {
    public:
        FileReader(const std::string& fileName);
        ~FileReader();
    private:
        std::string readWord() override;
        std::string readLine() override;
        bool eof() override;
    private:
        std::ifstream mFile;
    };

    class ConsoleReader : public Reader {
    public:
        ConsoleReader() = default;
    private:
        std::string readWord() override;
        std::string readLine() override;
        bool eof() override;
    };

private:
    inline static ReaderPtr mReader = nullptr;
};