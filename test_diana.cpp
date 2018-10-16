#include <iostream>
#include <fstream>
#include <memory>

#include "diana/DianaInterface.h"
#include <iostream>
#include <fstream>
#include <memory>
#include "utils/Utilities.h"
#include <math.h>

#include "src/utils/utils.hpp"

using namespace std;

int main(int, char**) {
    bool usehdd=false,cleaningMode=true;
    DianaInterface diana(usehdd, true, cleaningMode);
    diana.insertKeyword("test1",1);
    diana.insertKeyword("test1",2);
    diana.insertKeyword("test1",3);
    cout<<diana.searchKeyword("test1").size()<<endl;
    diana.deleteKeyword("test1",1);
    cout<<diana.searchKeyword("test1").size()<<endl;       
    return 0;
}

