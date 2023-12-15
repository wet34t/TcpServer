#include <thread>
#include <chrono>
#include <memory>
#include <queue>
#include "TcpConnection.hpp"

#define MAXCONNECTIONS 50 // Max concurrent connections; probably should be based on specs

class ConnectionsQueue {
private:
    std::queue<std::shared_ptr<TcpConnection>> connectionsQueue_;
    std::mutex m;

public:
    ~ConnectionsQueue();
    bool Push(std::shared_ptr<TcpConnection>);
    std::shared_ptr<TcpConnection> Pop();
    int Size();
    void Clear();
};

class ConnectionsThread {
private:
    bool active_;
    std::thread thread_;
    std::shared_ptr<ConnectionsQueue> connections_;

    void run();

public:
    ConnectionsThread(std::shared_ptr<ConnectionsQueue>);
    ~ConnectionsThread();
    void Join();
};