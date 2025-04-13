#include "Utils.h"

#include <algorithm>

bool isValidIpAddress(const std::string& ipAddress) {
    std::istringstream iss(ipAddress);
    std::string segment;
    int count = 0;
    while (std::getline(iss, segment, '.')) {
        if (segment.empty() || segment.size() > 3 || !std::all_of(segment.begin(), segment.end(), std::isdigit)) {
            return false;
        }
        int num = std::stoi(segment);
        if (num < 0 || num > 255) {
            return false;
        }
        if (++count > 4) {
            return false;
        }
    }
    return count == 4;
}

bool isValidPort(int port) {
    return port >= 0 && port <= 65535;
}

Id generateId() {
    static Id id = 0;
    return ++id;
}

std::optional<Id> stringToId(const std::string& idString) {
    return stringToInt<Id>(idString);
}

std::string idToString(const Id& id) {
    return std::to_string(id);
}