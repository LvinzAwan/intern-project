#include "core/Log.hpp"
#include <iostream>

namespace core {

void info(const std::string& msg) {
    std::cout << "[INFO] " << msg << "\n";
}

void error(const std::string& msg) {
    std::cerr << "[ERROR] " << msg << "\n";
}

}
