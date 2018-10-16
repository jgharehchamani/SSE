#include "fides/FidesClientRunner.h"
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
#include "fides/FidesClient.h"
#include "src/utils/Utilities.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <math.h>
#include "src/utils/utils.hpp"

using namespace std;

int main(int argv, char** argc) {
    bool usehdd = false, cleaningMode = true;
    FidesClientRunner fides_runner("localhost:4241", usehdd, cleaningMode);
    fides_runner.setup();
    fides_runner.insertKeyword("test1", 1);
    fides_runner.insertKeyword("test1", 2);
    fides_runner.insertKeyword("test1", 3);
    cout << fides_runner.searchKeyword("test1").size() << endl;
    fides_runner.deleteKeyword("test1", 1);
    cout << fides_runner.searchKeyword("test1").size() << endl;
    return 0;
}

