#ifndef IP_STATISTICS
#define IP_STATISTICS

#include <thread>
#include <map>

struct IpStatistics_t {
private:
time_t start_;
std::map<std::string, std::tuple<unsigned, unsigned, unsigned>> addressesInfo_;
mutable std::mutex m;

public:
    IpStatistics_t() :
    start_(time(0))
    {
    }

    void Reset() {
        std::lock_guard<std::mutex> l(m);
        addressesInfo_.clear();
    }

    void RegisterConnection(std::string &ipAddress) {
        std::lock_guard<std::mutex> l(m);
        std::get<0>(addressesInfo_[ipAddress])++;
    }

    void RegisterSuccess(std::string &ipAddress) {
        std::lock_guard<std::mutex> l(m);
        std::get<1>(addressesInfo_[ipAddress])++;
    }

    void RegisterErrors(std::string &ipAddress) {
        std::lock_guard<std::mutex> l(m);
        std::get<2>(addressesInfo_[ipAddress])++;
    }

    unsigned GetConnection(const std::string &ipAddress) const {
        std::lock_guard<std::mutex> l(m);
        try {
            return std::get<0>(addressesInfo_.at(ipAddress));
        } catch (std::out_of_range) {
            return 0;
        }
    }

    unsigned GetSuccess(const std::string &ipAddress) const {
        std::lock_guard<std::mutex> l(m);
        try {
            return std::get<1>(addressesInfo_.at(ipAddress));
        } catch (std::out_of_range) {
            return 0;
        }
    }

    unsigned GetErrors(const std::string &ipAddress) const {
        std::lock_guard<std::mutex> l(m);
        try {
            return std::get<2>(addressesInfo_.at(ipAddress));
        } catch (std::out_of_range) {
            return 0;
        }
    }

    unsigned GetTimeAlive() const {
        return time(0) - start_;
    }
};

#endif