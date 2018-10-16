#include "mitra/MitraClientRunner.h"
#include "utils/logger.hpp"

#include <sse/crypto/utils.hpp>


#include <list>
#include <mutex>
#include <iostream>

#include <unistd.h>
#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include <math.h>
#include "mitra/Client.h"
#include "src/utils/Utilities.h"
using namespace std;

int main(int argv, char** argc) {
    bool usehdd = false, cleaningMode = true;
    MitraClientRunner client_runner("localhost:4241", usehdd, cleaningMode);
    client_runner.update(OP::INS, "test1", 1);
    client_runner.update(OP::INS, "test1", 2);
    client_runner.update(OP::INS, "test1", 3);
    cout << client_runner.search("test1").size() << endl;
    client_runner.update(OP::DEL, "test1", 1);
    cout << client_runner.search("test1").size() << endl;
    return 0;
}
