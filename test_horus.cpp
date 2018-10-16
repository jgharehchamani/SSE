#include "horus/Horus.h"
using namespace std;

int main(int, char**) {
    bool usehdd = false;
    Horus horus(usehdd, 10);
    horus.insert("test1", 1);
    horus.insert("test1", 2);
    horus.insert("test1", 3);
    cout << horus.search("test1").size() << endl;
    horus.remove("test1", 1);
    cout << horus.search("test1").size() << endl;
    return 0;
}

