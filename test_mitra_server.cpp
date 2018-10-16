#include "mitra/MitraServerRunner.h"
#include "utils/logger.hpp"

#include <grpc++/server.h>
#include <grpc++/server_builder.h>

#include <sse/crypto/utils.hpp>

#include <stdio.h>
#include <csignal>
#include <unistd.h>

int main(int argc, char** argv) {
    sse::crypto::init_crypto_lib();

    MitraServerRunner service;

    grpc::ServerBuilder builder;
    builder.AddListeningPort("0.0.0.0:4241", grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    server->Wait();
    sse::crypto::cleanup_crypto_lib();
    return 0;
}