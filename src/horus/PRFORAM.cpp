#include "PRFORAM.hpp"
#include "utils/Utilities.h"
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <random>
#include <cmath>
#include <cassert>
#include <cstring>
#include <map>
#include <stdexcept>

PRFORAM::PRFORAM(int maxSize, bytes<Key> key)
: key(key) {
    AES::Setup();
    depth = floor(log2(maxSize / Z));
    bucketCount = pow(2, depth + 1) - 1;
    blockSize = sizeof (Box); // B
    size_t blockCount = Z * (pow(2, depth + 1) - 1);
    size_t storeBlockSize = IV + AES::GetCiphertextLength(Z * (blockSize));
    size_t storeBlockCount = blockCount;
    clen_size = AES::GetCiphertextLength((blockSize) * Z);
    plaintext_size = (blockSize) * Z;
    store = new RAMStore(storeBlockCount, storeBlockSize);
    // Intialise state of PRFORAM is new
    for (size_t i = 0; i < bucketCount; i++) {
        Bucket bucket;
        for (int z = 0; z < Z; z++) {
            bucket[z].id = 0;
            bucket[z].data.resize(blockSize, 0);
        }
        WriteBucket(i, bucket);
    }
}

PRFORAM::~PRFORAM() {
    AES::Cleanup();
}

// Fetches the array index a bucket that lise on a specific path

int PRFORAM::GetBoxOnPath(int leaf, int curDepth) {
    leaf += bucketCount / 2;
    for (int d = depth - 1; d >= curDepth; d--) {
        leaf = (leaf + 1) / 2 - 1;
    }

    return leaf;
}

// Write bucket to a single block

block PRFORAM::SerialiseBucket(Bucket bucket) {
    block buffer;

    for (int z = 0; z < Z; z++) {
        Block b = bucket[z];

        // Write block data
        buffer.insert(buffer.end(), b.data.begin(), b.data.end());
    }

    assert(buffer.size() == Z * (blockSize));

    return buffer;
}

Bucket PRFORAM::DeserialiseBucket(block buffer) {
    assert(buffer.size() == Z * (blockSize));

    Bucket bucket;

    for (int z = 0; z < Z; z++) {
        Block &block = bucket[z];

        block.data.assign(buffer.begin(), buffer.begin() + blockSize);
        Box* node = convertBlockToBox(block.data);
        block.id = node->key;
        delete node;
        buffer.erase(buffer.begin(), buffer.begin() + blockSize);
    }

    return bucket;
}

Bucket PRFORAM::ReadBucket(int index) {
    block ciphertext = store->Read(index);
    block buffer = AES::Decrypt(key, ciphertext, clen_size);
    Bucket bucket = DeserialiseBucket(buffer);
    return bucket;
}

void PRFORAM::WriteBucket(int index, Bucket bucket) {
    block b = SerialiseBucket(bucket);
    block ciphertext = AES::Encrypt(key, b, clen_size, plaintext_size);
    store->Write(index, ciphertext);
}

// Fetches blocks along a path, adding them to the stash

void PRFORAM::FetchPath(int leaf, bool batchRead) {
    for (size_t d = 0; d <= depth; d++) {
        int node = GetBoxOnPath(leaf, d);
        if (batchRead) {
            if (find(viewmap.begin(), viewmap.end(), node) != viewmap.end()) {
                continue;
            } else {
                viewmap.push_back(node);
            }
        }
        Bucket bucket = ReadBucket(node);

        for (int z = 0; z < Z; z++) {
            Block &block = bucket[z];

            if (block.id != 0) { // It isn't a dummy block   
                Box* n = convertBlockToBox(block.data);
                if (stash.count(block.id) == 0) {
                    stash[block.id] = n;
                } else {
                    delete n;
                }
            }
        }
    }
}

// Gets a list of blocks on the stash which can be placed at a specific point

std::vector<Bid> PRFORAM::GetIntersectingBlocks(int x, int curDepth) {
    std::vector<Bid> validBlocks;
    int node = GetBoxOnPath(x, curDepth);
    for (auto b : stash) {
        Bid bid = b.first;
        if (b.second != NULL && GetBoxOnPath(b.second->pos, curDepth) == node) {
            validBlocks.push_back(bid);
            if (validBlocks.size() >= Z) {
                return validBlocks;
            }
        }
    }
    return validBlocks;
}

// Greedily writes blocks from the stash to the tree, pushing blocks as deep into the tree as possible

void PRFORAM::WritePath(int leaf, int d) {
    // Write back the path

    // Find blocks that can be on this bucket
    int node = GetBoxOnPath(leaf, d);
    if (find(viewmap.begin(), viewmap.end(), node) == viewmap.end()) {

        auto validBlocks = GetIntersectingBlocks(leaf, d);
        // Write blocks to tree
        Bucket bucket;
        for (int z = 0; z < std::min((int) validBlocks.size(), Z); z++) {
            Block &block = bucket[z];
            block.id = validBlocks[z];
            Box* curnode = stash[block.id];
            block.data = convertBoxToBlock(curnode);
            delete curnode;
            stash.erase(block.id);
        }
        // Fill any empty spaces with dummy blocks
        for (int z = validBlocks.size(); z < Z; z++) {
            Block &block = bucket[z];
            block.id = 0;
            block.data.resize(blockSize, 0);
        }
        viewmap.push_back(node);
        // Write bucket to tree
        WriteBucket(node, bucket);
    }
}

// Updates the data of a block in the stash

void PRFORAM::WriteData(Bid bid, Box* node) {
    if (store->GetEmptySize() > 0) {
        stash[bid] = node;
        store->ReduceEmptyNumbers();
    } else {
        throw runtime_error("There is no more space in PRFORAM");
    }
}

Box* PRFORAM::ReadData(Bid bid) {
    if (stash.find(bid) == stash.end()) {
        return NULL;
    }
    return stash[bid];
}

// Fetches a block, allowing you to read and write  in a block

string PRFORAM::Access(Bid bid, Box*& box, int pos) {
    FetchPath(pos);
    box = ReadData(bid);
    string res = "";
    if (box != NULL) {
        res.assign(box->value.begin(), box->value.end());
        res = res.c_str();
    }
    viewmap.clear();
    for (int d = depth; d >= 0; d--) {
        WritePath(pos, d);
    }
    return res;
}

void PRFORAM::Access(Bid bid, Box*& box) {
    FetchPath(box->pos);
    WriteData(bid, box);
    viewmap.clear();
    for (int d = depth; d >= 0; d--) {
        WritePath(box->pos, d);
    }
}

string PRFORAM::ReadBox(Bid bid, int pos) {
    if (bid == 0) {
        return NULL;
    }
    if (stash.count(bid) == 0) {
        Box* box;
        auto value = Access(bid, box, pos);
        return value;
    } else {
        Box* box = stash[bid];
        string res = "";
        res.assign(box->value.begin(), box->value.end());
        res = res.c_str();
        return res;
    }
}

void PRFORAM::WriteBox(Bid bid, string value, int pos) {
    if (bid == 0) {
        throw runtime_error("Box id is not set");
    }
    Box* box = new Box();
    box->key = bid;
    std::fill(box->value.begin(), box->value.end(), 0);
    std::copy(value.begin(), value.end(), box->value.begin());
    box->pos = pos;
    Access(bid, box);
}

Box* PRFORAM::convertBlockToBox(block b) {
    Box* node = new Box();
    std::array<byte_t, sizeof (Box) > arr;
    std::copy(b.begin(), b.begin() + sizeof (Box), arr.begin());
    from_bytes(arr, *node);
    return node;
}

block PRFORAM::convertBoxToBlock(Box* node) {
    std::array<byte_t, sizeof (Box) > data = to_bytes(*node);
    block b(data.begin(), data.end());
    return b;
}

void PRFORAM::Print() {
    for (unsigned int i = 0; i < bucketCount; i++) {
        block ciphertext = store->Read(i);
        block buffer = AES::Decrypt(key, ciphertext, clen_size);
        Bucket bucket = DeserialiseBucket(buffer);
        Box* node = convertBlockToBox(bucket[0].data);
        cout << node->key << " ";
        delete node;
    }
    cout << endl;
}

vector<string> PRFORAM::batchRead(vector<pair<Bid, int> > batchQuery) {
    vector<string> result;
    set<int> leafs;
    viewmap.clear();
    for (auto item : batchQuery) {
        if (stash.count(item.second) == 0) {
            FetchPath(item.second, true);
            leafs.insert(item.second);
            Box* box;
            box = ReadData(item.first);
            string res = "";
            if (box != NULL) {
                res.assign(box->value.begin(), box->value.end());
                res = res.c_str();
            }
            result.push_back(res);
        } else {
            Box* box = stash[item.second];
            string res = "";
            res.assign(box->value.begin(), box->value.end());
            res = res.c_str();
            result.push_back(res);
        }
    }
    viewmap.clear();
    for (int d = depth; d >= 0; d--) {
        for (auto item : leafs) {
            WritePath(item, d);
        }
    }
    return result;
}

void PRFORAM::batchWrite(map<Bid, string> values, map<Bid, int> poses) {
    set<int> leafs;
    auto valuesIterator = values.begin();
    auto posesIterator = poses.begin();
    viewmap.clear();
    for (unsigned int i = 0; i < values.size(); i++) {
        Bid bid = valuesIterator->first;
        string value = valuesIterator->second;
        int pos = posesIterator->second;
        if (stash.count(pos) == 0) {
            FetchPath(pos, true);
            leafs.insert(pos);
        }
        Box* box = new Box();
        box->key = bid;
        std::fill(box->value.begin(), box->value.end(), 0);
        std::copy(value.begin(), value.end(), box->value.begin());
        box->pos = pos;
        WriteData(bid, box);
        valuesIterator++;
        posesIterator++;
    }
    viewmap.clear();
    for (int d = depth; d >= 0; d--) {
        for (auto item : leafs) {
            WritePath(item, d);
        }
    }
}
