#ifndef LOGGER
#define LOGGER

#include <iostream>
#include <thread>

class Logger {
private:
std::ostream *infoStream_;
std::ostream *errStream_;
mutable std::mutex m;

public:
    Logger(std::ostream &infoStream, std::ostream &errStream)
    {
        infoStream_ = &infoStream;
        errStream_ = &errStream;
    }

    template <typename T>
    void Info(T message) const {
        std::lock_guard<std::mutex> l(m);
        *infoStream_ << "Log info: " << time(0) << " thread: " << std::this_thread::get_id() << ": " << message << "\n";
    }

    template <typename T>
    void Error(T message) const {
        std::lock_guard<std::mutex> l(m);
        *errStream_ << "Log Error: " << time(0) << " thread: " << std::this_thread::get_id() << ": " <<  message << "\n";
    }

    template <typename T>
    const Logger& operator<<(T message) const {
        std::lock_guard<std::mutex> l(m);
        *infoStream_ << message;
        return *this;
    }
};

#endif