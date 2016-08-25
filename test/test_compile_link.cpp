#include "TSocketAddress.h"
#include "TSocket.h"
#include "testHelper.h"

typedef LWSocket::strbase stringClass;

#include "defaultTypedefs.h"

void demo() {
    SocketAddress a("www.ipify.org", 80 );
    StreamSocket sock( IPAddress::IPv4 );
}
