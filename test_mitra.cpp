#include "mitra/Client.h"
#include "src/utils/Utilities.h"
using namespace std;

int main(int, char**) {
    bool usehdd = false, cleaningMode = true;
    Server server(usehdd, cleaningMode);
    Client client(&server, cleaningMode);
    client.update(OP::INS, "test1", 1);
    client.update(OP::INS, "test1", 2);
    client.update(OP::INS, "test1", 3);
    cout << client.search("test1").size() << endl;
    client.update(OP::DEL, "test1", 1);
    cout << client.search("test1").size() << endl;
    return 0;
}

