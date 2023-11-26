#ifndef TCP_SERVER
#define TCP_SERVER

#include <thread>
#include <memory>
#include "TcpConnection.hpp"
#include "Logger.hpp"
#include "IpStatistics.hpp"

#define maxConnections 4
#define maxErrorsBeforeBlock 2

using namespace boost;

class TcpServer {
private:
    asio::ip::tcp::endpoint ep_;
    asio::io_context &context_;
    asio::ip::tcp::acceptor acceptor_;
    const Logger &log_;
    unsigned totalConnections_;
    std::vector<std::pair <std::thread,std::shared_ptr<TcpConnection>>> connectionThreads_;
    std::shared_ptr<struct IpStatistics_t> ipStats_;
    
    void clearConnections();
    void acceptHandler(system::error_code, asio::ip::tcp::socket);
    bool acceptConnection(std::string &, uint_least16_t);

public:
    TcpServer(asio::io_context &, asio::ip::tcp, uint_least16_t, Logger &);
    ~TcpServer();
    void StartListen();
    unsigned GetTotalConnections() const;
};

#endif
