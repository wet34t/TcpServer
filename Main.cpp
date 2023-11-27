#include "TcpServer.hpp"

using namespace boost;

// Compiled using
// g++ Main.cpp TcpConnection.cpp TcpServer.cpp -std=c++14
int main(int argc, char **argv)
{
    Logger log(std::cout, std::cerr);
    if (argc < 2) {
        log.Error("Port number required from command line");
        return 0;
    }
    uint_least16_t port;
    try {
        port = (uint_least16_t)std::stoi(argv[1]);
    } catch (std::invalid_argument) {
        log.Error("Require a valid port number from command line");
        return 0;
    }

    try {
        asio::io_context context;
        std::unique_ptr<TcpServer> server = std::make_unique<TcpServer>(context, asio::ip::tcp::v4(), port, log); // v4() for INADDR_ANY or v6() for in6addr_any
        server->StartListen();
        context.run(); // Could potentially run this on a separate thread, and main thread can do some other things.
    } catch (std::exception& e)
    {
        log.Error("An Error has occured");
        log.Error(e.what());
    }

    return 0;
}
