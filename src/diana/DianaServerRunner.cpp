#include "DianaServerRunner.h"

DianaServerRunner::DianaServerRunner() {
}

DianaServerRunner::~DianaServerRunner() {
}

grpc::Status DianaServerRunner::setup(grpc::ServerContext* context, const SetupMessage* request, google::protobuf::Empty* e) {
    bool usehdd = request->usehdd();
    string client_master_key_path = "diana_derivation_master.key";
    string client_kw_token_master_key_path = "diana_kw_token_master.key";

    ifstream client_master_key_in(client_master_key_path.c_str());
    ifstream client_kw_token_master_key_in(client_kw_token_master_key_path.c_str());

    if ((client_kw_token_master_key_in.good() != client_master_key_in.good())) {
        client_master_key_in.close();
        client_kw_token_master_key_in.close();

        throw std::runtime_error("All streams are not in the same state");
    }

    if (client_master_key_in.good() == true) {
        // the files exist
        cout << "Restart Diana client and server" << endl;

        stringstream client_master_key_buf, client_kw_token_key_buf;

        client_master_key_buf << client_master_key_in.rdbuf();
        client_kw_token_key_buf << client_kw_token_master_key_in.rdbuf();

        serverDel.reset(new DianaServer<update_token_type>("diana_remote_server_del.dat", usehdd));

        serverIns.reset(new DianaServer<index_type>("diana_remote_server_ins.dat", usehdd));

    } else {
        cout << "Create new Diana client-server instances" << endl;

        serverIns.reset(new DianaServer<index_type>("diana_remote_server_ins.dat", 1000, usehdd));

        serverDel.reset(new DianaServer<update_token_type>("diana_remote_server_del.dat", 1000, usehdd));

    }
    client_master_key_in.close();
    client_kw_token_master_key_in.close();
    return grpc::Status::OK;
}

grpc::Status DianaServerRunner::deleteKeyword(grpc::ServerContext* context, const DeleteRequestMessage* mes, UpdateResponse* response) {
    UpdateRequest<update_token_type> req;
    std::copy(mes->update_token().begin(), mes->update_token().end(), req.token.begin());
    std::copy(mes->index().begin(), mes->index().end(), req.index.begin());
    Utilities::startTimer(10);
    serverDel->update(req);
    auto t = Utilities::stopTimer(10);
    response->set_comptime(t);
    return grpc::Status::OK;
}

grpc::Status DianaServerRunner::insertKeyword(grpc::ServerContext* context, const InsertRequestMessage* mes, UpdateResponse* response) {
    update_token_type delCntMapKey, delCntMapValue;
    UpdateRequest<index_type> req;
    req.index = mes->index();
    std::copy(mes->update_token().begin(), mes->update_token().end(), req.token.begin());
    std::copy(mes->delete_key().begin(), mes->delete_key().end(), delCntMapKey.begin());
    std::copy(mes->delete_value().begin(), mes->delete_value().end(), delCntMapValue.begin());
    Utilities::startTimer(10);
    serverIns->update(req);
    serverDel->delCntMap[delCntMapKey] = delCntMapValue;
    auto t = Utilities::stopTimer(10);
    response->set_comptime(t);
    return grpc::Status::OK;
}

grpc::Status DianaServerRunner::batchInsertKeyword(grpc::ServerContext* context, const BatchInsertRequestMessage* mes, UpdateResponse* response) {
    double totalTime=0;
    for (int i = 0; i < mes->index_size(); i++) {
        update_token_type delCntMapKey, delCntMapValue;
        UpdateRequest<index_type> req;
        req.index = mes->index(i);
        std::copy(mes->update_token(i).begin(), mes->update_token(i).end(), req.token.begin());
        std::copy(mes->delete_key(i).begin(), mes->delete_key(i).end(), delCntMapKey.begin());
        std::copy(mes->delete_value(i).begin(), mes->delete_value(i).end(), delCntMapValue.begin());
        Utilities::startTimer(10);
        serverIns->update(req);
        serverDel->delCntMap[delCntMapKey] = delCntMapValue;
        auto t = Utilities::stopTimer(10);
        totalTime+=t;
    }
    response->set_comptime(totalTime);
    return grpc::Status::OK;
}

grpc::Status DianaServerRunner::searchDelete(grpc::ServerContext* context, const SearchRequestMessage* mes, SearchDelReply* reply) {
    SearchRequest req;
    std::list<update_token_type> resDel;
    req.add_count = mes->add_count();
    for (auto it = mes->token_list().begin(); it != mes->token_list().end(); ++it) {
        search_token_key_type st;
        std::copy(it->token().begin(), it->token().end(), st.begin());
        req.token_list.push_back(std::make_pair(st, it->depth()));
    }
    std::copy(mes->kw_token().begin(), mes->kw_token().end(), req.kw_token.begin());
    Utilities::startTimer(10);
    resDel = serverDel->search_simple_parallel(req, 1, false);
    auto t = Utilities::stopTimer(10);
    reply->set_comptime(t);
    for (auto it : resDel) {
        auto targetValue = serverDel->delCntMap[it];
        reply->add_result(targetValue.data(), targetValue.size());
    }
    return grpc::Status::OK;
}

grpc::Status DianaServerRunner::searchInsert(grpc::ServerContext* context, const SearchRequestMessage* mes, SearchInsReply* reply) {
    SearchRequest req;
    std::list<index_type> resIns;
    req.add_count = mes->add_count();
    for (int i = 0; i < mes->token_list_size(); i++) {
        SearchToken searchToen = mes->token_list(i);
        search_token_key_type st;
        std::copy(searchToen.token().begin(), searchToen.token().end(), st.begin());
        req.token_list.push_back(std::make_pair(st, searchToen.depth()));
    }

    std::copy(mes->kw_token().begin(), mes->kw_token().end(), req.kw_token.begin());
    Utilities::startTimer(10);
    resIns = serverIns->search_simple_parallel(req, 1, false);
    auto t = Utilities::stopTimer(10);
    reply->set_comptime(t);
    for (auto it : resIns) {
        reply->add_result((uint64_t) it);
    }
    return grpc::Status::OK;
}


