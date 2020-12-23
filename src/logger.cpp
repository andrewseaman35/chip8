#include <iostream>

#include "constants.h"
#include "logger.h"


using namespace std;

// const int DEBUG_LEVEL = Logger::Levels::Info | Logger::Levels::Debug;
const int DEBUG_LEVEL = Logger::Levels::Target;

Logger* Logger::_logger = nullptr;

Logger* Logger::getLogger() {
    if (_logger == nullptr) {
        _logger = new Logger();
    }
    return _logger;
}

void Logger::log(const char* s) {
    cout << s;
}

void Logger::info(const char* s) {
    if (DEBUG_LEVEL & Levels::Info) {
        this->log(s);
    }
}

void Logger::error(const char* s) {
    if (DEBUG_LEVEL & Levels::Error) {
        this->log(s);
    }
}

void Logger::debug(const char* s) {
    if (DEBUG_LEVEL & Levels::Debug) {
        this->log(s);
    }
}

void Logger::display(const char* s) {
    if (DEBUG_LEVEL & Levels::Display) {
        this->log(s);
    }
}

void Logger::target(const char* s) {
    if (DEBUG_LEVEL & Levels::Target) {
        this->log(s);
    }
}

void Logger::log(string s) {
    char buffer[s.length() + 1];
    strcpy(buffer, s.c_str());
    this->log(buffer);
}

void Logger::info(string s) {
    if (DEBUG_LEVEL & Levels::Info) {
        this->log(s);
    }
}

void Logger::error(string s) {
    if (DEBUG_LEVEL & Levels::Error) {
        this->log(s);
    }
}

void Logger::debug(string s) {
    if (DEBUG_LEVEL & Levels::Debug) {
        this->log(s);
    }
}

void Logger::display(string s) {
    if (DEBUG_LEVEL & Levels::Display) {
        this->log(s);
    }
}

void Logger::target(string s) {
    if (DEBUG_LEVEL & Levels::Target) {
        this->log(s);
    }
}
