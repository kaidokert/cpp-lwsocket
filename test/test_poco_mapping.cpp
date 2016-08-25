#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <cassert>
#include <stdint.h>
#include <fcntl.h>

#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/Exception.h"

#include <netdb.h>

#include "TSocketAddress.h"
#include "TSocket.h"
#include "testHelper.h"

using namespace std;
using Poco::Net::SocketAddress;
using Poco::Net::StreamSocket;
using Poco::Net::IPAddress;

struct wrapSocketAddress : public SocketAddress {
    wrapSocketAddress(const char *host, int port) : SocketAddress(host, port) {}
    bool good() {
        return true;
    }
};

struct wrapStreamSocket : public StreamSocket {
    wrapStreamSocket(wrapSocketAddress &a) : StreamSocket(a) {}
    wrapStreamSocket(Poco::Net::IPAddress::Family fam) : StreamSocket(fam) {}
    bool good() {
        return true;
    }
};

void dump_in_addr(const char * f, const in_addr *a) {
    printf("--in_addr--\n");
    printf("%s: in_addr=0x%08X\n", f, htonl(a->s_addr) );
    uint8_t * p =  (uint8_t *)& a->s_addr;
    printf("%s: IP %d.%d.%d.%d\n",f,p[0],p[1],p[2],p[3]);
}

void dump_sockaddr_in(const char * f , const sockaddr_in *a) {
    printf("--sockaddr_in--\n");
    printf("%s: sin_len =% d\n", f, a->sin_len);
    printf("%s: sin_family = %d\n",f, a->sin_family);
    printf("%s: sin_port = %d\n", f,a->sin_port);
    printf("%s: sin_zero = %d\n", f, a->sin_zero[0]);
    dump_in_addr(f, &a->sin_addr);
}
void dump_sockaddr(const char *f, const sockaddr *a) {
    printf("--sockaddr--\n");
    printf("%s: sa_len = %d\n",f,a->sa_len);
    printf("%s: sa_family = %d\n",f,a->sa_family);
}

void errMsg(const char *msg) {
    std::cerr << "Failed " << msg << std::endl;
}

template<class TSocketAddress, class TStreamSocket, class TAddrType>
void _demoHttpConnect2(const char *host, int port, const char *ep) {
    std::cout << "Trying host:" << host << " port:" << port << " ep:" << ep << std::endl;
    TSocketAddress sa(host, port );
    if(!sa.good())
        return errMsg("failed socketaddress");
    printf("sa.port=%d\n\n",sa.port() );
    std::cout << "creating socket" << std::endl;
    //TStreamSocket socket(sa);
    TStreamSocket socket( TAddrType::IPv4 );
    if(!socket.good())
        return errMsg("failed socket()");
    std::cout << "socket created" << std::endl;
    socket.connect(sa, 3 * 500 * 1000);
    std::cout << "done connecting socket" << std::endl;
    if(!socket.good())
        return errMsg("failed connect");
    std::ostringstream oss;

    oss << "GET " << ep << " HTTP/1.1\r\n"
        "Host: " << host << "\r\n"
        "\r\n";
    std::string str = oss.str();
    std::cout << "write:" << str;
    const char *bytes = str.c_str();
    int len = (int)str.length();
    int sent = socket.sendBytes(bytes, len );
    if(!socket.good())
        return errMsg("failed sendBytes");
    char buffer[512];
    memset(buffer, 0, sizeof(buffer));
    int recvd = socket.receiveBytes(buffer, sizeof(buffer));
    if(!socket.good())
        return errMsg("failed receiveBytes");
    std::cout << "buffer: " << buffer << std::endl;
}

typedef LWSocket::TSocketAddress<LWSocket::strbase> mySocketAddress;
typedef LWSocket::TIPAddress<LWSocket::strbase> myIPAddress;
using LWSocket::TStreamSocket;

void demoHttpConnect2(const char *host, int port, const char *ep) {
    printf("---------myconnect------------\n");
    _demoHttpConnect2<mySocketAddress, TStreamSocket<mySocketAddress, myIPAddress::Family> , myIPAddress>(host, port , ep);
    printf("---------wrapconnect------------\n");
    _demoHttpConnect2<wrapSocketAddress, TStreamSocket<wrapSocketAddress, myIPAddress::Family> , myIPAddress >(host, port, ep);
    printf("---------originalconnect------------\n");
    _demoHttpConnect2<wrapSocketAddress, wrapStreamSocket, IPAddress >(host, port, ep);
}

int apiConnect(const char *host,int port, const char *ep) {
    try {
        demoHttpConnect2(host, port, ep);
    } catch (Poco::Exception &pe) {
        std::cout << "PocoException: " << pe.displayText() << std::endl;
    } catch (std::exception &e) {
        std::cout << "exception:" << e.what() << std::endl;
        throw;
    } catch (...) {
        std::cout << "Ellipsis exception" << std::endl;
        throw;
    }
}

int main(int argc, char** argv) {
    apiConnect( "www.ipify.org",  80 , "?format=json");
#if 1
    apiConnect("demo.nonexistent.com",  80 , "?format=json");
    apiConnect( "www.ipify.org", 801 , "?format=json");
    apiConnect( "www.ipify.org",  80 , "/thiswouldnotexist/");
#endif
    return 0;
}
