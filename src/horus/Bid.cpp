#include <algorithm>

#include "Bid.h"

Bid::Bid() {
    std::fill(id.begin(),id.end(),0);
}

Bid::Bid(string value) {
    std::fill(id.begin(),id.end(),0);
    std::copy(value.begin(), value.end(), id.begin());
}

Bid::~Bid() {
}

Bid::Bid(int value) {
    std::fill(id.begin(),id.end(),0);
    auto arr = to_bytes(value);
    std::copy(arr.begin(), arr.end(), id.begin());
}

Bid::Bid(std::array<byte_t, ID_SIZE> value) {
    std::copy(value.begin(), value.end(), id.begin());
}

Bid Bid::operator++() {
    id[ID_SIZE - 1]++;
    for (int i = ID_SIZE - 1; i > 0; i--) {
        if (id[i] == 0) {
            id[i - 1]++;
        } else {
            break;
        }
    }
}

Bid& Bid::operator=(int other) {
    for (int i = 0; i < 4; i++) {
        id[3 - i] = (other >> (i * 8));
    }
    std::fill(id.begin()+4,id.end(),0);
}

bool Bid::operator!=(const int rhs) const {
    for (int i = 0; i < 4; i++) {
        if (id[3 - i] != (rhs >> (i * 8))) {
            return true;
        }
    }
    return false;
}

bool Bid::operator!=(const Bid rhs) const {
    for (int i = 0; i < 4; i++) {
        if (id[i] != rhs.id[i]) {
            return true;
        }
    }
    return false;
}

bool Bid::operator<(const Bid& b)const {
    for (int i = 0; i < ID_SIZE; i++) {
        if (id[i] < b.id[i]) {
            return true;
        } else if (id[i] > b.id[i]) {
            return false;
        }
    }
    return false;
}

bool Bid::operator<=(const Bid& b)const {
    for (int i = 0; i < ID_SIZE; i++) {
        if (id[i] < b.id[i]) {
            return true;
        } else if (id[i] > b.id[i]) {
            return false;
        }
    }
    return true;
}

bool Bid::operator>(const Bid& b)const {
    for (int i = 0; i < ID_SIZE; i++) {
        if (id[i] > b.id[i]) {
            return true;
        } else if (id[i] < b.id[i]) {
            return false;
        }
    }
    return false;
}

bool Bid::operator>=(const Bid& b)const {
    for (int i = 0; i < ID_SIZE; i++) {
        if (id[i] > b.id[i]) {
            return true;
        } else if (id[i] < b.id[i]) {
            return false;
        }
    }
    return true;
}

bool Bid::operator==(const int rhs) const {
    for (int i = 0; i < 4; i++) {
        if (id[3 - i] != (rhs >> (i * 8))) {
            return false;
        }
    }
    return true;
}

bool Bid::operator==(const Bid rhs) const {
    for (int i = 0; i < ID_SIZE; i++) {
        if (id[i] != rhs.id[i]) {
            return false;
        }
    }
    return true;
}

Bid& Bid::operator=(std::vector<byte_t> other) {
    for (int i = 0; i < ID_SIZE; i++) {
        id[i] = other[i];
    }
}

ostream& operator<<(ostream &o, Bid& bid) {
    for(int i=0;i<ID_SIZE;i++){
    o << (int)bid.id[i]<<"-";
    }
    return o;
}