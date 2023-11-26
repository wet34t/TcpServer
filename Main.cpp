#include "TcpServer.hpp"

using namespace boost;

int main()      // TODO COMMAND LINE PORT
{
    uint_least16_t port = 3333;
    Logger log(std::cout, std::cout);

    log << std::this_thread::get_id() << ": starting program\n";

    try {
        asio::io_context context;
        
        std::unique_ptr<TcpServer> server = std::make_unique<TcpServer>(context, asio::ip::tcp::v6(), port, log);// TODO INADDR_ANY
        server->StartListen();
        context.run();
    } catch (std::exception& e)
    {
        log.Error("An Error has occured");
        log.Error(e.what());
    }

    return 0;
}
