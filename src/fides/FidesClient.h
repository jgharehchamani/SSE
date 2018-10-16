#ifndef FIDES_H
#define FIDES_H
#include <iostream>
#include <iostream>
#include <fstream>
#include <memory>
#include <map>
#include "../utils/Utilities.h"

#include "fides/sophos_client.hpp"
#include "fides/sophos_server.hpp"
#include "../utils/utils.hpp"

using namespace std;
using namespace sse::sophos;

class FidesClient {
private:
    bool deleteItems;
    unique_ptr<SophosClient> insertClient;
    unique_ptr<SophosServer> insertServer;
    double totalUpdateCommSize = 0;
    double totalSearchCommSize = 0;

public:
    FidesClient(bool usehdd, bool deleteItems);
    virtual ~FidesClient();
    void insertKeyword(string key, index_type ind, UpdateRequest& u_req);
    void deleteKeyword(string key, index_type ind, UpdateRequest& u_req);
    void searchRequest(string key, SearchRequest& s_req_ins);
    list<index_type> searchProcess(list<std::array<uint8_t, kUpdateTokenSize> > res_ins);
    void insertKeyword(string key, index_type ind);
    void deleteKeyword(string key, index_type ind);
    list<index_type> searchKeyword(string key);
    double getTotalSearchCommSize() const;
    double getTotalUpdateCommSize() const;
    double getServerStorageSize();
    void setTotalSearchCommSize(double totalSearchCommSize);
    void setTotalUpdateCommSize(double totalUpdateCommSize);
};

#endif /* FIDES_H */

