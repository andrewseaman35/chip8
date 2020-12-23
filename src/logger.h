#ifndef LOGGER
#define LOGGER

#include <stdint.h>


using namespace std;


class Logger {
private:
    Logger() = default;
    Logger(const Logger&);
    Logger& operator= (const Logger&);

    static Logger* _logger;

public:
    static Logger* getLogger();

    enum Levels {
        Error    = 0x01,
        Info     = 0x02,
        Debug    = 0x04,
        Display  = 0x08,
        Target   = 0x10,
    };

    // Log without level checking
    void log(const char* s);
    void log(string s);

    void info(const char* s);
    void debug(const char* s);
    void error(const char* s);
    void display(const char* s);
    void target(const char* s);

    void info(string s);
    void debug(string s);
    void error(string s);
    void display(string s);
    void target(string s);
};



#endif // LOGGER
