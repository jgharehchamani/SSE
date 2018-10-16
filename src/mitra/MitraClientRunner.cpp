#include "MitraClientRunner.h"

#include "Client.h"

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

MitraClientRunner::MitraClientRunner(string address, bool usehdd, bool deleteFiles) {
    std::shared_ptr<grpc::Channel> channel(grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));
    stub_ = Mitra::NewStub(channel);
    client_ = make_unique<Client>(deleteFiles);
    grpc::ClientContext context;
    SetupMessage message;
    google::protobuf::Empty e;
    message.set_usehdd(usehdd);
    message.set_cleaningmode(deleteFiles);
    this->cleaningFiles = deleteFiles;

    grpc::Status status = stub_->setup(&context, message, &e);

    if (!status.ok()) {
        cout << "Setup failed: " << std::endl;
    }
}

MitraClientRunner::~MitraClientRunner() {
}

void MitraClientRunner::update(OP op, std::string keyword, int index) {
    clientUpdateComputationTime = 0;
    Utilities::startTimer(2);
    Utilities::startTimer(1);
    grpc::ClientContext context;
    UpdateMessage message;
    UpdateResponse response;
    prf_type addr, val;
    client_->updateRequest(op, keyword, index, addr, val);
    clientUpdateComputationTime += Utilities::stopTimer(1);

    message.set_address(addr.data(), addr.size());
    message.set_value(val.data(), val.size());

    grpc::Status status = stub_->update(&context, message, &response);
    serverUpdateComputationTime = response.comptime();

    if (!status.ok()) {
        cout << "Update failed:" << std::endl;
        cout << status.error_message() << std::endl;
    }
    totalUpdateTime = Utilities::stopTimer(2);
}

vector<int> MitraClientRunner::search(std::string keyword) {
    clientSearchComputationTime = 0;
    Utilities::startTimer(2);
    Utilities::startTimer(1);
    grpc::ClientContext context, context2;
    SearchMessage message;
    SearchResponse response;
    vector<prf_type> addresses, tokens;
    vector<int> result;

    prf_type k_w = client_->searchRequest(keyword, addresses);
    clientSearchComputationTime += Utilities::stopTimer(1);
    for (auto it : addresses) {
        message.add_address(it.data(), it.size());
    }

    grpc::Status status = stub_->search(&context, message, &response);
    serverSearchComputationTime = response.comptime();
    if (!status.ok()) {
        cout << "search failed:" << std::endl;
        cout << status.error_message() << std::endl;
    }
    for (int i = 0; i < response.ciphertext_size(); i++) {
        prf_type item;
        copy(response.ciphertext(i).begin(), response.ciphertext(i).end(), item.begin());
        tokens.emplace_back(item);
    }

    Utilities::startTimer(1);
    map<prf_type, prf_type> cleaningPairs;
    client_->searchProcess(tokens, k_w, result, cleaningPairs);
    clientSearchComputationTime += Utilities::stopTimer(1);
    if (cleaningFiles) {
        BatchUpdateMessage batchMessage;
        UpdateResponse batchResponse;
        for (auto p : cleaningPairs) {
            batchMessage.add_address(p.first.data(), p.first.size());
            batchMessage.add_value(p.second.data(), p.second.size());
        }
        status = stub_->batchUpdate(&context2, batchMessage, &batchResponse);
        if (!status.ok()) {
            cout << "Update failed:" << std::endl;
            cout << status.error_message() << std::endl;
        }
        serverSearchComputationTime += batchResponse.comptime();
    }
    totalSearchTime = Utilities::stopTimer(2);
    return result;
}

double MitraClientRunner::getClientStorage(int keywordLength) {
    return client_->getFileCntSize()*(keywordLength + 4);
}

