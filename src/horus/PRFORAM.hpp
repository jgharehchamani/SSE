#ifndef PRFORAM_H
#define PRFORAM_H

#include "AES.hpp"
#include <random>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include "RAMStore.hpp"
#include <map>
#include <set>
#include <bits/stdc++.h>
#include "Bid.h"
#include "ORAM.hpp"
using namespace std;

class Box {
public:

    Box() {
    }

    ~Box() {
    }
    Bid key;
    std::array< byte_t, 16> value;
    int pos;
};

class PRFORAM {
private:
    RAMStore* store;
    using Stash = std::unordered_map<Bid, block>;
    size_t depth;
    size_t blockSize;
    map<Bid, Box*> stash;
    vector<int> leafList;
    vector<int> viewmap;
    set<Bid> modified;
    size_t bucketCount;

    set<int> emptyBoxs;
    bytes<Key> key;

    map<int, vector<Bid> > nodePoses;
    vector<Bid> deleted;

    int GetBoxOnPath(int leaf, int depth);
    std::vector<Bid> GetIntersectingBlocks(int x, int depth);

    void FetchPath(int leaf, bool batchRead = false);
    void WritePath(int leaf, int d);

    Box* ReadData(Bid bid);
    void WriteData(Bid bid, Box* b);

    block SerialiseBucket(Bucket bucket);
    Bucket DeserialiseBucket(block buffer);

    Bucket ReadBucket(int pos);
    void WriteBucket(int pos, Bucket bucket);
    string Access(Bid bid, Box*& node, int pos);
    void Access(Bid bid, Box*& node);

    size_t plaintext_size;
    size_t clen_size;

    bool WasSerialised();
    void Print();
    static Box* convertBlockToBox(block b);
    static block convertBoxToBlock(Box* node);

public:
    PRFORAM(int maxSize, bytes<Key> key);
    ~PRFORAM();

    string ReadBox(Bid bid, int pos);
    void WriteBox(Bid bid, string value, int pos);
    vector<string> batchRead(vector<pair<Bid, int> > batchQuery);
    void batchWrite(map<Bid, string> values, map<Bid, int> poses);
};

#endif