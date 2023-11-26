#ifndef IP_STATISTICS
#define IP_STATISTICS

#include <thread>
#include <map>

struct IpStatistics_t {
private:
time_t start_;
// Simple map of ip (strings) to tuple of info - total connections, successful connections and connection errors
std::map<std::string, std::tuple<unsigned, unsigned, unsigned>> addressesInfo_;
mutable std::mutex m; // Simple mutex guarding against access to addressesInfo_;

public:
    /*
    * struct IpStatistics_t - Constructor
    * Simple statistics struct to hold some stats based on IP
    */
    IpStatistics_t() :
    start_(time(0))
    {
    }

    /*
    * To clear all stats
    */
    void Reset() {
        std::lock_guard<std::mutex> l(m);
        addressesInfo_.clear();
        start_ = time(0);
    }

    /*
    * Register a new connection from IP
    *
    * param - std::string ipAddress: IP in string
    */
    void RegisterConnection(std::string &ipAddress) {
        std::lock_guard<std::mutex> l(m);
        std::get<0>(addressesInfo_[ipAddress])++;
    }

    /*
    * Register a new successful connection from IP
    *
    * param - std::string ipAddress: IP in string
    */
    void RegisterSuccess(std::string &ipAddress) {
        std::lock_guard<std::mutex> l(m);
        std::get<1>(addressesInfo_[ipAddress])++;
    }

    /*
    * Register a new connection error from IP
    *
    * param - std::string ipAddress: IP in string
    */
    void RegisterErrors(std::string &ipAddress) {
        std::lock_guard<std::mutex> l(m);
        std::get<2>(addressesInfo_[ipAddress])++;
    }

    /*
    * Get total connections from IP
    *
    * param - std::string ipAddress: IP in string
    *
    * return unsigned - number of connections by this IP
    */
    unsigned GetConnection(const std::string &ipAddress) const {
        std::lock_guard<std::mutex> l(m);
        try {
            return std::get<0>(addressesInfo_.at(ipAddress));
        } catch (std::out_of_range) { // Doesn't exist
            return 0;
        }
    }

    /*
    * Get total succssful connections from IP
    *
    * param - std::string ipAddress: IP in string
    *
    * return unsigned - number of succss by this IP
    */
    unsigned GetSuccess(const std::string &ipAddress) const {
        std::lock_guard<std::mutex> l(m);
        try {
            return std::get<1>(addressesInfo_.at(ipAddress));
        } catch (std::out_of_range) { // Doesn't exist
            return 0;
        }
    }

    /*
    * Get total connections errors from IP
    *
    * param - std::string ipAddress: IP in string
    *
    * return unsigned - number of errors by this IP
    */
    unsigned GetErrors(const std::string &ipAddress) const {
        std::lock_guard<std::mutex> l(m);
        try {
            return std::get<2>(addressesInfo_.at(ipAddress));
        } catch (std::out_of_range) { // Doesn't exist
            return 0;
        }
    }

    /*
    * Get start time
    *
    * return time_t - when this IpStatistics_t got created
    */
    time_t GetStartTime() const {
        return start_;
    }
};

#endif