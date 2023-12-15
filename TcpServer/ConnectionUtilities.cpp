#include "ConnectionUtilities.hpp"

bool ConnectionsQueue::Push(std::shared_ptr<TcpConnection> con) {
    std::lock_guard<std::mutex> l(m);
    if (connectionsQueue_.size() >= MAXCONNECTIONS) {
        return false;
    } else {
        connectionsQueue_.push(std::move(con));
        return true;
    }
}

std::shared_ptr<TcpConnection> ConnectionsQueue::Pop() {
    std::lock_guard<std::mutex> l(m);
    if (connectionsQueue_.size()) {
        std::shared_ptr<TcpConnection> con = connectionsQueue_.front();
        connectionsQueue_.pop();
        return con;
    } else {
        return nullptr;
    }
}

int ConnectionsQueue::Size() {
    std::lock_guard<std::mutex> l(m);
    return connectionsQueue_.size();
}

void ConnectionsQueue::Clear() {
    std::lock_guard<std::mutex> l(m);
    connectionsQueue_ = std::queue<std::shared_ptr<TcpConnection>>();
}

ConnectionsQueue::~ConnectionsQueue() {
    Clear();
}

void ConnectionsThread::run() {
    while (active_) {
        std::shared_ptr<TcpConnection> con = connections_->Pop();
        if (con)
            con->ReadPackets();
        else std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void ConnectionsThread::Join() {
    active_ = false;
    if (thread_.joinable()) {
        thread_.join();
    }
}

ConnectionsThread::ConnectionsThread(std::shared_ptr<ConnectionsQueue> connections) :
active_(true),
connections_(connections),
thread_(&ConnectionsThread::run, this) {
}

ConnectionsThread::~ConnectionsThread() {
    Join();
}
