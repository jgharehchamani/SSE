#include "Orion.h"

Orion::Orion(bool usehdd, int maxSize) {
    this->useHDD = usehdd;
    bytes<Key> key1{0};
    bytes<Key> key2{1};
    srch = new OMAP(maxSize, key1);
    updt = new OMAP(maxSize, key2);
}

Orion::~Orion() {
    delete srch;
    delete updt;
}

void Orion::insert(string keyword, int ind) {
    Bid mapKey = createBid(keyword, ind);
    auto updt_cnt = updt->find(mapKey);
    if (updt_cnt == "") {
        if (UpdtCnt.count(keyword) == 0) {
            UpdtCnt[keyword] = 0;
        }
        UpdtCnt[keyword]++;
        updt->insert(mapKey, to_string(UpdtCnt[keyword]));
        Bid key = createBid(keyword, UpdtCnt[keyword]);
        srch->insert(key, to_string(ind));
        LastIND[keyword] = ind;
    }
}

/**
 * This function executes an insert in setup mode. Indeed, it is not applied until endSetup()
 */
void Orion::setupInsert(string keyword, int ind) {
    Bid mapKey = createBid(keyword, ind);
    if (UpdtCnt.count(keyword) == 0) {
        UpdtCnt[keyword] = 0;
    }
    UpdtCnt[keyword]++;
    setupPairs1[mapKey]=to_string(UpdtCnt[keyword]);
    Bid key = createBid(keyword, UpdtCnt[keyword]);
    setupPairs2[key]= to_string(ind);
    LastIND[keyword] = ind;
}

void Orion::remove(string keyword, int ind) {
    Bid mapKey = createBid(keyword, ind);
    string updt_cnt = updt->find(mapKey);
    if (stoi(updt_cnt) > 0) {
        updt->insert(mapKey, to_string(-1));
        UpdtCnt[keyword]--;
        if (UpdtCnt[keyword] > 0) {
            if (UpdtCnt[keyword] + 1 != stoi(updt_cnt)) {
                Bid curKey = createBid(keyword, LastIND[keyword]);
                updt->insert(curKey, updt_cnt);
                Bid curKey2 = createBid(keyword, stoi(updt_cnt));
                srch->insert(curKey2, to_string(LastIND[keyword]));
            }
            Bid key = createBid(keyword, UpdtCnt[keyword]);
            string idstr = srch->find(key);
            int lastID = stoi(idstr);
            LastIND[keyword] = lastID;
        } else {
            LastIND.erase(keyword);
        }
    }
}

/**
 * This function executes a remove in setup mode. Indeed, it is not applied until endSetup()
  */
void Orion::setupRemove(string keyword, int ind) {
 Bid mapKey = createBid(keyword, ind);
    string updt_cnt = setupPairs1[mapKey];
    if (stoi(updt_cnt) > 0) {
        setupPairs1[mapKey]= to_string(-1);
        UpdtCnt[keyword]--;
        if (UpdtCnt[keyword] > 0) {
            if (UpdtCnt[keyword] + 1 != stoi(updt_cnt)) {
                Bid curKey = createBid(keyword, LastIND[keyword]);
                setupPairs1[curKey]= updt_cnt;
                Bid curKey2 = createBid(keyword, stoi(updt_cnt));
                setupPairs2[curKey2]= to_string(LastIND[keyword]);
            }
            Bid key = createBid(keyword, UpdtCnt[keyword]);
            string idstr = setupPairs2[key];
            int lastID = stoi(idstr);
            LastIND[keyword] = lastID;
        } else {
            LastIND.erase(keyword);
        }
    }
}

vector<int> Orion::search(string keyword) {
    vector<int> result;
    vector<Bid> bids;
    if (UpdtCnt.count(keyword) != 0) {
        for (int i = 1; i <= UpdtCnt[keyword]; i++) {
            Bid bid = createBid(keyword, i);
            bids.push_back(bid);
        }
    }
    auto tmpRes = srch->batchSearch(bids);
    for(auto item:tmpRes){
        result.push_back(stoi(item));
    }
    return result;
}

/**
 * This function is used for initial setup of scheme because normal update is time consuming
 */
void Orion::beginSetup() {
    setupPairs1.clear();
    setupPairs2.clear();
}

/**
 * This function is used for finishing setup of scheme because normal update is time consuming
 */
void Orion::endSetup() {
    updt->batchInsert(setupPairs1);
    srch->batchInsert(setupPairs2);
}

Bid Orion::createBid(string keyword, int number) {
    Bid bid(keyword);
    auto arr = to_bytes(number);
    std::copy(arr.begin(), arr.end(), bid.id.end() - 4);
    return bid;
}
