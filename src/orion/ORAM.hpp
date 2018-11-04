#ifndef ORAM_H
#define ORAM_H

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

using namespace std;

class Node {
public:

    Node() {
    }

    ~Node() {
    }
    Bid key;
    std::array< byte_t, 16> value;
    int pos;
    Bid leftID;
    int leftPos;
    Bid rightID;
    int rightPos;
    unsigned int height;
};

struct Block {
    Bid id;
    block data;
};

using Bucket = std::array<Block, Z>;

class ORAM {
private:
    RAMStore* store;
    using Stash = std::unordered_map<Bid, block>;
    size_t depth;
    size_t blockSize;
    map<Bid, Node*> cache;
    vector<int> leafList;
    vector<int> readviewmap;
    vector<int> writeviewmap;
    set<Bid> modified;
    int readCnt = 0;
    bytes<Key> key;

    // Randomness
    std::random_device rd;
    std::mt19937 mt;
    std::uniform_int_distribution<int> dis;

    int RandomPath();
    int GetNodeOnPath(int leaf, int depth);
    std::vector<Bid> GetIntersectingBlocks(int x, int depth);

    void FetchPath(int leaf);
    void WritePath(int leaf, int level);

    Node* ReadData(Bid bid);
    void WriteData(Bid bid, Node* b);

    block SerialiseBucket(Bucket bucket);
    Bucket DeserialiseBucket(block buffer);

    Bucket ReadBucket(int pos);
    void WriteBucket(int pos, Bucket bucket);
    void Access(Bid bid, Node*& node, int lastLeaf, int newLeaf);
    void Access(Bid bid, Node*& node);


    size_t plaintext_size;
    size_t bucketCount;
    size_t clen_size;
    bool batchWrite = false;

    bool WasSerialised();
    void Print();

public:
    ORAM(int maxSize, bytes<Key> key);
    ~ORAM();

    Node* ReadNode(Bid bid, int lastLeaf, int newLeaf);
    Node* ReadNode(Bid bid);
    int WriteNode(Bid bid, Node* n);
    void start(bool batchWrite);
    void finilize(bool find, Bid& rootKey, int& rootPos);
    static Node* convertBlockToNode(block b);
    static block convertNodeToBlock(Node* node);
};

#endif
