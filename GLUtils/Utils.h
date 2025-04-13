#pragma once

#include <optional>
#include <string>
#include <functional>
#include <sstream>
#include <concepts>

bool isValidIpAddress(const std::string& ipAddress);
bool isValidPort(int port);

template <std::integral T>
std::optional<T> stringToInt(const std::string& value, const std::function<bool(T)>& condition = [](T) { return true; }) {
    std::istringstream iss(value);
    T result;
    iss >> result;
    if (iss.fail() || !iss.eof() || !condition(result)) {
        return std::nullopt;
    }
    return result;
}

using Id = uint64_t;
Id generateId();
std::optional<Id> stringToId(const std::string& idString);
std::string idToString(const Id& id);