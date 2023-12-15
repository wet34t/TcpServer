#ifndef LOGGER
#define LOGGER

#include <iostream>
#include <thread>

class Logger {
private:
std::ostream *infoStream_;
std::ostream *errStream_;
mutable std::mutex mInfo_; // Mutex to prevent multiple logging simultaneously
mutable std::mutex mErr_; // Mutex to prevent multiple error logging simultaneously

public:
    /*
    * class Logger - Constructor
    * Simple logger class to do some logging
    * 
    * param - std::ostream &infoStream: output stream for logging info
    * param - std::ostream &errStream: output stream for logging errors 
    */
    Logger(std::ostream &infoStream, std::ostream &errStream)
    {
        infoStream_ = &infoStream;
        errStream_ = &errStream;
    }

    /*
    * Writes a message of template type T to the infostream
    *
    * param - T message: info to be logged, also logs some stuff like time and thread id
    */
    template <typename T>
    void Info(T message) const {
        std::lock_guard<std::mutex> l(mInfo_);
        // May throw exception if message type not supported by << operator
        *infoStream_ << "Log info: " << time(0) << " thread: " << std::this_thread::get_id() << ": " << message << "\n";
    }

    /*
    * Writes a message of template type T to the errorstream
    *
    * param - T message: erro to be logged, also logs some stuff like time and thread id
    */
    template <typename T>
    void Error(T message) const {
        std::lock_guard<std::mutex> l(mErr_);
        // May throw exception if message type not supported by << operator
        *errStream_ << "Log Error: " << time(0) << " thread: " << std::this_thread::get_id() << ": " <<  message << "\n";
    }

    /*
    * Similar to info, but instead use operator <<
    *
    * param - T message: info to be logged
    * 
    * return const Logger& - this logger
    */
    template <typename T>
    const Logger& operator<<(T message) const {
        std::lock_guard<std::mutex> l(mInfo_);
        // May throw exception if message type not supported by << operator
        *infoStream_ << message;
        return *this;
    }
};

#endif