#include "FidesServerRunner.h"

FidesServerRunner::FidesServerRunner() {
}

grpc::Status FidesServerRunner::setup(grpc::ServerContext* context, const SetupMessage* message, google::protobuf::Empty* e) {
    bool usehdd;
    usehdd = message->usehdd();
    this->deleteItem = message->cleaningmode();

    string client_sk_path = "tdp_sk.key";
    string client_master_key_path = "derivation_master.key";
    string server_pk_path = "tdp_pk.key";

    ifstream client_sk_in(client_sk_path.c_str());
    ifstream client_master_key_in(client_master_key_path.c_str());
    ifstream server_pk_in(server_pk_path.c_str());

    if ((client_sk_in.good() != client_master_key_in.good()) || (client_sk_in.good() != server_pk_in.good())) {
        client_sk_in.close();
        client_master_key_in.close();
        server_pk_in.close();

        throw std::runtime_error("All streams are not in the same state");
    }

    if (client_sk_in.good() == true) {
        // the files exist
        cout << "Restart client and server" << endl;

        stringstream server_pk_buf;

        server_pk_buf << server_pk_in.rdbuf();

        insertServer.reset(new SophosServer("server.dat2", server_pk_buf.str(), usehdd));

    } else {
        insertServer.reset(new SophosServer("server.dat2", 1000, message->public_key(), usehdd));

        // write keys to files
        ofstream server_pk_out(server_pk_path.c_str());
        server_pk_out << insertServer->public_key();
        server_pk_out.close();
    }
    return grpc::Status::OK;
}

grpc::Status FidesServerRunner::update(grpc::ServerContext* context, const UpdateRequestMessage* message, UpdateResponse* response) {
    UpdateRequest req;
    std::copy(message->index().begin(), message->index().end(), req.index.begin());
    std::copy(message->update_token().begin(), message->update_token().end(), req.token.begin());
    Utilities::startTimer(10);
    insertServer->update(req);
    auto t = Utilities::stopTimer(10);
    response->set_comptime(t);
    return grpc::Status::OK;
}

grpc::Status FidesServerRunner::batchUpdate(grpc::ServerContext* context, const BatchUpdateRequestMessage* message, UpdateResponse* response) {
    double totalTime=0;
    for (int i = 0; i < message->index_size(); i++) {
        UpdateRequest req;
        std::copy(message->index(i).begin(), message->index(i).end(), req.index.begin());
        std::copy(message->update_token(i).begin(), message->update_token(i).end(), req.token.begin());
        Utilities::startTimer(10);
        insertServer->update(req);
        auto t = Utilities::stopTimer(10);
        totalTime+=t;        
    }
    response->set_comptime(totalTime);
    return grpc::Status::OK;
}

grpc::Status FidesServerRunner::search(grpc::ServerContext* context, const SearchRequestMessage* message, SearchReply* reply) {
    SearchRequest req;

    req.add_count = message->add_count();
    req.derivation_key = message->derivation_key();
    std::copy(message->search_token().begin(), message->search_token().end(), req.token.begin());
    Utilities::startTimer(10);
    list<std::array<uint8_t, kUpdateTokenSize> > res_ins = insertServer->search(req, deleteItem);
    auto t = Utilities::stopTimer(10);
    reply->set_comptime(t);

    for (auto it : res_ins) {
        reply->add_index(it.data(), it.size());
    }
    return grpc::Status::OK;
}

FidesServerRunner::~FidesServerRunner() {
}

