#include <iostream>
#include <fstream>
#include <memory>
#include "src/janus/JanusInterface.h"
#include <iostream>
#include <fstream>
#include <memory>
#include "utils/Utilities.h"
#include <math.h>
#include "src/utils/utils.hpp"

using namespace std;

int main(int, char**) {
    bool usehdd = false;
    JanusInterface janus(usehdd, 99);
    janus.insertKeyword("test1", 1);
    janus.insertKeyword("test1", 2);
    janus.insertKeyword("test1", 3);
    cout << janus.searchKeyword("test1").size() << endl;
    janus.deleteKeyword("test1", 1);
    cout << janus.searchKeyword("test1").size() << endl;
    return 0;
}

