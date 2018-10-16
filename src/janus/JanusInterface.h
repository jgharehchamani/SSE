#ifndef JANUSINTERFACE_H
#define JANUSINTERFACE_H
#include <iostream>
#include <ostream>
#include <fstream>

#include <sse/crypto/puncturable_enc.hpp>
#include <sse/crypto/random.hpp>
#include <sse/crypto/utils.hpp>

#include <chrono>
#include <cassert>

#include "janus/janus_client.hpp"
#include "janus/janus_server.hpp"

using namespace sse::crypto;
using namespace sse::janus;
using namespace std;

typedef uint64_t index_type;

class JanusInterface {
private:


public:
    JanusInterface(bool usehdd,int id);
    virtual ~JanusInterface();
    void insertKeyword(string key, index_type ind);
    void deleteKeyword(string key, index_type ind);
    list<index_type> searchKeyword(string key);
    double totalClientUpdateTime = 0;
    double totalServerUpdateTime = 0;
    double totalUpdateCommSize = 0;
    int totalUpdates = 0;
    unique_ptr<JanusClient> client;
    unique_ptr<JanusServer> server;


};

#endif /* JANUSINTERFACE_H */

