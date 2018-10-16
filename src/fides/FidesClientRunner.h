#ifndef FIDESCLIENTRUNNER_H
#define FIDESCLIENTRUNNER_H

#include "FidesClient.h"

#include "fides.grpc.pb.h"
#include <thread>
#include <atomic>
#include <grpc++/channel.h>
#include <mutex>
#include <condition_variable>

class FidesClientRunner : public Fides::Service {
public:
    FidesClientRunner(string address, bool usehdd,bool deleteItem);
    virtual ~FidesClientRunner();
    void insertKeyword(string key, index_type ind);    
    void deleteKeyword(string key, index_type ind);
    list<index_type> searchKeyword(string key);
    double getTotalSearchCommSize();
    double getTotalUpdateCommSize();
    std::unique_ptr<FidesClient> client_;
    double totalUpdateTime;
    double totalSearchTime;
    double clientSearchComputationTime;
    double serverSearchComputationTime;
    double clientUpdateComputationTime;
    double serverUpdateComputationTime;
    void setup();
    bool setupMode;
private:
    bool deleteItem,usehdd;    
    std::unique_ptr<Fides::Stub> stub_;
};

#endif /* FIDESCLIENTRUNNER_H */