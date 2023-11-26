#ifndef TCP_CONNECTION
#define TCP_CONNECTION

#include <string>
#include <vector>
#include <boost/asio.hpp>
#include "Logger.hpp"
#include "IpStatistics.hpp"

#define TYPESIZE 2
#define LENSIZE 4
#define TIMEOUT 10
#define MAXBATCHSIZE 10

using namespace boost;

class TcpConnection {
private:
    asio::ip::tcp::socket sock_;
    const Logger &log_;
    std::shared_ptr<struct IpStatistics_t> ipStats_;
    std::string remoteIpAddress_;
    uint_least16_t remotePort_;
    bool byeReceived_;
    bool active_;
    bool err_;

    std::string getHeaderType(unsigned *, system::error_code &);
    void readBytes(size_t, system::error_code &);
    void readBytesToHex(size_t, unsigned *, system::error_code &);
    bool waitForBytes(size_t);
    void processBody(size_t);
    size_t processHeader();
    void handleError(std::string);

public:
    TcpConnection(asio::ip::tcp::socket, const Logger &, std::shared_ptr<struct IpStatistics_t>);
    ~TcpConnection();
    void ReadPackets();
    bool IsActive() const;
};

#endif