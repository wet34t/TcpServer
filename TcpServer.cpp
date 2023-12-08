#include "TcpServer.hpp"

using namespace boost;

void TcpServer::acceptHandler(system::error_code err, asio::ip::tcp::socket socket) {
    std::string remoteIp = socket.remote_endpoint().address().to_string();
    ipStats_->RegisterConnection(remoteIp);
    if (!acceptConnection(remoteIp, socket.remote_endpoint().port())) {
        startListen();
        return;
    }
    if (err) {
        ipStats_->RegisterErrors(remoteIp);
        log_.Error("An Error has occured");
        startListen();
        return;
    }

    // Create a new TcpConnection and let it handle read on a new thread
    // Could also consider using asio::ip::tcp::socket::async_read_some instead
    std::shared_ptr<TcpConnection> con = std::make_shared<TcpConnection>(std::move(socket), log_, ipStats_);
    while (!connectionsQueue_->Push(con)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    }

    totalConnections_++;
    startListen();
}

bool TcpServer::acceptConnection(std::string &ipAddress, const uint_least16_t port) const {
    return ipStats_->GetErrors(ipAddress) <= MAXALLOWEDERRORS;
    // This function has potential to be enhanced, currently just checks if this IP has too many previous errors
}

void TcpServer::startListen() {
    if (alive_) acceptor_.async_accept(std::bind(&TcpServer::acceptHandler, this, std::placeholders::_1, std::placeholders::_2));
}

void TcpServer::StartServer() {
    alive_ = true;
    for (int i=0; i<MAXTHREADS; i++) {
        std::unique_ptr<ConnectionsThread> t = std::make_unique<ConnectionsThread>(connectionsQueue_);
        threads_.push_back(std::move(t));
    }

    startListen();
}

void TcpServer::StopServer() {
    alive_ = false;
    for (auto it = threads_.begin(); it != threads_.end(); it++ ) {
        (*it)->Join();
    }
    threads_.clear();
    connectionsQueue_->Clear();
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
totalConnections_(0),
alive_(true) {
    ipStats_ = std::make_shared<IpStatistics_t>();
    connectionsQueue_ = std::make_shared<ConnectionsQueue>();
}

TcpServer::~TcpServer() {
    StopServer();
    // Potentially can print out some interesting stats as well.
}
