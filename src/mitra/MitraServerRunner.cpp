#include "MitraServerRunner.h"

MitraServerRunner::MitraServerRunner() {   
}

MitraServerRunner::~MitraServerRunner() {
}

grpc::Status MitraServerRunner::setup(grpc::ServerContext* context, const SetupMessage* request, google::protobuf::Empty* e) {
    bool usehdd,cleaningMode;
    cleaningMode = request->cleaningmode();
    usehdd = request->usehdd();
    server_ = make_unique<Server>(usehdd,cleaningMode);
    return grpc::Status::OK;
}
grpc::Status MitraServerRunner::update(grpc::ServerContext* context, const UpdateMessage* mes, UpdateResponse* response) {
    if (!server_) {
        // problem, the server is already set up
        return grpc::Status(grpc::FAILED_PRECONDITION, "The server is not set up");
    }
    prf_type addr, val;
    std::copy(mes->address().begin(), mes->address().end(), addr.begin());
    std::copy(mes->value().begin(), mes->value().end(), val.begin());
    Utilities::startTimer(10);
    server_->update(addr, val);
    auto t = Utilities::stopTimer(10);
    response->set_comptime(t);
    return grpc::Status::OK;
}

grpc::Status MitraServerRunner::batchUpdate(grpc::ServerContext* context, const BatchUpdateMessage* mes, UpdateResponse* response) {
    if (!server_) {
        // problem, the server is already set up
        return grpc::Status(grpc::FAILED_PRECONDITION, "The server is not set up");
    }
    double totalTime = 0;
    for (int i = 0; i < mes->address_size(); i++) {
        prf_type addr, value;
        copy(mes->address(i).begin(), mes->address(i).end(), addr.begin());
        copy(mes->value(i).begin(), mes->value(i).end(), value.begin());
        Utilities::startTimer(10);
        server_->update(addr, value);
        auto t = Utilities::stopTimer(10);
        totalTime += t;        
    }
    response->set_comptime(totalTime);
    return grpc::Status::OK;
}

grpc::Status MitraServerRunner::search(grpc::ServerContext* context, const SearchMessage* message, SearchResponse* response) {
    if (!server_) {
        // problem, the server is already set up
        return grpc::Status(grpc::FAILED_PRECONDITION, "The server is not set up");
    }
    vector<prf_type> addresses, tokens;
    for (int i = 0; i < message->address_size(); i++) {
        prf_type item;
        copy(message->address(i).begin(), message->address(i).end(), item.begin());
        addresses.emplace_back(item);
    }
    Utilities::startTimer(10);
    tokens = server_->search(addresses);
    auto t = Utilities::stopTimer(10);
    response->set_comptime(t);
    for (auto it : tokens) {
        response->add_ciphertext(it.data(), it.size());
    }
    return grpc::Status::OK;
}