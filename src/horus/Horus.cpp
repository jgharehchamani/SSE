#include "Horus.h"
#include "utils/Utilities.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <sse/crypto/prg.hpp>

using namespace boost::algorithm;

Horus::Horus(bool usehdd, int maxSize) {
    this->useHDD = usehdd;
    bytes<Key> key1{0};
    bytes<Key> key2{1};
    OMAP_updt = new OMAP(maxSize * 2, key1);
    ORAM_srch = new PRFORAM(maxSize * 2, key2);
    this->maxSize = maxSize;
}

Horus::~Horus() {
    delete OMAP_updt;
    delete ORAM_srch;
}

void Horus::insert(string keyword, int ind) {
    Bid mapKey = createBid(0, keyword, ind);
    string resStr = OMAP_updt->find(mapKey);
    std::vector<std::string> tokens;
    split(tokens, resStr, is_any_of("#"));
    if (resStr == "" || (tokens.size() > 0 && tokens[0] == "-1")) {
        if (UpdtCnt.count(keyword) == 0) {
            UpdtCnt[keyword] = 0;
            latestUpdatedCounter[keyword] = 0;
            SrchCnt[keyword] = 0;
        }
        if (Access.count(keyword) == 0) {
            Access[keyword] = 1;
        }
        UpdtCnt[keyword]++;
        Bid mapKey2 = createBid(1, keyword, UpdtCnt[keyword]);
        resStr = OMAP_updt->find(mapKey2);
        int acc_cnt = 1;
        if (resStr != "") {
            tokens.clear();
            split(tokens, resStr, is_any_of("#"));
            acc_cnt = stoi(tokens[1]) + 1;
            if (acc_cnt > Access[keyword]) {
                Access[keyword] = acc_cnt;
            }
        }
        OMAP_updt->insert(mapKey, to_string(UpdtCnt[keyword]) + "#" + to_string(acc_cnt));
        OMAP_updt->insert(mapKey2, to_string(ind) + "#" + to_string(acc_cnt));
        Bid oramID = createBid(keyword, UpdtCnt[keyword], SrchCnt[keyword], acc_cnt);
        int pos = generatePosition(keyword, UpdtCnt[keyword], SrchCnt[keyword], acc_cnt);
        ORAM_srch->WriteBox(oramID, to_string(ind), pos);
        if (latestUpdatedCounter[keyword] < UpdtCnt[keyword]) {
            latestUpdatedCounter[keyword]++;
            if (acc_cnt > 1) {
                Bid oramID2 = createBid(keyword, UpdtCnt[keyword], SrchCnt[keyword], 1);
                int pos2 = generatePosition(keyword, UpdtCnt[keyword], SrchCnt[keyword], 1);
                ORAM_srch->WriteBox(oramID2, "@" + to_string(acc_cnt), pos2);
            }
        }
        LastIND[keyword] = ind;
    }
}

/**
 * This function executes an insert in setup mode. Indeed, it is not applied until endSetup().
 */
void Horus::setupInsert(string keyword, int ind) {
    Bid mapKey = createBid(0, keyword, ind);
    std::vector<std::string> tokens;
    if (UpdtCnt.count(keyword) == 0) {
        UpdtCnt[keyword] = 0;
    }
    if (Access.count(keyword) == 0) {
        Access[keyword] = 1;
    }
    UpdtCnt[keyword]++;
    Bid mapKey2 = createBid(1, keyword, UpdtCnt[keyword]);
    int acc_cnt = 1;

    setupPairs1[mapKey] = to_string(UpdtCnt[keyword]) + "#" + to_string(acc_cnt);
    setupPairs1[mapKey2] = to_string(ind) + "#" + to_string(acc_cnt);
    Bid oramID = createBid(keyword, UpdtCnt[keyword], SrchCnt[keyword], acc_cnt);
    int pos = generatePosition(keyword, UpdtCnt[keyword], SrchCnt[keyword], acc_cnt);
    setupPairs2[oramID] = to_string(ind);
    setupPairsPos[oramID] = pos;
    LastIND[keyword] = ind;
}

void Horus::remove(string keyword, int ind) {
    Bid mapKey = createBid(0, keyword, ind);
    string resStr = OMAP_updt->find(mapKey);
    std::vector<std::string> tokens;
    split(tokens, resStr, is_any_of("#"));
    if (tokens.size() > 0 && stoi(tokens[0]) > 0) {
        int updt_cnt = stoi(tokens[0]);
        int acc_cnt = stoi(tokens[1]);
        OMAP_updt->insert(mapKey, "-1#" + to_string(acc_cnt + 1));
        UpdtCnt[keyword]--;
        if (UpdtCnt[keyword] > 0) {
            if (UpdtCnt[keyword] + 1 != updt_cnt) {
                acc_cnt++;
                if (acc_cnt > Access[keyword]) {
                    Access[keyword] = acc_cnt;
                }
                Bid oramID = createBid(keyword, updt_cnt, SrchCnt[keyword], acc_cnt);
                int pos = generatePosition(keyword, updt_cnt, SrchCnt[keyword], acc_cnt);
                ORAM_srch->WriteBox(oramID, to_string(LastIND[keyword]), pos);
                mapKey = createBid(0, keyword, LastIND[keyword]);
                OMAP_updt->insert(mapKey, to_string(updt_cnt) + "#" + to_string(acc_cnt));
                mapKey = createBid(1, keyword, updt_cnt);
                OMAP_updt->insert(mapKey, to_string(LastIND[keyword]) + "#" + to_string(acc_cnt));
            }
            Bid mapKey2 = createBid(1, keyword, UpdtCnt[keyword]);
            resStr = OMAP_updt->find(mapKey2);
            tokens.clear();
            split(tokens, resStr, is_any_of("#"));
            LastIND[keyword] = stoi(tokens[0]);
        } else {
            LastIND.erase(keyword);
        }
    }
}

/**
 * This function executes a remove in setup mode. Indeed, it is not applied until endSetup().
 */
void Horus::setupRemove(string keyword, int ind) {
    Bid mapKey = createBid(0, keyword, ind);
    string resStr = setupPairs1[mapKey];
    std::vector<std::string> tokens;
    split(tokens, resStr, is_any_of("#"));
    if (tokens.size() > 0 && stoi(tokens[0]) > 0) {
        int updt_cnt = stoi(tokens[0]);
        int acc_cnt = stoi(tokens[1]);
        setupPairs1[mapKey] = "-1#" + to_string(acc_cnt + 1);
        UpdtCnt[keyword]--;
        if (UpdtCnt[keyword] > 0) {
            if (UpdtCnt[keyword] + 1 != updt_cnt) {
                acc_cnt++;
                if (acc_cnt > Access[keyword]) {
                    Access[keyword] = acc_cnt;
                }
                Bid oramID = createBid(keyword, updt_cnt, SrchCnt[keyword], acc_cnt);
                int pos = generatePosition(keyword, updt_cnt, SrchCnt[keyword], acc_cnt);
                setupPairs2[oramID] = to_string(LastIND[keyword]);
                setupPairsPos[oramID] = pos;
                mapKey = createBid(0, keyword, LastIND[keyword]);
                setupPairs1[mapKey] = to_string(updt_cnt) + "#" + to_string(acc_cnt);
                mapKey = createBid(1, keyword, updt_cnt);
                setupPairs1[mapKey] = to_string(LastIND[keyword]) + "#" + to_string(acc_cnt);
            }
            Bid mapKey2 = createBid(1, keyword, UpdtCnt[keyword]);
            resStr = setupPairs1[mapKey2];
            tokens.clear();
            split(tokens, resStr, is_any_of("#"));
            LastIND[keyword] = stoi(tokens[0]);
        } else {
            LastIND.erase(keyword);
        }
    }
}

vector<int> Horus::search(string keyword) {
    vector<int> result;
    vector<int> left, right, curValue, lastAcc;
    string* lastID = new string[UpdtCnt[keyword]];
    vector<bool> foundItem;
    vector<pair<Bid, int> > batchWriteQuery;
    vector<pair<Bid, string> > batchWriteQueryID;
    for (int i = 1; i <= UpdtCnt[keyword]; i++) {
        foundItem.push_back(false);
        right.push_back(Access[keyword]);
        left.push_back(1);
        curValue.push_back(1);
        lastAcc.push_back(0);
    }
    bool firstRun = true;
    bool foundAll = false;
    while (!foundAll) {
        foundAll = true;
        vector<pair<Bid, int> > batchReadQuery;
        for (int i = 1; i <= UpdtCnt[keyword]; i++) {
            if (foundItem[i - 1] == false) {
                foundAll = false;
                int pos = generatePosition(keyword, i, SrchCnt[keyword], curValue[i - 1]);
                Bid oramID = createBid(keyword, i, SrchCnt[keyword], curValue[i - 1]);
                batchReadQuery.push_back(make_pair(oramID, pos));
            }
        }
        if (foundAll == false) {
            vector<string> ids = ORAM_srch->batchRead(batchReadQuery);
            int tmpCnt = 0;
            for (int i = 0; i < UpdtCnt[keyword]; i++) {
                if (foundItem[i] == false) {
                    string id = ids[tmpCnt];
                    tmpCnt++;
                    if (firstRun) {
                        if (id != "" && id.at(0) == '@' && left[i] == 1) {
                            left[i] = stoi(id.substr(1, id.length() - 1));
                            curValue[i] = (left[i] + right[i]) / 2;
                            lastAcc[i] = curValue[i];
                        } else {
                            curValue[i] = (left[i] + right[i]) / 2;
                        }
                        continue;
                    }

                    if (id == "" || left[i] >= right[i]) {
                        if (right[i] <= left[i]) {
                            foundItem[i] = true;
                            if (lastID[i] == "") {
                                lastID[i] = id;
                                lastAcc[i] = curValue[i];
                            }
                            result.push_back(stoi(lastID[i]));
                            Bid oramID = createBid(keyword, i + 1, SrchCnt[keyword] + 1, lastAcc[i]);
                            int pos = generatePosition(keyword, i + 1, SrchCnt[keyword] + 1, lastAcc[i]);
                            batchWriteQuery.push_back(make_pair(oramID, pos));
                            batchWriteQueryID.push_back(make_pair(oramID, lastID[i]));
                            if (lastAcc[i] > 1) {
                                oramID = createBid(keyword, i + 1, SrchCnt[keyword] + 1, 1);
                                pos = generatePosition(keyword, i + 1, SrchCnt[keyword] + 1, 1);
                                batchWriteQuery.push_back(make_pair(oramID, pos));
                                batchWriteQueryID.push_back(make_pair(oramID, "@" + to_string(lastAcc[i])));
                            }
                        } else {

                            if (curValue[i] == right[i]) {
                                curValue[i] = (int) floor((double) (left[i] + right[i]) / 2.0);
                                right[i] = curValue[i];
                            } else {
                                right[i] = curValue[i];
                                curValue[i] = (int) ceil((double) (left[i] + right[i]) / 2.0);
                            }
                        }
                    } else {
                        lastID[i] = id;
                        lastAcc[i] = curValue[i];
                        if (curValue[i] == left[i]) {
                            left[i] = curValue[i];
                            curValue[i] = (int) ceil((double) (left[i] + right[i]) / 2.0);
                        } else {
                            left[i] = curValue[i];
                            curValue[i] = (int) floor((double) (left[i] + right[i]) / 2.0);
                        }
                    }
                }
            }
            firstRun = false;
        }
    }
    SrchCnt[keyword]++;
    ORAM_srch->batchWrite(batchWriteQueryID, batchWriteQuery);
    latestUpdatedCounter[keyword] = result.size();
    delete[] lastID;
    return result;
}

/**
 * This function is used for initial setup of scheme because normal update is time consuming
 */
void Horus::beginSetup() {
    setupPairs1.clear();
    setupPairs2.clear();
}

/**
 * This function is used for finishing setup of scheme because normal update is time consuming
 */
void Horus::endSetup() {
    OMAP_updt->batchInsert(setupPairs1);
    ORAM_srch->SetupBatchWriteBox(setupPairs2, setupPairsPos);
}

Bid Horus::createBid(byte_t prefix, string keyword, int number) {
    Bid bid;
    bid.id[0] = prefix;
    std::copy(keyword.begin(), keyword.end(), bid.id.begin() + 1);
    auto arr = to_bytes(number);
    std::copy(arr.begin(), arr.end(), bid.id.end() - 4);
    return bid;
}

Bid Horus::createBid(string keyword, int val1, int val2, int val3) {
    Bid bid;
    auto arr = to_bytes(val1);
    std::copy(arr.begin(), arr.end(), bid.id.begin());
    arr = to_bytes(val2);
    std::copy(arr.begin(), arr.end(), bid.id.begin() + 4);
    arr = to_bytes(val3);
    std::copy(arr.begin(), arr.end(), bid.id.begin() + 8);
    std::copy(keyword.begin(), keyword.end(), bid.id.begin() + 12);
    return bid;
}

Bid Horus::createBid(string keyword, int number) {
    Bid bid(keyword);
    auto arr = to_bytes(number);
    std::copy(arr.begin(), arr.end(), bid.id.end() - 4);
    return bid;
}

/*
 * This function generates the corresponding position of Path-ORAM using a AES PRF
 */
int Horus::generatePosition(string keyword, int updt_cnt, int src_cnt, int acc_cnt) {
    string key = keyword + "-" + to_string(updt_cnt) + "-" + to_string(src_cnt) + "-" + to_string(acc_cnt);
    unsigned int* result = new unsigned int();
    sse::crypto::Prg::derive((unsigned char*) key.c_str(), 0, 4, (unsigned char*) result);
    return (*result) % (int) ((pow(2, floor(log2(maxSize * 2)) + 1) - 1) / 2);
}
