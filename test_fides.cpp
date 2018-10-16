#include <iostream>
#include <fstream>
#include <memory>
#include <math.h>
#include "fides/sophos_client.hpp"
#include "fides/sophos_server.hpp"
#include "src/utils/utils.hpp"
#include "fides/FidesClient.h"

using namespace std;

int main(int, char**) {
    bool usehdd = false, cleaningMode = true;
    FidesClient fides(usehdd, cleaningMode);
    fides.insertKeyword("test1", 1);
    fides.insertKeyword("test1", 2);
    fides.insertKeyword("test1", 3);
    cout << fides.searchKeyword("test1").size() << endl;
    fides.deleteKeyword("test1", 1);
    cout << fides.searchKeyword("test1").size() << endl;
    return 0;
}

