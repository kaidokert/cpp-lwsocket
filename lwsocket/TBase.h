#ifndef TBASE_H
#define	TBASE_H

namespace LWSocket {

template <class TSelf>
class TNoCopy {
    TNoCopy(const TNoCopy &);
    TNoCopy & operator = (const TNoCopy &);
protected:
    TNoCopy() {}
};

template <class TSelf>
struct TBoolChecked {
    bool good() {
        return good_;
    }
    bool bad() {
        return !good_;
    }
private:
    bool good_;
protected:
    TBoolChecked() : good_(false) {
    }
    void setStatus(bool set) {
        good_ = set;
    }
    void setOk() {
        good_ = true;
    }
    void setFailed() {
        good_= false;
    }
};

}

#endif	/* TBASE_H */

