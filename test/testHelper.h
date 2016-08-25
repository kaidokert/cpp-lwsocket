#ifndef TESTHELPER_H
#define	TESTHELPER_H

namespace LWSocket {

struct strbase {
    const char *cs;
    strbase(const char *c) : cs(c) {
    }
    const char * c_str() const {
        return cs;
    }
};

}

#endif	/* TESTHELPER_H */

