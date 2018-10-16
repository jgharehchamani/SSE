#ifndef HORUS_H
#define HORUS_H
#include "OMAP.h"
#include "PRFORAM.hpp"
#include<iostream>
using namespace std;

class Horus {
private:
    bool useHDD;
    map<Bid,string > setupPairs1;
    map<Bid,string > setupPairs2;
    map<Bid,int > setupPairsPos;
    OMAP* OMAP_updt;    
    map<string, int> UpdtCnt;
    map<string, int> SrchCnt;
    map<string, int> Access;
    map<string, int> LastIND;
    map<string, int> latestUpdatedCounter;
    
    Bid createBid(byte_t prefix,string keyword,int number);
    Bid createBid(string keyword, int number);
    Bid createBid( string keyword,int val1,int val2,int val3) ;
    int generatePosition(string keyword,int updt_cnt,int src_cnt,int acc_cnt);
    map<string,int> poses;
    int maxSize;    
    
public:
    PRFORAM* ORAM_srch;
    void insert(string keyword, int ind);
    void setupInsert(string keyword, int ind);
    void remove(string keyword, int ind);
    void setupRemove(string keyword, int ind);
    vector<int> search(string keyword);
    Horus(bool useHDD,int maxSize);    
    virtual ~Horus();
    void beginSetup();
    void endSetup();

};

#endif /* HORUS_H */

