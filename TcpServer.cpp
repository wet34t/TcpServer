#include "TcpServer.hpp"

using namespace boost;

void TcpServer::clearConnections() {
    // The current design is to clear stale connections when too many active connections
    // This function could be called periodically by the Server to clear stale connections
    // Or a connection could notify the server that its done.
    for (auto it = connectionThreads_.begin(); it != connectionThreads_.end(); ) {
        // Clear connections if no longer active
        if (!it->second->IsActive()) {
            it->first.join();
            it = connectionThreads_.erase(it);
        } else {
            it++;
        }
    }
}

void TcpServer::acceptHandler(system::error_code err, asio::ip::tcp::socket socket) {
    std::string remoteIp = socket.remote_endpoint().address().to_string();
    ipStats_->RegisterConnection(remoteIp);
    if (!acceptConnection(remoteIp, socket.remote_endpoint().port())) {
        StartListen();
        return;
    }
    if (err) {
        ipStats_->RegisterErrors(remoteIp);
        log_.Error("An Error has occured");
        StartListen();
        return;
    }

    // Create a new TcpConnection and let it handle read on a new thread
    // Could also consider using asio::ip::tcp::socket::async_read_some instead
    std::shared_ptr<TcpConnection> con = std::make_shared<TcpConnection>(std::move(socket), log_, ipStats_);
    std::thread t(&TcpConnection::ReadPackets, con);
    connectionThreads_.push_back(std::pair <std::thread,std::shared_ptr<TcpConnection>>(std::move(t), con));
    totalConnections_++;

    while (!(connectionThreads_.size() < MAXCONNECTIONS)) { // Too many connections, try to clear some stale ones
        clearConnections();
        if (connectionThreads_.size() < MAXCONNECTIONS) break;
        // Still no space, wait a while and try again
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    }
    if (connectionThreads_.size() < MAXCONNECTIONS)
        StartListen();
}

bool TcpServer::acceptConnection(std::string &ipAddress, const uint_least16_t port) const {
    return ipStats_->GetErrors(ipAddress) <= MAXALLOWEDERRORS;
    // This function has potential to be enhanced, currently just checks if this IP has too many previous errors
}

void TcpServer::StartListen() {
    acceptor_.async_accept(std::bind(&TcpServer::acceptHandler, this, std::placeholders::_1, std::placeholders::_2));
}

unsigned TcpServer::GetTotalConnections() const {
    return totalConnections_;
}

time_t TcpServer::GetStartTime() const {
    return ipStats_->GetStartTime();
}

TcpServer::TcpServer(asio::io_context &context, asio::ip::tcp type, uint_least16_t port, Logger &log) :
context_(context),
ep_(type, port),
acceptor_(context_, ep_),
log_(log),
totalConnections_(0)
{
    ipStats_ = std::make_shared<IpStatistics_t>();
}

TcpServer::~TcpServer() {
    for (auto it = connectionThreads_.begin(); it != connectionThreads_.end(); it++) {
        it->first.join();
    }
    connectionThreads_.clear();
    // Potentially can print out some interesting stats as well.
}
