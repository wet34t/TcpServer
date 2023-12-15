#ifndef TCP_CONNECTION
#define TCP_CONNECTION

#include <string>
#include <vector>
#include <boost/asio.hpp>
#include "Logger.hpp"
#include "IpStatistics.hpp"

#define TYPESIZE 2
#define LENSIZE 4

using namespace boost;

class TcpConnection {
private:
    asio::ip::tcp::socket sock_; // Socket for this connection
    const Logger &log_; // Logger to log information, shared with Server and other connections
    std::shared_ptr<struct IpStatistics_t> ipStats_; // IpStatistics_t to gather stats, shared with Server and other connections
    std::string remoteIpAddress_; // From connection IP
    uint_least16_t remotePort_; // From connection port
    bool byeReceived_; // Whether Goodbye is receieved
    bool active_; // Whether connection is active
    bool err_; // If an error has occured
    const std::string HELLO = "Hello";
    const std::string DATA = "Data";
    const std::string GOODBYE = "Goodbye";
    const unsigned HELLO1 = 225; // e1
    const unsigned HELLO2 = 16; // 10
    const unsigned DATA1 = 218; // da
    const unsigned DATA2 = 122; // 7a
    const unsigned GOODBYE1 = 11; // 0b
    const unsigned GOODBYE2 = 30; // 1e
    const int TIMEOUT = 1000; // Max time to wait before existing if no messages arrive
    const int MAXBATCHSIZE = 10000; // Max message size to read at a time
    const int DATAPRINT = 4;

    /*
    * Get the header type (i.e Hello, Data, Goodbye) based on msg
    *
    * param - unsigned * msg: the msg containing the header
    * param - error_code & err: update err if error occurs
    * 
    * return std::string - the header type
    */
    std::string getHeaderType(unsigned *msg, system::error_code &err);

    /*
    * Read bytes using asio
    *
    * param - size_t: the msg size to be read
    * param - error_code & err: update err if error occurs
    */
    void readBytes(size_t, system::error_code &err);

    /*
    * Read bytes and convert it to unsigned array
    *
    * param - size_t: the msg size to be read
    * param - unsigned *arr: store the parsed result into arr
    * param - error_code & err: update err if error occurs
    */
    void readBytesToHex(size_t, unsigned *arr, system::error_code &);

    /*
    * Wait for bytes to be available before reading, timeout if no bytes received for too long
    *
    * param - size_t: the size required to wait for
    * 
    * return bool - wheather the wait was successful
    */
    bool waitForBytes(size_t);

    /*
    * Process the body which is the value of variable length
    *
    * param - size_t: the size of the body that requires processing
    * 
    * return std::string - the string summarizing the body
    */
    std::string processBody(size_t);

    /*
    * Process the header including type and length, find the length and summarize header
    *
    * param - size_t *: pointer to be updated with the actual length of the value
    * 
    * return std::string - the string summarizing the header to be printed
    */
    std::string processHeader(size_t *);

    /*
    * This function is called when an error happens
    *
    * param - std::string msg: the msg containing the error
    */
    void handleError(std::string msg);

public:
    /*
    * class TcpConnection - Constructor
    * Class to handle a TCP connection
    * 
    * param - asio::ip::tcp::socket: socket associated with this connection
    * param - const Logger&: the logger used to log information and errors
    * param - std::shared_ptr<struct IpStatistics_t>: A shared pointer of IpStatistics_t for gather stats
    */
    TcpConnection(asio::ip::tcp::socket, const Logger &, std::shared_ptr<struct IpStatistics_t>);

    /*
    * Delete the copy and assignment constructors because we probably don't want to copy a TcpConnection
    */
    TcpConnection(const TcpConnection&) = delete;
    TcpConnection& operator=(const TcpConnection&) = delete;

    /*
    * class TcpConnection - Destructor
    * Destroy this TcpConnection
    */
    ~TcpConnection();

    /*
    * Allow this connection to start reading the packets
    */
    void ReadPackets();

    /*
    * Whether the connection is still active
    *
    * return bool - connection is active or not
    */
    bool IsActive() const;
};

#endif