#ifndef DIANACLIENTRUNNER_H
#define DIANACLIENTRUNNER_H

#include "diana.grpc.pb.h"
#include <iostream>
#include <list>
#include "DianaInterface.h"
using namespace std;


class DianaClientRunner : public Diana::Service {
public:
    DianaClientRunner(string address,bool usehdd,bool deleteItems);
    virtual ~DianaClientRunner();
    void insertKeyword(string key, index_type ind);    
    void deleteKeyword(string key, index_type ind);
    void setup();
    list<index_type> searchKeyword(string key);
    double getTotalSearchCommSize();
    double getTotalUpdateCommSize();
    std::unique_ptr<DianaInterface> client_;
    double totalUpdateTime;
    double totalSearchTime;
    double clientSearchComputationTime;
    double serverSearchComputationTime;
    double clientUpdateComputationTime;
    double serverUpdateComputationTime;
private:
    bool deleteItem,usehdd;
    std::unique_ptr<Diana::Stub> stub_;
};

#endif /* DIANACLIENTRUNNER_H */