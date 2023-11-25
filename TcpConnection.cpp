#include "TcpConnection.hpp"

using namespace boost;

std::string TcpConnection::read(size_t len) {
    std::vector<char> buff(len);
    system::error_code err;
    sock_.read_some(asio::buffer(buff.data(), len), err);
    std::string str(buff.begin(), buff.end());
    if (!err) {
        return str;
    }
    endConnection_ = true;
    return "";
}

bool TcpConnection::waitForResult(size_t len) {
    time_t s = time(0);
    size_t bytes = sock_.available();
    int messages = 1;
    while (bytes < len) {
        size_t next = sock_.available();
        if (next == bytes && time(0) - s > timeout) return false;
        else if (next == bytes && time(0) - s > messages) {
            log_ << "Waiting..." << messages << "\n";
            messages++;
        }
        else if (next != bytes) {
            bytes = next;
            s = time(0);
        }
        
    }
    return true;
}

void TcpConnection::processBody(size_t bodyLen) {
    if (!waitForResult( bodyLen)) {
        log_.Error("An error has occured");
        return;
    }
    std::string result = read(bodyLen);
    log_ << "Obtained body result " << result << "\n";
}

size_t TcpConnection::processHeader() {
    if (!waitForResult(TYPESIZE)) {
        log_.Error("An error has occured");
        endConnection_ = true;
        return 0;
    }

    std::string result = read(TYPESIZE);
    if (result == GOODBYEHEADER) endConnection_ = true;
    log_ << "Obtained header result " << result << "\n";

    if (!waitForResult(LENSIZE)) {
        log_.Error("An error has occured");
        endConnection_ = true;
        return 0;
    }

    result = read(LENSIZE);
    log_ << "Obtained len result " << result << "\n";
    try {
        int num = std::stoi(result);
        return (size_t)num;
    } catch (std::exception& e) {
        std::string errMessage = "Error occured converting str to num";
        log_.Error(errMessage + e.what());
        endConnection_ = true;
        return 0;
    }
}

asio::ip::tcp::socket& TcpConnection::GetSock() {
    return sock_;
}

void TcpConnection::ReadPackets() {
    log_ << std::this_thread::get_id() << ": connection from " << sock_.local_endpoint().address() << " " << sock_.local_endpoint().port() << " to ";
    log_ << sock_.remote_endpoint().address() << " " << sock_.remote_endpoint().port() << " established at " << time(0) - start_ << "\n";
    while (!endConnection_) {
        size_t bodyLen = processHeader();
        if (bodyLen)
            processBody(bodyLen);
    }
    log_ << "Closing connection at " << time(0) - start_ << "\n";
    sock_.close();
}

TcpConnection::TcpConnection(boost::asio::io_context& context, const logger &log) :
sock_(context),
start_(time(0)),
log_(log),
endConnection_(false)
{
    log_.Info("New Connection created");
}