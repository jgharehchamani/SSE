#ifndef DIANASERVERRUNNER_H
#define DIANASERVERRUNNER_H

#include "diana.grpc.pb.h"

#include <string>
#include <memory>
#include <mutex>
#include <string>
#include <iostream>
#include <grpc++/server.h>
#include <grpc++/server_context.h>
#include "diana/diana_server.hpp"
using namespace sse::diana;
using namespace std;

typedef uint64_t index_type;

class DianaServerRunner : public Diana::Service {
public:
    DianaServerRunner();
    virtual ~DianaServerRunner();
    grpc::Status setup(grpc::ServerContext* context, const SetupMessage* request, google::protobuf::Empty* e) ;
    grpc::Status insertKeyword(grpc::ServerContext* context, const InsertRequestMessage* request, UpdateResponse* response) ;
    grpc::Status batchInsertKeyword(grpc::ServerContext* context, const BatchInsertRequestMessage* request, UpdateResponse* response) ;
    grpc::Status deleteKeyword(grpc::ServerContext* context, const DeleteRequestMessage* request, UpdateResponse* response) ;
    grpc::Status searchDelete(grpc::ServerContext* context, const SearchRequestMessage* mes, SearchDelReply* res) ;
    grpc::Status searchInsert(grpc::ServerContext* context, const SearchRequestMessage* mes, SearchInsReply* res) ;
private:
    unique_ptr<DianaServer<update_token_type> > serverDel;
    unique_ptr<DianaServer<index_type> > serverIns;
    bool deleteItem;
};

#endif /* DIANASERVERRUNNER_H */