#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <chrono>
#include <thread>

void sendMessage(boost::asio::ip::tcp::socket &sock, std::string message, int n) {
    for (int i=0; i<n; i++) {
        boost::asio::write(sock, boost::asio::buffer(message));
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void connect() {
    try {
        using namespace std::string_literals;
        boost::system::error_code err;
        boost::asio::io_context context;
        boost::asio::ip::tcp::socket socket(context);
        boost::asio::ip::tcp::endpoint ep(boost::asio::ip::make_address("192.168.0.167"), 31415); // Some hard coded test data
        socket.connect(ep, err);
        if (err) {
            std::cout << "An error has occured";
            return;
        }
        std::cout << "Connection from " << socket.local_endpoint().address() << " " << socket.local_endpoint().port() << " to " << socket.remote_endpoint().address() << " " << socket.remote_endpoint().port() << " established" << std::endl;
        const unsigned char Hello1 = 225, Hello2 = 16;
        const unsigned char Data1 = 218, Data2 = 122;
        const unsigned char Goodbye1 = 11, Goodbye2 = 30;
        std::string Hello = "", Data = "", Goodbye = "";
        Hello += Hello1;
        Hello += Hello2;
        Data += Data1;
        Data += Data2;
        Goodbye += Goodbye1;
        Goodbye += Goodbye2;
        std::string Length1 = "\0\0\0"s; // Use string literals to add '0'
        std::string Length2 = "\0"s;
        Length1 += (char)1;
        Length2 += (char)3;
        Length2 += (char)247;
        Length2 += (char)160;

        sendMessage(socket, Hello+Length1+"8", 3);
        std::this_thread::sleep_for(std::chrono::milliseconds(7000));
        sendMessage(socket, Data, 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(7000));
        sendMessage(socket, Length2, 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(7000));
        sendMessage(socket, "abcdefghijklmnopqrstuvwxyz", 10000); // This usually takes a long time
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        sendMessage(socket, Goodbye+Length1+"J", 1);

        /* 
        Example output:
        [192.168.0.147:60908] [Hello] [1] [0x38]
        [192.168.0.147:60908] [Hello] [1] [0x38]
        [192.168.0.147:60908] [Hello] [1] [0x38]
        [192.168.0.147:60908] [Data] [260000] [0x61 0x62 0x63 0x64]
        [192.168.0.147:60908] [Goodbye] [1] [0x4a]
        */
    } catch (std::exception& e)
    {
        std::cout << "error occured " << e.what() << std::endl;
    }
}

int main() {
    connect();

    return 1;
}