#include "TcpServer.hpp"

using namespace boost;

void TcpServer::clearConnections() {
    for (auto it = connectionThreads_.begin(); it != connectionThreads_.end(); ) {
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
    log_.Info("New connection from " + remoteIp);
    log_ << "This Ip has previously connected " << ipStats_->GetConnection(remoteIp) << " times with " << ipStats_->GetSuccess(remoteIp) << " success, and " << ipStats_->GetErrors(remoteIp) << " errors.\n";
    ipStats_->RegisterConnection(remoteIp);
    if (!acceptConnection(remoteIp, socket.remote_endpoint().port())) {
        log_.Info("Blocking known malicious Ip " + remoteIp);
        StartListen();
        return;
    }
    if (err) {
        ipStats_->RegisterErrors(remoteIp);
        log_.Error("An Error has occured");
        StartListen();
        return;
    }

    std::shared_ptr<TcpConnection> con = std::make_shared<TcpConnection>(std::move(socket), log_, ipStats_);
    std::thread t(&TcpConnection::ReadPackets, con);
    connectionThreads_.push_back(std::pair <std::thread,std::shared_ptr<TcpConnection>>(std::move(t), con));
    totalConnections_++;

    time_t s = time(0);
    unsigned interval = 0;
    while (!(connectionThreads_.size() < maxConnections)) {
        if (time(0) - s > interval) {
            log_.Info("Reached maximum allowed concurrent connections, Attempting to clear connections");
            interval += 5;
        }
        clearConnections();
    }
    if (connectionThreads_.size() < maxConnections && totalConnections_ < 6)
        StartListen();
}

bool TcpServer::acceptConnection(std::string &ipAddress, uint_least16_t port) {
    return ipStats_->GetErrors(ipAddress) <= maxErrorsBeforeBlock;
}

void TcpServer::StartListen() {
    acceptor_.async_accept(std::bind(&TcpServer::acceptHandler, this, std::placeholders::_1, std::placeholders::_2));
}

unsigned TcpServer::GetTotalConnections() const {
    return totalConnections_;
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
    log_.Info("Joining threads");
    for (auto it = connectionThreads_.begin(); it != connectionThreads_.end(); it++) {
        it->first.join();
    }
    connectionThreads_.clear();
    log_ << "Server on for " << ipStats_->GetTimeAlive() << " seconds\n";
    log_.Info("Server shutdown");
    
}
