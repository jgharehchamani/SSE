#include "orion/Orion.h"
using namespace std;

int main(int, char**) {
    bool usehdd = false;
    Orion orion(usehdd, 10);
    orion.insert("test1", 1);
    orion.insert("test1", 2);
    orion.insert("test1", 3);
    cout << orion.search("test1").size() << endl;
    orion.remove("test1", 1);
    cout << orion.search("test1").size() << endl;
    return 0;
}

