#include "FidesClientRunner.h"

#include "utils/thread_pool.hpp"
#include "utils/utils.hpp"
#include "utils/logger.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <thread>
#include <fstream>

#include <grpc/grpc.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>

FidesClientRunner::FidesClientRunner(string address, bool usehdd, bool deleteItem) {
    this->deleteItem = deleteItem;
    std::shared_ptr<grpc::Channel> channel(grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));
    stub_ = Fides::NewStub(channel);
    client_ = make_unique<FidesClient>(usehdd, deleteItem);
    setupMode = false;
    this->usehdd = usehdd;
}

void FidesClientRunner::setup() {
    grpc::ClientContext context;
    SetupMessage message;
    google::protobuf::Empty e;
    message.set_usehdd(usehdd);
    message.set_cleaningmode(deleteItem);

    grpc::Status status = stub_->setup(&context, message, &e);

    if (!status.ok()) {
        cout << "Setup failed: " << std::endl;
    }
}

FidesClientRunner::~FidesClientRunner() {
}

void FidesClientRunner::insertKeyword(string keyword, index_type index) {
    clientUpdateComputationTime = 0;
    Utilities::startTimer(2);
    Utilities::startTimer(1);
    grpc::ClientContext context;
    UpdateRequestMessage message;
    UpdateResponse response;

    UpdateRequest req;
    client_->insertKeyword(keyword, index, req);
    clientUpdateComputationTime += Utilities::stopTimer(1);
    message.set_update_token(req.token.data(), req.token.size());
    message.set_index(req.index.data(), req.index.size());
    if (!setupMode) {
        grpc::Status status = stub_->update(&context, message, &response);
        serverUpdateComputationTime = response.comptime();

        if (!status.ok()) {
            cout << "Update failed:" << std::endl;
        }
    }
    totalUpdateTime = Utilities::stopTimer(2);
}

void FidesClientRunner::deleteKeyword(string keyword, index_type index) {
    clientUpdateComputationTime = 0;
    Utilities::startTimer(2);
    Utilities::startTimer(1);
    grpc::ClientContext context;
    UpdateRequestMessage message;
    UpdateResponse response;

    UpdateRequest req;
    client_->deleteKeyword(keyword, index, req);
    clientUpdateComputationTime += Utilities::stopTimer(1);
    message.set_update_token(req.token.data(), req.token.size());
    message.set_index(req.index.data(), req.index.size());

    grpc::Status status = stub_->update(&context, message, &response);
    serverUpdateComputationTime = response.comptime();
    if (!status.ok()) {
        cout << "Update failed:" << std::endl;
    }
    totalUpdateTime = Utilities::stopTimer(2);
}

list<index_type> FidesClientRunner::searchKeyword(string keyword) {
    clientSearchComputationTime = 0;
    Utilities::startTimer(2);
    Utilities::startTimer(1);
    SearchRequest req;
    client_->searchRequest(keyword, req);
    clientSearchComputationTime += Utilities::stopTimer(1);
    grpc::ClientContext context;
    SearchRequestMessage message;
    SearchReply reply;


    message.set_add_count(req.add_count);
    message.set_derivation_key(req.derivation_key);
    message.set_search_token(req.token.data(), req.token.size());

    list<std::array<uint8_t, kUpdateTokenSize> > ciphers;

    stub_->search(&context, message, &reply);
    serverSearchComputationTime = reply.comptime();
    for (int i = 0; i < reply.index_size(); i++) {
        std::array<uint8_t, kUpdateTokenSize> cipher;
        std::copy(reply.index(i).begin(), reply.index(i).end(), cipher.begin());
        ciphers.push_back(cipher);
    }
    Utilities::startTimer(1);
    list<index_type> res = client_->searchProcess(ciphers);
    clientSearchComputationTime += Utilities::stopTimer(1);
    if (deleteItem) {
        BatchUpdateRequestMessage batchMessage;
        UpdateResponse batchResponse;
        grpc::ClientContext batchContext;
        for (auto item : res) {
            Utilities::startTimer(1);
            UpdateRequest breq;
            client_->insertKeyword(keyword, item, breq);
            clientSearchComputationTime += Utilities::stopTimer(1);
            batchMessage.add_update_token(breq.token.data(), breq.token.size());
            batchMessage.add_index(breq.index.data(), breq.index.size());
            client_->setTotalSearchCommSize(client_->getTotalSearchCommSize() + client_->getTotalUpdateCommSize());
        }
        grpc::Status status = stub_->batchUpdate(&batchContext, batchMessage, &batchResponse);
        serverSearchComputationTime = batchResponse.comptime();

        if (!status.ok()) {
            cout << "Update failed:" << std::endl;
        }
    }
    totalSearchTime = Utilities::stopTimer(2);
    return res;
}

double FidesClientRunner::getTotalSearchCommSize() {
    return client_->getTotalSearchCommSize();
}

double FidesClientRunner::getTotalUpdateCommSize() {
    return client_->getTotalUpdateCommSize();
}

