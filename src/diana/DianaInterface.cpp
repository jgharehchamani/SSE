#include "DianaInterface.h"
#include "../utils/Utilities.h"
#include <random>
#include <iostream>
#include <string>
#include <map>
#include <math.h>
#include <random>
#include <stdlib.h>
#include <sse/crypto/hash.hpp>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>

DianaInterface::DianaInterface(bool usehdd, bool initialize, bool deleteResults) {
    this->deleteResults = deleteResults;
    if (initialize) {
        initializeClientAndServer(usehdd);
    } else {
        initializeClient();
    }
    setupMode = false;
}

void DianaInterface::initializeClientAndServer(bool usehdd) {
    sse::logger::set_severity(sse::logger::ERROR);

    string client_master_key_path = "diana_derivation_master.key";
    string client_kw_token_master_key_path = "diana_kw_token_master.key";

    ifstream client_master_key_in(client_master_key_path.c_str());
    ifstream client_kw_token_master_key_in(client_kw_token_master_key_path.c_str());

    if ((client_kw_token_master_key_in.good() != client_master_key_in.good())) {
        client_master_key_in.close();
        client_kw_token_master_key_in.close();

        throw std::runtime_error("All streams are not in the same state");
    }

    if (client_master_key_in.good() == true) {
        // the files exist
        cout << "Restart Diana client and server" << endl;

        stringstream client_master_key_buf, client_kw_token_key_buf;

        client_master_key_buf << client_master_key_in.rdbuf();
        client_kw_token_key_buf << client_kw_token_master_key_in.rdbuf();

        clientDel.reset(new DianaClient<update_token_type>("diana_client_del.sav", client_master_key_buf.str(), client_kw_token_key_buf.str()));

        serverDel.reset(new DianaServer<update_token_type>("diana_server_del.dat", usehdd));

        clientIns.reset(new DianaClient<index_type>("diana_client_ins.sav", client_master_key_buf.str(), client_kw_token_key_buf.str()));

        serverIns.reset(new DianaServer<index_type>("diana_server_ins.dat", usehdd));

    } else {
        cout << "Create new Diana client-server instances" << endl;

        clientIns.reset(new DianaClient<index_type>("diana_client_ins.sav"));

        serverIns.reset(new DianaServer<index_type>("diana_server_ins.dat", 1000, usehdd));

        clientDel.reset(new DianaClient<update_token_type>("diana_client_del.sav"));

        serverDel.reset(new DianaServer<update_token_type>("diana_server_del.dat", 1000, usehdd));

        // write keys to files

        ofstream client_master_key_out(client_master_key_path.c_str());
        client_master_key_out << clientIns->master_derivation_key();
        client_master_key_out.close();

        ofstream client_kw_token_master_key_out(client_kw_token_master_key_path.c_str());
        client_kw_token_master_key_out << clientIns->kw_token_master_key();
        client_kw_token_master_key_out.close();
    }
    client_master_key_in.close();
    client_kw_token_master_key_in.close();
}

void DianaInterface::initializeClient() {
    sse::logger::set_severity(sse::logger::ERROR);

    string client_master_key_path = "diana_derivation_master.key";
    string client_kw_token_master_key_path = "diana_kw_token_master.key";

    ifstream client_master_key_in(client_master_key_path.c_str());
    ifstream client_kw_token_master_key_in(client_kw_token_master_key_path.c_str());

    if ((client_kw_token_master_key_in.good() != client_master_key_in.good())) {
        client_master_key_in.close();
        client_kw_token_master_key_in.close();

        throw std::runtime_error("All streams are not in the same state");
    }

    if (client_master_key_in.good() == true) {
        // the files exist
        cout << "Restart Diana client and server" << endl;

        stringstream client_master_key_buf, client_kw_token_key_buf;

        client_master_key_buf << client_master_key_in.rdbuf();
        client_kw_token_key_buf << client_kw_token_master_key_in.rdbuf();

        clientDel.reset(new DianaClient<update_token_type>("diana_client_del.sav", client_master_key_buf.str(), client_kw_token_key_buf.str()));

        clientIns.reset(new DianaClient<index_type>("diana_client_ins.sav", client_master_key_buf.str(), client_kw_token_key_buf.str()));

    } else {
        cout << "Create new Diana client-server instances" << endl;

        clientIns.reset(new DianaClient<index_type>("diana_client_ins.sav"));

        clientDel.reset(new DianaClient<update_token_type>("diana_client_del.sav"));

        // write keys to files

        ofstream client_master_key_out(client_master_key_path.c_str());
        client_master_key_out << clientIns->master_derivation_key();
        client_master_key_out.close();

        ofstream client_kw_token_master_key_out(client_kw_token_master_key_path.c_str());
        client_kw_token_master_key_out << clientIns->kw_token_master_key();
        client_kw_token_master_key_out.close();
    }
    client_master_key_in.close();
    client_kw_token_master_key_in.close();
}

DianaInterface::~DianaInterface() {

}

//This function is used for the client and server mode
void DianaInterface::insertKeyword(string keyword, index_type ind, UpdateRequest<index_type>& u_req, update_token_type& delCntMapKey, update_token_type& delCntMapValue) {
    totalUpdateCommSize = 0;
    string key;
    if (keywordsToken.count(keyword) != 0) {
        key = keywordsToken[keyword];
    } else {
        keywordsCounter[keyword] = 1;
        key = keyword + to_string(keywordsCounter[keyword]);
        keywordsToken[keyword] = key;
    }
    uint32_t kw_counter;
    u_req = clientIns->update_request(key, ind, kw_counter);
    string hash_string = sse::crypto::Hash::hash(key + "|" + to_string(ind));
    std::copy_n(hash_string.begin(), kUpdateTokenSize, delCntMapKey.begin());
    delCntMapValue = Utilities::encode(to_string(kw_counter));
    totalUpdateCommSize = (sizeof (u_req.index) + kUpdateTokenSize) + kUpdateTokenSize * 2;
}

//This function is used for the single machine mode
void DianaInterface::insertKeyword(string keyword, index_type ind) {
    totalUpdateCommSize = 0;
    string key;
    if (keywordsToken.count(keyword) != 0) {
        key = keywordsToken[keyword];
    } else {
        keywordsCounter[keyword] = 1;
        key = keyword + to_string(keywordsCounter[keyword]);
        keywordsToken[keyword] = key;
    }
    UpdateRequest<index_type> u_req;
    uint32_t kw_counter;
    u_req = clientIns->update_request(key, ind, kw_counter);
    if (!setupMode) {
        serverIns->update(u_req);
    }
    string hash_string = sse::crypto::Hash::hash(key + "|" + to_string(ind));
    update_token_type delCntMapKey;
    std::copy_n(hash_string.begin(), kUpdateTokenSize, delCntMapKey.begin());
    update_token_type delCntMapValue = Utilities::encode(to_string(kw_counter));
    if (!setupMode) {
        serverDel->delCntMap[delCntMapKey] = delCntMapValue;
    }
    totalUpdateCommSize = (sizeof (u_req.index) + kUpdateTokenSize) + kUpdateTokenSize * 2;
}

//This function is used for the single machine mode
list<index_type> DianaInterface::searchKeyword(string keyword) {
    totalSearchCommSize = 0;
    string key = keywordsToken[keyword];
    SearchRequest del_s_req;
    std::list<update_token_type> resDel;
    del_s_req = clientDel->search_request(key);
    totalSearchCommSize += sizeof (del_s_req.add_count) + sizeof (del_s_req.kw_token) + del_s_req.token_list.size()*(1 + sizeof (search_token_key_type));
    resDel = serverDel->search_simple_parallel(del_s_req, 1, false);
    totalSearchCommSize += resDel.size() * sizeof (update_token_type);
    std::vector<uint32_t> delCnts;
    for (auto item : resDel) {
        string cnt = Utilities::decode(serverDel->delCntMap[item]);
        delCnts.push_back(stoi(cnt));
    }
    SearchRequest ins_s_req;
    std::list<index_type> resIns;
    ins_s_req = clientIns->search_request(key, delCnts);
    totalSearchCommSize += sizeof (ins_s_req.add_count) + sizeof (ins_s_req.kw_token) + ins_s_req.token_list.size()*(1 + sizeof (search_token_key_type));
    resIns = serverIns->search_simple_parallel(ins_s_req, 1, false);
    totalSearchCommSize += resIns.size() * sizeof (index_type);
    if (deleteResults) {
        keywordsCounter[keyword]++;
        keywordsToken[keyword] = keyword + to_string(keywordsCounter[keyword]);
        for (auto item : resIns) {
            insertKeyword(keyword, item);
            totalSearchCommSize += totalUpdateCommSize;
        }
    }
    return resIns;
}

//This function is used for the client and server mode
void DianaInterface::searchDeletedKeyword(string keyword, SearchRequest& del_s_req) {
    totalSearchCommSize = 0;
    string key = keywordsToken[keyword];
    std::list<update_token_type> resDel;
    del_s_req = clientDel->search_request(key);
    totalSearchCommSize += sizeof (del_s_req.add_count) + sizeof (del_s_req.kw_token) + del_s_req.token_list.size()*(1 + sizeof (search_token_key_type));
}

//This function is used for the client and server mode
void DianaInterface::searchInsertedKeyword(string keyword, std::list<update_token_type>& resDel, SearchRequest& ins_s_req) {
    totalSearchCommSize += resDel.size() * sizeof (update_token_type);
    string key = keywordsToken[keyword];
    std::vector<uint32_t> delCnts;
    for (auto item : resDel) {
        string cnt = Utilities::decode(item);
        delCnts.push_back(stoi(cnt));
    }
    ins_s_req = clientIns->search_request(key, delCnts);
    totalSearchCommSize += sizeof (ins_s_req.add_count) + sizeof (ins_s_req.kw_token) + ins_s_req.token_list.size()*(1 + sizeof (search_token_key_type));
}

//This function is used for the client and server mode
void DianaInterface::processSearch(std::list<index_type>& resIns, string keyword) {
    totalSearchCommSize += resIns.size() * sizeof (index_type);
    if (deleteResults) {
        keywordsCounter[keyword]++;
        keywordsToken[keyword] = keyword + to_string(keywordsCounter[keyword]);
    }
}

//This function is used for the client and server mode
void DianaInterface::deleteKeyword(string keyword, index_type ind) {
    totalUpdateCommSize = 0;
    string key;
    if (keywordsToken.count(keyword) != 0) {
        key = keywordsToken[keyword];
    } else {
        keywordsCounter[keyword] = 1;
        key = keyword + to_string(keywordsCounter[keyword]);
        keywordsToken[keyword] = key;
    }
    UpdateRequest<update_token_type> u_req;
    string hash_string = sse::crypto::Hash::hash(key + "|" + to_string(ind));
    update_token_type delCntMapKey;
    std::copy_n(hash_string.begin(), kUpdateTokenSize, delCntMapKey.begin());
    uint32_t kw_counter;
    u_req = clientDel->update_request(key, delCntMapKey, kw_counter);
    serverDel->update(u_req);
    totalUpdateCommSize = (2 * kUpdateTokenSize);
}

//This function is used for the single machine mode
void DianaInterface::deleteKeyword(string keyword, index_type ind, UpdateRequest<update_token_type>& u_req) {
    totalUpdateCommSize = 0;
    string key;
    if (keywordsToken.count(keyword) != 0) {
        key = keywordsToken[keyword];
    } else {
        keywordsCounter[keyword] = 1;
        key = keyword + to_string(keywordsCounter[keyword]);
        keywordsToken[keyword] = key;
    }
    string hash_string = sse::crypto::Hash::hash(key + "|" + to_string(ind));
    update_token_type delCntMapKey;
    std::copy_n(hash_string.begin(), kUpdateTokenSize, delCntMapKey.begin());
    uint32_t kw_counter;
    u_req = clientDel->update_request(key, delCntMapKey, kw_counter);
    totalUpdateCommSize = (2 * kUpdateTokenSize);
}

int DianaInterface::getTotalSearchCommSize() const {
    return totalSearchCommSize;
}

double DianaInterface::getTotalUpdateCommSize() const {
    return totalUpdateCommSize;
}

bool DianaInterface::isSetupMode() const {
    return setupMode;
}

void DianaInterface::setSetupMode(bool setupMode) {
    this->setupMode = setupMode;
}