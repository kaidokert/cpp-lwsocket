#ifndef TSOCKET_H
#define	TSOCKET_H

#include "TBase.h"

extern "C" {

#ifdef LWIP_TCP
#undef connect
#define system_connect lwip_connect
#define system_close close
#else
#ifdef _MSC_VER
#include <Winsock2.h>
#define system_connect connect
#define system_close closesocket
#else
#include <fcntl.h>
#include <sys/select.h>
#include <sys/errno.h>
#include <unistd.h>
#define system_connect connect
#define system_close close
#ifdef __DARWIN_C_LEVEL
#include <netinet/tcp.h>
#endif
#endif
#endif
}

namespace LWSocket {

#ifdef _MSC_VER
#define SOCK_EINTR			WSAEINTR
#define SOCK_EINVAL			WSAEINVAL
#define SOCK_EWOULDBLOCK	WSAEWOULDBLOCK
#define SOCK_EINPROGRESS	WSAEINPROGRESS
#else
#define SOCK_EINTR          EINTR       //4
#define SOCK_EINVAL         EINVAL      //22
#define SOCK_EWOULDBLOCK    EWOULDBLOCK //35
#define SOCK_EINPROGRESS    EINPROGRESS //36
#endif

template<class TSocketAddress, class TIPAddressFamily >
class TStreamSocket : public TBoolChecked<TStreamSocket <TSocketAddress, TIPAddressFamily > >,
    private TNoCopy< TStreamSocket <TSocketAddress, TIPAddressFamily >  > {
    typedef unsigned long int timeType;
    int socket_;
    timeType recvTimeout_;
    void setBlocking_(bool flag) {
#if _MSC_VER
        unsigned long flags = flag ? 0 : 1;
        ioctlsocket(socket_,FIONBIO,&flags);
#else
        int arg = fcntl(socket_,F_GETFL,0);
        long flags = arg & ~O_NONBLOCK;
        if (!flag) flags |= O_NONBLOCK;
        fcntl(socket_,F_SETFL, flags);
#endif
    }
    enum poll_ops {
        POLL_READ = 1,
        POLL_WRITE = 2,
        POLL_ERROR = 4,
        POLL_READ_AND_ERROR = 5,
        POLL_ALL = 7
    };
    bool poll_(timeType microseconds, poll_ops op = POLL_ALL ) {
        fd_set read,write,except;
        FD_ZERO(&read);
        FD_ZERO(&write);
        FD_ZERO(&except);
        if (op & POLL_READ)
            FD_SET(socket_,&read);
        if (op & POLL_WRITE)
            FD_SET(socket_,&write);
        if (op & POLL_ERROR)
            FD_SET(socket_,&except);
        timeval tv = {  0, 0  };
        tv.tv_sec  = microseconds / 1000000;
        tv.tv_usec = microseconds % 1000000;
        int sockfd = socket_ + 1;
        int res = ::select( sockfd ,&read,&write,&except, &tv  );
        return res > 0;
    }
    void connect_(TSocketAddress &a, timeType microseconds) {
        this->setFailed();
        setBlocking_(false);
        int res = ::system_connect(socket_, a.addr(), a.length());
        if(res != 0) {
            int err = lastError();
            if(err!=SOCK_EINPROGRESS && err!=SOCK_EWOULDBLOCK)
                return;
            if (!poll_(microseconds)) //timeout
                return;
            err = socketError();
            if (err)
                return;
        }
        setBlocking_(true);
        this->setOk();
    }
    void connect_(TSocketAddress &a) {
        int res = ::system_connect(socket_, a.addr(), a.length());
        this->setStatus(res >= 0);
    }
    void createSocket_() {
        socket_ = ::socket(AF_INET, SOCK_STREAM, 0);
        recvTimeout_ = 0;
        this->setStatus( socket_ != -1 );
    }
    void getOption_inner_(int level,int option,void * value, socklen_t length) {
        ::getsockopt(socket_,level,option, (char *) value, &length);
    }
    int setOption_inner_(int level, int option, void *value, socklen_t length) {
        return ::setsockopt(socket_,level,option, (char *) value, length);
    }
    template<typename T>
    void getOption_(int level, int option, T & value) {
        socklen_t sz = sizeof(T);
        getOption_inner_(level,option, &value , sz );
    }
    enum {
        T_INVALID_SOCKET = -1
    };
    int get_errno() {
#ifdef _MSC_VER
        return  WSAGetLastError();
#else
        return errno;
#endif
    }
public:
    template<typename T>
    int setOption_(int level, int option, T value) {
        socklen_t sz = sizeof(T);
        return setOption_inner_(level,option, &value, sz );
    }
    int lastError() {
        return get_errno();
    }
    int socketError() {
        int result(0);
        getOption_(SOL_SOCKET, SO_ERROR, result);
        return result;
    }
    TStreamSocket(TSocketAddress &a) :
        socket_(T_INVALID_SOCKET), recvTimeout_(0) {
        createSocket_();
        connect_(&a);
    }
    TStreamSocket(TIPAddressFamily ) :
        socket_(T_INVALID_SOCKET), recvTimeout_(0) {
        createSocket_();
    }
    ~TStreamSocket() {
        if (socket_ != T_INVALID_SOCKET) {
            ::system_close(socket_);
        }
    }
    void connect(TSocketAddress &a, timeType microseconds) {
        connect_(a, microseconds);
    }
    int sendBytes(const void *buffer, size_t len) {
        const char *buff = (const char *)buffer;
        int res = ::send(socket_, buff, len , 0);
        this->setStatus( res > 0);
        return res;
    }
    void setReceiveTimeout(const timeType microseconds) {
        recvTimeout_ = microseconds;
    }
    int receiveBytes(void *buffer, size_t len) {
        if (recvTimeout_) {
            if (!poll_(recvTimeout_, POLL_READ_AND_ERROR )) {
                this->setFailed();
                return 0;
            }
        }
        char *buff = (char *)buffer;
        int res = ::recv(socket_, buff, len, 0);
        this->setStatus( res > 0);
        return res;
    }
};

}

#endif	/* TSOCKET_H */

