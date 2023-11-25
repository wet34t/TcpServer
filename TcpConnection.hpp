#ifndef TCP_CONNECTION
#define TCP_CONNECTION

#include <string>
#include <vector>
#include <thread>
#include <boost/asio.hpp>
#include "logger.hpp"

#define TYPESIZE 2
#define LENSIZE 4
#define GOODBYEHEADER "by"
#define timeout 10

using namespace boost;

class TcpConnection {
private:
    asio::ip::tcp::socket sock_;
    time_t start_;
    const logger &log_;
    bool endConnection_;

    std::string read(size_t);
    bool waitForResult(size_t);
    void processBody(size_t);
    size_t processHeader();

public:
    TcpConnection(boost::asio::io_context& context, const logger &log);
    void ReadPackets();
    asio::ip::tcp::socket& GetSock();
};

#endif