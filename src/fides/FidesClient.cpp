#include "FidesClient.h"
#include <sstream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <sse/dbparser/DBParserJSON.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
using namespace boost::algorithm;

FidesClient::FidesClient(bool usehdd, bool deleteItems) {
    this->deleteItems = deleteItems;
    string client_sk_path = "tdp_sk.key";
    string client_master_key_path = "derivation_master.key";
    string server_pk_path = "tdp_pk.key";
    string client_tdp_prg_key_path = "tdp_prg.key";

    ifstream client_sk_in(client_sk_path.c_str());
    ifstream client_master_key_in(client_master_key_path.c_str());
    ifstream server_pk_in;
    server_pk_in.open(server_pk_path.c_str());
    ifstream client_tdp_prg_key_in_ins(client_tdp_prg_key_path.c_str());

    if ((client_sk_in.good() != client_master_key_in.good()) || (client_sk_in.good() != server_pk_in.good())) {
        server_pk_in.close();
        client_sk_in.close();
        client_master_key_in.close();


        throw std::runtime_error("All streams are not in the same state");
    }

    if (client_sk_in.good() == true) {
        // the files exist
        cout << "Restart client and server" << endl;

        stringstream client_sk_buf, client_master_key_buf, server_pk_buf, client_tdp_prg_key_buf;

        client_sk_buf << client_sk_in.rdbuf();
        client_master_key_buf << client_master_key_in.rdbuf();
        server_pk_buf << server_pk_in.rdbuf();
        client_tdp_prg_key_buf << client_tdp_prg_key_in_ins.rdbuf();

        insertClient.reset(new SophosClient("client.sav", client_sk_buf.str(), client_master_key_buf.str(), client_tdp_prg_key_buf.str()));
        insertServer.reset(new SophosServer("server.dat", server_pk_buf.str(), usehdd));

    } else {
        insertClient.reset(new SophosClient("client.sav", 1000));
        insertServer.reset(new SophosServer("server.dat", 1000, insertClient->public_key(), usehdd));

        // write keys to files
        ofstream client_sk_out(client_sk_path.c_str());
        client_sk_out << insertClient->private_key();
        client_sk_out.close();

        ofstream client_master_key_out(client_master_key_path.c_str());
        client_master_key_out << insertClient->master_derivation_key();
        client_master_key_out.close();

        ofstream server_pk_out(server_pk_path.c_str());
        server_pk_out << insertServer->public_key();
        server_pk_out.close();
    }
}

FidesClient::~FidesClient() {
    insertClient.release();
    insertServer.release();
}

//This function is used for the client and server mode
void FidesClient::deleteKeyword(string key, index_type ind, UpdateRequest& u_req) {
    u_req = insertClient->update_request(key, Utilities::encode(to_string(ind) + "|0"));
    totalUpdateCommSize = (sizeof (u_req.index) + sizeof (u_req.token));
}

//This function is used for the client and server mode
void FidesClient::insertKeyword(string key, index_type ind, UpdateRequest& u_req) {
    u_req = insertClient->update_request(key, Utilities::encode(to_string(ind) + "|1"));
    totalUpdateCommSize = (sizeof (u_req.index) + sizeof (u_req.token));
}

//This function is used for the client and server mode
void FidesClient::searchRequest(string key, SearchRequest& s_req_ins) {
    totalSearchCommSize = 0;
    s_req_ins = insertClient->search_request(key);
    totalSearchCommSize += (sizeof (s_req_ins.add_count))+(sizeof (s_req_ins.token));
}

//This function is used for the client and server mode
list<index_type> FidesClient::searchProcess(list<std::array<uint8_t, kUpdateTokenSize> > res_ins) {
    list<index_type> final_res;
    map<index_type, int> remove;
    for (array<uint8_t, kUpdateTokenSize> i : res_ins) {
        string plainValue = Utilities::decode(i);
        std::vector<std::string> tokens;
        split(tokens, plainValue, is_any_of("|"));
        index_type finalPlainValue = stoi(tokens[0]);
        remove[finalPlainValue] += (2 * stoi(tokens[1]) - 1);
    }
    for (auto const& cur : remove) {
        if (cur.second > 0) {
            final_res.push_back(cur.first);
        }
    }
    totalSearchCommSize += (kUpdateTokenSize * res_ins.size());
    return final_res;
}

//This function is used for the single machine mode
void FidesClient::deleteKeyword(string key, index_type ind) {
    UpdateRequest u_req;
    u_req = insertClient->update_request(key, Utilities::encode(to_string(ind) + "|0"));
    totalUpdateCommSize = (sizeof (u_req.index) + sizeof (u_req.token));
    insertServer->update(u_req);
}

//This function is used for the single machine mode
void FidesClient::insertKeyword(string key, index_type ind) {
    UpdateRequest u_req;
    u_req = insertClient->update_request(key, Utilities::encode(to_string(ind) + "|1"));
    totalUpdateCommSize = (sizeof (u_req.index) + sizeof (u_req.token));
    insertServer->update(u_req);
}

//This function is used for the single machine mode
list<index_type> FidesClient::searchKeyword(string key) {
    list<index_type> final_res;
    SearchRequest s_req_ins;
    list<std::array<uint8_t, kUpdateTokenSize> > res_ins;
    s_req_ins = insertClient->search_request(key);
    res_ins = insertServer->search(s_req_ins, deleteItems);
    search_token_type st = s_req_ins.token;

    map<index_type, int> remove;
    int finalsize = 0;
    for (array<uint8_t, kUpdateTokenSize> i : res_ins) {
        string plainValue = Utilities::decode(i);
        std::vector<std::string> tokens;
        split(tokens, plainValue, is_any_of("|"));
        index_type finalPlainValue = stoi(tokens[0]);
        remove[finalPlainValue] += (2 * stoi(tokens[1]) - 1);
        st = insertServer->public_tdp_.eval(st);
    }

    for (auto const& cur : remove) {
        if (cur.second > 0) {
            final_res.push_back(cur.first);
            if (deleteItems) {
                totalUpdateCommSize = 0;
                insertKeyword(key, cur.first);
                finalsize += totalUpdateCommSize;
            }
        }
    }
    totalSearchCommSize = finalsize + (sizeof (s_req_ins.add_count))+(sizeof (s_req_ins.token))+(kUpdateTokenSize * res_ins.size());
    return final_res;
}

double FidesClient::getTotalSearchCommSize() const {
    return totalSearchCommSize;
}

double FidesClient::getTotalUpdateCommSize() const {
    return totalUpdateCommSize;
}

double FidesClient::getServerStorageSize() {
    return (sizeof (insertServer->curArray.begin()->first) + sizeof (insertServer->curArray.begin()->second)) * insertServer->curArray.size();
}

void FidesClient::setTotalSearchCommSize(double totalSearchCommSize) {
    this->totalSearchCommSize = totalSearchCommSize;
}

void FidesClient::setTotalUpdateCommSize(double totalUpdateCommSize) {
    this->totalUpdateCommSize = totalUpdateCommSize;
}