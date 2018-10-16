#include "Client.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <sse/crypto/prg.hpp>

using namespace std;
using namespace boost::algorithm;

Client::Client(Server* server, bool deleteFiles) {
    this->server = server;
    setupMode = false;
    this->deleteFiles = deleteFiles;
}

Client::Client(bool deleteFiles) {
    setupMode = false;
    this->deleteFiles = deleteFiles;
}

Client::~Client() {
}

void Client::updateRequest(OP op, string keyword, int ind, prf_type& addr, prf_type& val) {
    prf_type k_w;
    memset(k_w.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), k_w.data());
    int fileCnt = 0, srcCnt = 0;

    if (FileCnt.find(k_w) == FileCnt.end()) {
        FileCnt[k_w] = 1;
        fileCnt = 1;
    } else {
        FileCnt[k_w]++;
        fileCnt = FileCnt[k_w];
    }
    if (deleteFiles) {
        if (SrcCnt.find(k_w) == SrcCnt.end()) {
            SrcCnt[k_w] = 0;
            srcCnt = 0;
        } else {
            srcCnt = SrcCnt[k_w];
        }
    }

    prf_type rnd;
    getAESRandomValue(k_w.data(), 0, srcCnt, fileCnt, addr.data());
    getAESRandomValue(k_w.data(), 1, srcCnt, fileCnt, rnd.data());
    val = bitwiseXOR(ind, op, rnd);
    totalUpdateCommSize = (sizeof (prf_type) * 2);
}

void Client::update(OP op, string keyword, int ind) {
    prf_type k_w;
    memset(k_w.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), k_w.data());
    int fileCnt = 0, srcCnt = 0;

    if (FileCnt.find(k_w) == FileCnt.end()) {
        FileCnt[k_w] = 1;
        fileCnt = 1;
    } else {
        FileCnt[k_w]++;
        fileCnt = FileCnt[k_w];
    }
    if (deleteFiles) {
        if (SrcCnt.find(k_w) == SrcCnt.end()) {
            SrcCnt[k_w] = 0;
            srcCnt = 0;
        } else {
            srcCnt = SrcCnt[k_w];
        }
    }

    prf_type addr, rnd;
    getAESRandomValue(k_w.data(), 0, srcCnt, fileCnt, addr.data());
    getAESRandomValue(k_w.data(), 1, srcCnt, fileCnt, rnd.data());
    prf_type val = bitwiseXOR(ind, op, rnd);
    totalUpdateCommSize = (sizeof (prf_type) * 2);
    if (!setupMode) {
        server->update(addr, val);
    }

}

prf_type Client::searchRequest(string keyword, vector<prf_type>& KList) {
    totalSearchCommSize = 0;
    prf_type k_w;
    memset(k_w.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), k_w.data());
    if (FileCnt.find(k_w) == FileCnt.end()) {
        return k_w;
    }
    int fileCnt = FileCnt[k_w], srcCnt = 0;
    if (deleteFiles) {
        srcCnt = SrcCnt[k_w];
    }
    KList.reserve(fileCnt);

    for (int i = 1; i <= fileCnt; i++) {
        prf_type rnd;
        getAESRandomValue(k_w.data(), 0, srcCnt, i, rnd.data());
        KList.emplace_back(rnd);
    }
    totalSearchCommSize += sizeof (prf_type) * KList.size();
    return k_w;
}

void Client::searchProcess(vector<prf_type> encIndexes, prf_type k_w, vector<int>& finalRes, map<prf_type, prf_type>& cleaningPairs) {
    map<int, int> remove;
    int srcCnt = 0;
    int fileCnt = 0;
    if (deleteFiles) {
        srcCnt = SrcCnt[k_w];
    }
    finalRes.reserve(encIndexes.size());
    int cnt = 1;
    for (auto i = encIndexes.begin(); i != encIndexes.end(); i++) {
        prf_type tmp;
        getAESRandomValue(k_w.data(), 1, srcCnt, cnt, tmp.data());
        prf_type decodedString = *i;
        prf_type plaintextBytes = bitwiseXOR(tmp, decodedString);
        int plaintext = (*((int*) &plaintextBytes[0]));
        remove[plaintext] += (2 * plaintextBytes[4] - 1);
        cnt++;
    }
    if (deleteFiles) {
        SrcCnt[k_w]++;
        srcCnt++;
    }
    for (auto const& cur : remove) {
        if (cur.second < 0) {
            finalRes.emplace_back(cur.first);
            if (deleteFiles) {
                fileCnt++;
                prf_type addr, rnd;
                getAESRandomValue(k_w.data(), 0, srcCnt, fileCnt, addr.data());
                getAESRandomValue(k_w.data(), 1, srcCnt, fileCnt, rnd.data());
                prf_type val = bitwiseXOR(cur.first, OP::INS, rnd);
                cleaningPairs.insert(make_pair(addr, val));
            }
        }
    }
    if (deleteFiles) {
        FileCnt[k_w] = fileCnt;
        totalSearchCommSize += (fileCnt * 2 * sizeof (prf_type));
    }
    totalSearchCommSize += encIndexes.size() * sizeof (prf_type);
}

vector<int> Client::search(string keyword) {
    totalSearchCommSize = 0;
    vector<int> finalRes;
    vector<prf_type> KList;
    prf_type k_w;
    memset(k_w.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), k_w.data());
    if (FileCnt.find(k_w) == FileCnt.end()) {
        return finalRes;
    }
    int fileCnt = FileCnt[k_w], srcCnt = 0;
    if (deleteFiles) {
        srcCnt = SrcCnt[k_w];
    }
    KList.reserve(fileCnt);
    finalRes.reserve(fileCnt);
    for (int i = 1; i <= fileCnt; i++) {
        prf_type rnd;
        getAESRandomValue(k_w.data(), 0, srcCnt, i, rnd.data());
        KList.emplace_back(rnd);
    }
    vector<prf_type> encIndexes = server->search(KList);
    map<int, int> remove;
    int cnt = 1;
    for (auto i = encIndexes.begin(); i != encIndexes.end(); i++) {
        prf_type tmp;
        getAESRandomValue(k_w.data(), 1, srcCnt, cnt, tmp.data());
        prf_type decodedString = *i;
        prf_type plaintextBytes = bitwiseXOR(tmp, decodedString);
        int plaintext = (*((int*) &plaintextBytes[0]));
        remove[plaintext] += (2 * plaintextBytes[4] - 1);
        cnt++;
    }
    if (deleteFiles) {
        SrcCnt[k_w]++;
        fileCnt = 0;
        srcCnt++;
    }
    for (auto const& cur : remove) {
        if (cur.second < 0) {
            finalRes.emplace_back(cur.first);
            if (deleteFiles) {
                fileCnt++;
                prf_type addr, rnd;
                getAESRandomValue(k_w.data(), 0, srcCnt, fileCnt, addr.data());
                getAESRandomValue(k_w.data(), 1, srcCnt, fileCnt, rnd.data());
                prf_type val = bitwiseXOR(cur.first, OP::INS, rnd);
                server->update(addr, val);
            }
        }
    }
    if (deleteFiles) {
        FileCnt[k_w] = fileCnt;
        totalSearchCommSize += (fileCnt * 2 * sizeof (prf_type));
    }
    totalSearchCommSize += sizeof (prf_type) * KList.size() + encIndexes.size() * sizeof (prf_type);

    return finalRes;
}

prf_type Client::bitwiseXOR(int input1, int op, prf_type input2) {
    prf_type result;
    result[3] = input2[3] ^ ((input1 >> 24) & 0xFF);
    result[2] = input2[2] ^ ((input1 >> 16) & 0xFF);
    result[1] = input2[1] ^ ((input1 >> 8) & 0xFF);
    result[0] = input2[0] ^ (input1 & 0xFF);
    result[4] = input2[4] ^ (op & 0xFF);
    for (int i = 5; i < AES_KEY_SIZE; i++) {
        result[i] = (rand() % 255) ^ input2[i];
    }
    return result;
}

prf_type Client::bitwiseXOR(prf_type input1, prf_type input2) {
    prf_type result;
    for (unsigned int i = 0; i < input2.size(); i++) {
        result[i] = input1.at(i) ^ input2[i];
    }
    return result;
}

void Client::getAESRandomValue(unsigned char* keyword, int op, int srcCnt, int fileCnt, unsigned char* result) {
    if (deleteFiles) {
        *(int*) (&keyword[AES_KEY_SIZE - 9]) = srcCnt;
    }
    keyword[AES_KEY_SIZE - 5] = op & 0xFF;
    *(int*) (&keyword[AES_KEY_SIZE - 4]) = fileCnt;
    sse::crypto::Prg::derive((unsigned char*) keyword, 0, AES_KEY_SIZE, result);
}

int Client::getFileCntSize() const {
    return FileCnt.size();
}

bool Client::isSetupMode() const {
    return setupMode;
}

double Client::getTotalSearchCommSize() const {
    return totalSearchCommSize;
}

double Client::getTotalUpdateCommSize() const {
    return totalUpdateCommSize;
}

void Client::setSetupMode(bool setupMode) {
    this->setupMode = setupMode;
}