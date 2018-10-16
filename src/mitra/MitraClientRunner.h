
#ifndef MITRACLIENTRUNNER_H
#define MITRACLIENTRUNNER_H

#include "Client.h"

#include "mitra.grpc.pb.h"

#include <memory>
#include <thread>
#include <atomic>
#include <grpc++/channel.h>

#include <mutex>
#include <condition_variable>

#ifndef AES_KEY_SIZE
#define AES_KEY_SIZE CryptoPP::AES::DEFAULT_KEYLENGTH
typedef array<uint8_t, AES_KEY_SIZE> prf_type;
#endif

class MitraClientRunner : public Mitra::Service {
public:
    MitraClientRunner(string address, bool usehdd, bool deleteFiles);
    virtual ~MitraClientRunner();
    void update(OP op, std::string keyword, int index);
    vector<int> search(std::string keyword);
    double getClientStorage(int keywordLength);
    double totalUpdateTime;
    double totalSearchTime;
    double clientSearchComputationTime;
    double serverSearchComputationTime;
    double clientUpdateComputationTime;
    double serverUpdateComputationTime;
    std::unique_ptr<Client> client_;

private:
    std::unique_ptr<Mitra::Stub> stub_;
    bool cleaningFiles;
};

#endif /* MITRACLIENTRUNNER_H */

