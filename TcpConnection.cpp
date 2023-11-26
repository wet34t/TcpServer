#include "TcpConnection.hpp"
#include <iomanip>

using namespace boost;

std::string TcpConnection::getHeaderType(unsigned *headerType, system::error_code &err) {
    if (headerType[0] == HELLO1 && headerType[1] == HELLO2) {
        return HELLO; // Make no assumptions whether all messages need to start with Hello
    } else if (headerType[0] == DATA1 && headerType[1] == DATA2) {
        return DATA;
    } else if (headerType[0] == GOODBYE1 && headerType[1] == GOODBYE2) {
        byeReceived_ = true;
        return GOODBYE; // Assumes when Goodbye is received, stop receiving messages
    } else { // Unsupported header
        err = system::errc::make_error_code(system::errc::bad_message);
        return "";
    }
}

void TcpConnection::readBytes(size_t len, system::error_code &err) {
    std::vector<unsigned char> buff(len);
    sock_.read_some(asio::buffer(buff.data(), len), err);
    // Potentially can parse data as needed, i.e parse(buff);
}

void TcpConnection::readBytesToHex(size_t len, unsigned *result, system::error_code &err) { // This is to dec technically
    std::vector<unsigned char> buff(len);
    sock_.read_some(asio::buffer(buff.data(), len), err);

    for (int i=0; i<len; i++) {
        result[i] = (unsigned)buff[i]; // cast it to unsigned
    }
}

bool TcpConnection::waitForBytes(size_t len) {
    time_t s = time(0);
    system::error_code err;
    size_t bytes = sock_.available(err);
    if (err) return false;
    while (bytes < len) {
        size_t next = sock_.available(err);
        if (err || (next == bytes && time(0) - s > TIMEOUT)) {
            // Been waiting too long without getting any data at all
            return false;
        } else if (next != bytes) {
            // Got some data, so reset timer
            bytes = next;
            s = time(0);
        }
    }
    return true;
}

std::string TcpConnection::processBody(size_t bodyLen) {
    system::error_code err;
    // Only process the first 4 bytes or entire body whichever is less
    int hexes = std::min(DATAPRINT, (int)bodyLen);
    if (!waitForBytes(hexes)) {
        handleError("Error waiting for body message");
        return "";
    }
    unsigned bodyHexes[hexes];
    readBytesToHex(hexes, bodyHexes, err);
    if (err) {
        handleError("Error occured reading body message");
        return "";
    }

    std::stringstream ret; 
    for (int i=0; i<hexes; i++) {
        // This just converts it to hex format as required
        ret << "0x" << std::hex << std::setfill('0') << std::setw(2) << bodyHexes[i];
        if (i < hexes-1) {
            ret << " ";
        } 
    }

    // Process the rest of the body, max MAXBATCHSIZE per batch 
    bodyLen -= hexes;
    int len = std::min(MAXBATCHSIZE, (int)bodyLen);
    while (len <= bodyLen) {
        if (!len) break;
        if (!waitForBytes(len)) {
            handleError("Error waiting for body message");
            return "";
        }
        readBytes(len, err);
        if (err) {
            handleError("Error occured reading rest of the body messages");
            return "";
        }
        bodyLen -= len;
        len = std::min(MAXBATCHSIZE, (int)bodyLen);
    }
    return ret.str();
}

std::string TcpConnection::processHeader(size_t *len) {     
    system::error_code err;
    // Process the type which is first 2 bytes
    if (!waitForBytes(TYPESIZE)) {
        handleError("Error waiting to read header type");
        return "";
    }
    unsigned headerType[TYPESIZE];
    readBytesToHex(TYPESIZE, headerType, err);
    if (err) {
        handleError("Error occured reading header type");
        return "";
    }
    std::string ret = "[";
    ret += getHeaderType(headerType, err);
    ret += "] [";
    if (err) {
        handleError("Error not supported header type");
        return "";
    }

    // Process the Length, which is next 4 bytes
    if (!waitForBytes(LENSIZE)) {
        handleError("Error waiting to read header length");
        return "";
    }
    unsigned headerLen[LENSIZE];
    readBytesToHex(LENSIZE, headerLen, err);
    if (err) {
        handleError("Error occured reading header length");
        return "";
    }
    // Convert the length into unsigned, and set *len, so we can start processing body
    // Max is ffffffff or 4294967295
    unsigned length = (headerLen[0] << 24) | (headerLen[1] << 16) | (headerLen[2] << 8) | headerLen[3];
    ret += std::to_string(length);
    ret += "]";
    *len = (size_t)length;
    return ret;
}

void TcpConnection::handleError(std::string message = "") {
    log_.Error("An error has occured for: " + remoteIpAddress_ + ", " + message);
    err_ = true;
    ipStats_->RegisterErrors(remoteIpAddress_);
    sock_.close();
}

void TcpConnection::ReadPackets() {
    while (!byeReceived_) { // Keep reading until header type Goodbye is received
        size_t bodyLen;
        std::string header = processHeader(&bodyLen);
        if (err_) break;
        std::string body = "";
        if (bodyLen) {
            body = processBody(bodyLen);
        }
        if (err_) break;
        // Finished reading entire TLV blob, print the results as required
        log_ << "[" << remoteIpAddress_ << ":" << remotePort_ << "] " << header << " [" << body << "]\n";
    }
    if (!err_) {
        ipStats_->RegisterSuccess(remoteIpAddress_);
        sock_.close();
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
}

TcpConnection::~TcpConnection() {
    // Potentially can print out some interesting stats as well
}