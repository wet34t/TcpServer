#include <thread>
#include <memory>
#include "logger.hpp"

using namespace boost;

struct TcpServer_t {
private:
    asio::ip::tcp::endpoint ep_;
    asio::io_context context_;
    asio::ip::tcp::acceptor acceptor_;
    const logger &log_;
    std::vector<std::thread> threads_;
    // Malicious IPs
    // Free up connections

void acceptCallBack(std::shared_ptr<TcpConnection> con) {
    log_ << std::this_thread::get_id() << ": acceptCallBack\n";
    std::thread t(&TcpConnection::ReadPackets, con);
    threads_.push_back(std::move(t));
}

public:
    TcpServer_t(asio::ip::tcp type, uint_least16_t port, logger &log) :
    context_(),
    ep_(type, port),
    acceptor_(context_, ep_),
    log_(log)
    {
    }

    ~TcpServer_t() {
        log_.Info("Server Shutdown, joining threads\n");
        for (int i=0; i<threads_.size(); i++) {
            threads_[i].join();
        }
    }

    void Listen() {
        for (int i=0; i<5; i++) {
            std::shared_ptr<TcpConnection> con = std::make_shared<TcpConnection>(context_, log_);
            acceptor_.async_accept(con->GetSock(), std::bind(&TcpServer_t::acceptCallBack, this, con));
        }
        context_.run();

    }

};
