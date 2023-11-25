#ifndef LOGGER
#define LOGGER

#include <iostream>

class logger {
private:
std::ostream *stream_;
mutable std::mutex m;

public:
    logger(std::ostream &stream)
    {
        stream_ = &stream;
    }

    template <typename T>
    void Info(T message) const {
        std::lock_guard<std::mutex> l(m);
        *stream_ << "Log info: " << message << "\n";
    }

    template <typename T>
    void Error(T message) const {
        std::lock_guard<std::mutex> l(m);
        *stream_ << "Log Error: " << message << "\n";
    }

    template <typename T>
    const logger& operator<<(T message) const {
        std::lock_guard<std::mutex> l(m);
        *stream_ << message;
        return *this;
    }
};

#endif