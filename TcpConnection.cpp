#include "TcpConnection.hpp"

using namespace boost;

std::string TcpConnection::getHeaderType(unsigned *headerType, system::error_code &err) {
    if (headerType[0] == 225 && headerType[1] == 16) {
        std::cout << "Header is Hello" << std::endl;
        return "Hello";     // Do all messages need a Hello first?
    } else if (headerType[0] == 218 && headerType[1] == 122) {
        std::cout << "Header is Data" << std::endl;
        return "Data";
    } else if (headerType[0] == 11 && headerType[1] == 30) {
        std::cout << "Header is Goodbye" << std::endl;
        byeReceived_ = true;
        return "Goodbye";   // Presumably ending connection
    } else {
        err = boost::system::errc::make_error_code(boost::system::errc::bad_message);
        return "";
    }
}

void TcpConnection::readBytes(size_t len, system::error_code &err) {
    std::vector<unsigned char> buff(len);
    sock_.read_some(asio::buffer(buff.data(), len), err);
}

void TcpConnection::readBytesToHex(size_t len, unsigned *result, system::error_code &err) {
    std::vector<unsigned char> buff(len);
    sock_.read_some(asio::buffer(buff.data(), len), err);

    for (int i=0; i<len; i++) {
        result[i] = (unsigned)buff[i];  // To dec
        if (result[i] > 255) {  // Overflow
            err = boost::system::errc::make_error_code(boost::system::errc::bad_message);
            return;
        }
    }
}

bool TcpConnection::waitForBytes(size_t len) {
    time_t s = time(0);
    system::error_code err;
    size_t bytes = sock_.available(err);
    if (err) return false;
    while (bytes < len) {
        size_t next = sock_.available(err);
        if (err || (next == bytes && time(0) - s > TIMEOUT)) return false;
        else if (next != bytes) {
            bytes = next;
            s = time(0);
        }
    }
    return true;
}

void TcpConnection::processBody(size_t bodyLen) {
    system::error_code err;
    if (!waitForBytes(bodyLen)) {
        handleError("Error waiting for body message");
        return;
    }
    int hexes = std::min(4, (int)bodyLen);
    unsigned bodyHexes[hexes];
    readBytesToHex(hexes, bodyHexes, err);
    if (err) {
        handleError("Error occured reading body message");
    }
    bodyLen -= hexes;
    std::cout << "Got header type " << bodyHexes[0] << " " << bodyHexes[1] << " " << bodyHexes[2] << " " << bodyHexes[3] << std::endl; // TODO better print
    // << std::hex << std::setfill ('0') << std::setw(sizeof(your_type)*2) 
    int len = std::min(MAXBATCHSIZE, (int)bodyLen);
    while (len <= bodyLen) {
        if (!len) break;
        readBytes(len, err);
        if (err) {
            handleError("Error occured reading rest of the body messages");
            return;
        }
        bodyLen -= len;
        len = std::min(MAXBATCHSIZE, (int)bodyLen);
    }
}

size_t TcpConnection::processHeader() {     
    system::error_code err;
    if (!waitForBytes(TYPESIZE)) {
        handleError("Error waiting to read header type");
        return 0;
    }

    unsigned headerType[TYPESIZE];
    readBytesToHex(TYPESIZE, headerType, err);
    std::cout << "Got header type " << headerType[0] << " " << headerType[1] << std::endl;
    if (err) {
        handleError("Error occured reading header type");
        return 0;
    }
    std::string result = getHeaderType(headerType, err);
    if (err) {
        handleError("Error not supported header type");
        return 0;
    }

    if (!waitForBytes(LENSIZE)) {
        handleError("Error waiting to read header length");
        return 0;
    }

    unsigned headerLen[LENSIZE];
    readBytesToHex(LENSIZE, headerLen, err);
    if (err) {
        handleError("Error occured reading header length");
        return 0;
    }
    unsigned length = (headerLen[0] << 24) | (headerLen[1] << 16) | (headerLen[2] << 8) | headerLen[3];
    std::cout << "Got header length " << length << std::endl;
    return (size_t)length;
}

void TcpConnection::handleError(std::string message = "") {
    sock_.close();
    log_.Error("An error has occured for: " + remoteIpAddress_ + ", " + message);
    err_ = true;
    ipStats_->RegisterErrors(remoteIpAddress_);
}

void TcpConnection::ReadPackets() {
    while (!byeReceived_) {
        size_t bodyLen = processHeader();
        if (err_) break;
        if (bodyLen) processBody(bodyLen);
        if (err_) break;
    }
    if (!err_) {
        sock_.close();
        ipStats_->RegisterSuccess(remoteIpAddress_);
    }
    active_ = false;
}

bool TcpConnection::IsActive() const {
    return active_;
}

TcpConnection::TcpConnection(asio::ip::tcp::socket socket, const Logger &log, std::shared_ptr<struct IpStatistics_t> ipStats) :
sock_(std::move(socket)),
log_(log),
ipStats_(ipStats),
remoteIpAddress_(sock_.remote_endpoint().address().to_string()),
remotePort_(sock_.remote_endpoint().port()),
byeReceived_(false),
active_(true),
err_(false)
{
    log_.Info("New Connection created for " + remoteIpAddress_);
}

TcpConnection::~TcpConnection() {
    log_.Info("Closing connection for " + remoteIpAddress_);
}