#include "TcpConnection.hpp"
#include "TcpServer.hpp"

using namespace boost;

int main()
{
    int port = 3333;
    logger log(std::cout);

    log << std::this_thread::get_id() << ": starting program\n";

    struct TcpServer_t *server = new TcpServer_t(asio::ip::tcp::v6(), port, log);
    server->Listen();

    delete server;

    return 0;
}
