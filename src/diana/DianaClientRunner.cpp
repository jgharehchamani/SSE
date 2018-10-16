#include "DianaClientRunner.h"

#include "utils/thread_pool.hpp"
#include "utils/utils.hpp"
#include "utils/logger.hpp"
#include "DianaServerRunner.h"
#include <grpc++/channel.h>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include <memory>
#include <random>
#include <string>
#include <thread>
#include <fstream>

#include <grpc/grpc.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>

DianaClientRunner::DianaClientRunner(string address,bool usehdd, bool deleteItems) {
    this->deleteItem = deleteItems;
    this->usehdd=usehdd;
    std::shared_ptr<grpc::Channel> channel(grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));
    stub_ = Diana::NewStub(channel);
    client_ = make_unique<DianaInterface>(usehdd, false, deleteItems);
}

void DianaClientRunner::setup() {
    grpc::ClientContext context;
    SetupMessage message;
    google::protobuf::Empty e;
    message.set_usehdd(usehdd);

    grpc::Status status = stub_->setup(&context, message, &e);

    if (!status.ok()) {
        cout << "Setup failed: " << std::endl;
    }
}

DianaClientRunner::~DianaClientRunner() {
}

void DianaClientRunner::insertKeyword(string keyword, index_type index) {
    clientUpdateComputationTime = 0;
    Utilities::startTimer(2);
    Utilities::startTimer(1);
    grpc::ClientContext context;
    InsertRequestMessage message;
    UpdateResponse response;

    UpdateRequest<index_type> req;
    update_token_type delCntMapKey, delCntMapValue;
    client_->insertKeyword(keyword, index, req, delCntMapKey, delCntMapValue);
    clientUpdateComputationTime += Utilities::stopTimer(1);
    message.set_update_token(req.token.data(), req.token.size());
    message.set_index(req.index);
    message.set_delete_key(delCntMapKey.data(), delCntMapKey.size());
    message.set_delete_value(delCntMapValue.data(), delCntMapValue.size());
    if (!client_->isSetupMode()) {
        grpc::Status status = stub_->insertKeyword(&context, message, &response);
        serverUpdateComputationTime = response.comptime();
        if (!status.ok()) {
            cout << "Update failed:" << std::endl;
        }
    }
    totalUpdateTime = Utilities::stopTimer(2);
}

void DianaClientRunner::deleteKeyword(string keyword, index_type index) {
    clientUpdateComputationTime = 0;
    Utilities::startTimer(2);
    Utilities::startTimer(1);
    grpc::ClientContext context;
    DeleteRequestMessage message;
    UpdateResponse response;

    UpdateRequest<update_token_type> req;
    client_->deleteKeyword(keyword, index, req);
    clientUpdateComputationTime += Utilities::stopTimer(1);
    message.set_update_token(req.token.data(), req.token.size());
    message.set_index(req.index.data(), req.index.size());


    grpc::Status status = stub_->deleteKeyword(&context, message, &response);
    serverUpdateComputationTime = response.comptime();
    if (!status.ok()) {
        cout << "Update failed:" << std::endl;
    }
    totalUpdateTime = Utilities::stopTimer(2);
}

list<index_type> DianaClientRunner::searchKeyword(string keyword) {
    clientSearchComputationTime = 0;
    Utilities::startTimer(2);
    Utilities::startTimer(1);
    SearchRequest delReq, insReq;
    client_->searchDeletedKeyword(keyword, delReq);
    clientSearchComputationTime += Utilities::stopTimer(1);

    grpc::ClientContext delContext, insContext;
    SearchRequestMessage delMessage, insMessage;
    SearchDelReply delReply;
    SearchInsReply insReply;

    delMessage.set_add_count(delReq.add_count);
    delMessage.set_kw_token(delReq.kw_token.data(), delReq.kw_token.size());
    for (auto it : delReq.token_list) {
        SearchToken* searchToken = delMessage.add_token_list();
        searchToken->set_depth(it.second);
        searchToken->set_token(it.first.data(), it.first.size());
    }
    grpc::Status status = stub_->searchDelete(&delContext, delMessage, &delReply);
    serverSearchComputationTime = delReply.comptime();

    if (!status.ok()) {
        cout << "search delete failed:" << std::endl;
    }
    list<std::array<uint8_t, kUpdateTokenSize> > ciphers;
    for (int i = 0; i < delReply.result_size(); i++) {
        std::array<uint8_t, kUpdateTokenSize> cipher;
        std::copy(delReply.result(i).begin(), delReply.result(i).end(), cipher.begin());
        ciphers.push_back(cipher);
    }
    Utilities::startTimer(1);
    client_->searchInsertedKeyword(keyword, ciphers, insReq);
    clientSearchComputationTime += Utilities::stopTimer(1);
    insMessage.set_add_count(insReq.add_count);
    insMessage.set_kw_token(insReq.kw_token.data(), insReq.kw_token.size());
    for (auto it : insReq.token_list) {
        SearchToken* searchToken = insMessage.add_token_list();
        searchToken->set_depth(it.second);
        searchToken->set_token(it.first.data(), it.first.size());
    }

    status = stub_->searchInsert(&insContext, insMessage, &insReply);
    serverSearchComputationTime += insReply.comptime();

    if (!status.ok()) {
        cout << "search delete failed:" << std::endl;
    }
    std::list<uint64_t> results;
    for (int i = 0; i < insReply.result_size(); i++) {
        results.push_back(insReply.result(i));
    }
    Utilities::startTimer(1);
    client_->processSearch(results, keyword);
    clientSearchComputationTime += Utilities::stopTimer(1);

    if (deleteItem) {
        grpc::ClientContext batchContext;
        BatchInsertRequestMessage batchMessage;
        UpdateResponse batchResponse;
        for (auto item : results) {
            UpdateRequest<index_type> breq;
            update_token_type delCntMapKey, delCntMapValue;
            Utilities::startTimer(1);
            client_->insertKeyword(keyword, item, breq, delCntMapKey, delCntMapValue);
            batchMessage.add_update_token(breq.token.data(), breq.token.size());
            clientSearchComputationTime += Utilities::stopTimer(1);
            batchMessage.add_index(breq.index);
            batchMessage.add_delete_key(delCntMapKey.data(), delCntMapKey.size());
            batchMessage.add_delete_value(delCntMapValue.data(), delCntMapValue.size());
        }
        grpc::Status status = stub_->batchInsertKeyword(&batchContext, batchMessage, &batchResponse);
        serverSearchComputationTime += batchResponse.comptime();
        if (!status.ok()) {
            cout << "Update failed:" << std::endl;
        }
    }
    totalSearchTime = Utilities::stopTimer(2);
    return results;
}

double DianaClientRunner::getTotalSearchCommSize() {
    return client_->getTotalSearchCommSize();
}

double DianaClientRunner::getTotalUpdateCommSize() {
    return client_->getTotalUpdateCommSize();
}


