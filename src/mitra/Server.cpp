#include "Server.h"
#include <iostream>
#include <sstream>
#include <cryptopp/rng.h>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
#include <vector>
#include "utils/Utilities.h"

Server::Server(bool useHDD, bool deleteFiles) : edb_("mitra") {
    this->useRocksDB = useHDD;
    this->deleteFiles = deleteFiles;
    this->useRocksDB=false;
}

Server::~Server() {
}

void Server::update(prf_type addr, prf_type val) {
    if (useRocksDB) {
        edb_.put(addr, val);
    } else {
        DictW[addr] = val;
    }
}

vector<prf_type> Server::search(vector<prf_type> KList) {
    vector<prf_type> result;
    result.reserve(KList.size());
    prf_type notfound;
    memset(notfound.data(), 0, AES_KEY_SIZE);
    for (unsigned int i = 0; i < KList.size(); i++) {
        prf_type val;
        if (useRocksDB) {
            bool found = edb_.get(KList[i], val);
            if (found) {
                result.push_back(val);
                if (deleteFiles) {
                    edb_.remove(KList[i]);
                }
            }
        } else {
            val = DictW[KList[i]];
            if (val != notfound) {
                result.emplace_back(val);
            }
        }
    }
    return result;
}


