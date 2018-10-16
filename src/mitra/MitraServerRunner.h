
#ifndef MITRASERVERRUNNER_H
#define MITRASERVERRUNNER_H

#include "Server.h"
#include "mitra.grpc.pb.h"

#include <string>
#include <memory>
#include <mutex>

#include <grpc++/server.h>
#include <grpc++/server_context.h>

#ifndef AES_KEY_SIZE
#define AES_KEY_SIZE CryptoPP::AES::DEFAULT_KEYLENGTH
typedef array<uint8_t, AES_KEY_SIZE> prf_type;
#endif

class MitraServerRunner : public Mitra::Service {
public:
    MitraServerRunner();
    virtual ~MitraServerRunner();
    grpc::Status setup(grpc::ServerContext* context, const SetupMessage* request, google::protobuf::Empty* e) ;
    grpc::Status update(grpc::ServerContext* context, const UpdateMessage* request, UpdateResponse* response) ;
    grpc::Status batchUpdate(grpc::ServerContext* context, const BatchUpdateMessage* request, UpdateResponse* response) ;
    grpc::Status search(grpc::ServerContext* context, const SearchMessage* mes, SearchResponse* res) ;
private:
    std::unique_ptr<Server> server_;
};

#endif /* MITRASERVERRUNNER_H */

