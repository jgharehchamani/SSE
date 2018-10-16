#ifndef DIANAINTERFACE_H
#define DIANAINTERFACE_H
#include <iostream>
#include <fstream>
#include <memory>
#include <map>

#include "diana/diana_client.hpp"
#include "diana/diana_server.hpp"
#include "utils/utils.hpp"
#include "utils/logger.hpp"

using namespace sse::diana;
using namespace std;

typedef uint64_t index_type;

class DianaInterface {
private:
    void initializeClient();
    bool deleteResults;
    map< string, string> keywordsToken;
    map< string, int> keywordsCounter;
    double totalUpdateCommSize;
    int totalSearchCommSize;
    bool setupMode;
    unique_ptr<DianaClient<update_token_type>> clientDel;
    unique_ptr<DianaServer<update_token_type>> serverDel;
    unique_ptr<DianaClient<index_type>> clientIns;
    unique_ptr<DianaServer<index_type>> serverIns;

public:
    DianaInterface(bool usehdd, bool initialize, bool deleteResults);
    void initializeClientAndServer(bool usehdd);
    virtual ~DianaInterface();
    void insertKeyword(string key, index_type ind);
    void insertKeyword(string key, index_type ind, UpdateRequest<index_type>& u_req, update_token_type& delCntMapKey, update_token_type& delCntMapValue);
    void deleteKeyword(string key, index_type ind);
    void deleteKeyword(string key, index_type ind, UpdateRequest<update_token_type>& u_req);
    void searchDeletedKeyword(string key, SearchRequest& del_s_req);
    void searchInsertedKeyword(string key, std::list<update_token_type>& resDel, SearchRequest& ins_s_req);
    void processSearch(std::list<index_type>& resIns, string keyword);
    list<index_type> searchKeyword(string key);
    int getTotalSearchCommSize() const;
    double getTotalUpdateCommSize() const;
    bool isSetupMode() const; 
    void setSetupMode(bool setupMode);

};

#endif /* DIANAINTERFACE_H */

