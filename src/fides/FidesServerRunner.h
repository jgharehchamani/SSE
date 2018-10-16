#ifndef FIDESSERVERRUNNER_H
#define FIDESSERVERRUNNER_H

#include "fides.grpc.pb.h"

#include <string>
#include <memory>
#include <mutex>
#include <string>
#include <iostream>
#include <grpc++/server.h>
#include <grpc++/server_context.h>
#include "sophos_server.hpp"
using namespace sse::sophos;
using namespace std;

class FidesServerRunner : public Fides::Service {
public:
    FidesServerRunner();
    virtual ~FidesServerRunner();
    grpc::Status setup(grpc::ServerContext* context, const SetupMessage* request, google::protobuf::Empty* e) ;
    grpc::Status update(grpc::ServerContext* context, const UpdateRequestMessage* request, UpdateResponse* response) ;
    grpc::Status batchUpdate(grpc::ServerContext* context, const BatchUpdateRequestMessage* request, UpdateResponse* response) ;
    grpc::Status search(grpc::ServerContext* context, const SearchRequestMessage* mes, SearchReply* res) ;
private:
    unique_ptr<SophosServer> insertServer;
    bool deleteItem;
};

#endif /* FIDESSERVERRUNNER_H */