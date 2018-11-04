#ifndef ORION_H
#define ORION_H
#include "OMAP.h"
#include<iostream>
using namespace std;

class Orion {
private:
    bool useHDD;
    map<Bid,string > setupPairs1;
    map<Bid,string > setupPairs2;
    OMAP* srch,*updt;
    map<string, int> UpdtCnt;
    map<string, int> LastIND;        
    
public:
    Bid createBid(string keyword,int number);
    void insert(string keyword, int ind);
    void setupInsert(string keyword, int ind);
    void remove(string keyword, int ind);
    void setupRemove(string keyword, int ind);
    vector<int> search(string keyword);
    Orion(bool useHDD,int maxSize);    
    virtual ~Orion();
    void beginSetup();
    void endSetup();

};

#endif /* ORION_H */

