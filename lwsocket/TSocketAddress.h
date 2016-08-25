#ifndef TSOCKETADDRESS_H
#define	TSOCKETADDRESS_H

#include "TBase.h"

extern "C" {
#include <stdint.h>
#include <string.h>
#ifdef LWIP_TCP
#include <lwip/netdb.h>
#else
#ifdef _MSC_VER
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include "netinet/in.h"
#include <netdb.h>
#endif
#endif
}

namespace LWSocket {

template<class TString>
class TIPAddress : public TBoolChecked<TIPAddress < TString> > {
    in_addr addr_;
public:
    enum Family {
        IPv4
    };
    TIPAddress(const void* addr, size_t length) {
        memcpy(&addr_, addr, sizeof(addr_));
    }
    ~TIPAddress() {}
    const void * addr() const {
        return &addr_;
    }
};

template <class TString>
class TSocketAddress : public TBoolChecked<TSocketAddress <TString> >,
    private TNoCopy<TSocketAddress <TString> > {
    sockaddr_in addr_;
    void init_() {
        memset(&addr_,0,sizeof(addr_));
        //addr_.sin_len = sizeof(addr_);
        addr_.sin_family = AF_INET;
    }
    void initfrom_(const TString &host, uint16_t port) {
        init_();
        hostent * ent = gethostbyname(host.c_str());
        if(ent && ent->h_length) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
            in_addr * in = (in_addr*)ent->h_addr_list[0];
#pragma GCC diagnostic pop
            addr_.sin_addr.s_addr = in->s_addr;
            addr_.sin_port = htons(port);
            this->setOk();
        }
    }
    //disabled constructor
    TSocketAddress();
public:
    TSocketAddress(const TString &host, uint16_t port) : addr_( {
        0
    } ) {
        initfrom_(host,port);
    }
    const sockaddr* addr() const {
        return (sockaddr*) &addr_;
    }
    size_t length() const {
        return sizeof(addr_);
    }
    uint16_t port() const {
        return ntohs(addr_.sin_port);
    }
    TIPAddress<TString> host() const {
        return TIPAddress<TString>(  &addr_.sin_addr, sizeof(addr_.sin_addr) );
    }
};

}

#endif	/* TSOCKETADDRESS_H */

