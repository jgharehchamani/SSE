#include "JanusInterface.h"
#include "../utils/Utilities.h"
JanusInterface::JanusInterface(bool usehdd,int id) {
    init_crypto_lib();
    sse::logger::set_severity(sse::logger::ERROR);

    string client_master_key_path = "janus_master.key"+to_string(id);
    ifstream client_master_key_in(client_master_key_path.c_str());

    if (client_master_key_in.good() == true) {
        // the files exist
        cout << "Restart Janus client and server" << endl;

        stringstream client_master_key_buf, client_kw_token_key_buf;

        client_master_key_buf << client_master_key_in.rdbuf();

        cout << "MASTER KEY: " << hex_string(client_master_key_buf.str()) << "\n";

        client.reset(new JanusClient("janus_client.search.dat"+to_string(id), "janus_client.add.dat"+to_string(id), "janus_client.del.dat"+to_string(id), client_master_key_buf.str()));

        server.reset(new JanusServer("janus_server.add.dat"+to_string(id), "janus_server.del.dat"+to_string(id), "janus_server.cache.dat"+to_string(id),usehdd));

    } else {
        cout << "Create new Janus client-server instances" << endl;

        client.reset(new JanusClient("janus_client.search.dat"+to_string(id), "janus_client.add.dat"+to_string(id), "janus_client.del.dat"+to_string(id)));

        server.reset(new JanusServer("janus_server.add.dat"+to_string(id), "janus_server.del.dat"+to_string(id), "janus_server.cache.dat"+to_string(id),usehdd));

        // write keys to files

        ofstream client_master_key_out(client_master_key_path.c_str());
        client_master_key_out << client->master_key();
        client_master_key_out.close();
    }
    client_master_key_in.close();
}

JanusInterface::~JanusInterface() {
    cleanup_crypto_lib();
}

void JanusInterface::deleteKeyword(string key, index_type ind) {
    DeletionRequest del_req;
    del_req = client->deletion_request(key, ind);
    totalUpdateCommSize = (sizeof (del_req.index) + sizeof (del_req.token.begin()) * del_req.token.size() + sizeof (del_req.token));
    server->delete_entry(del_req);
}

void JanusInterface::insertKeyword(string key, index_type ind) {
    InsertionRequest add_req;
    add_req = client->insertion_request(key, ind);
    totalUpdateCommSize = (sizeof (add_req.index) + sizeof (add_req.token.begin()) * add_req.token.size() + sizeof (add_req.token));
    server->insert_entry(add_req);
}

list<index_type> JanusInterface::searchKeyword(string key) {
    Utilities::startTimer(200);
    SearchRequest s_req;
    s_req = client->search_request(key);
    std::list<index_type> res = server->search_parallel(s_req, 8);
    auto time = Utilities::stopTimer(200);
    cout<<"communication size:"<<(sizeof(res) + (sizeof(index_type) * res.size())+sizeof(s_req)+(sizeof(s_req.deletion_search_request.add_count)+sizeof(s_req.deletion_search_request.kw_token)+
            s_req.deletion_search_request.token_list.size()*16+sizeof(uint8_t))+sizeof(s_req.first_key_share)+sizeof(s_req.keyword_token)+
            (sizeof(s_req.insertion_search_request.add_count)+sizeof(s_req.insertion_search_request.kw_token)+
            s_req.insertion_search_request.token_list.size()*16+sizeof(uint8_t)))<<endl;
    cout << "search time: " << to_string(time) << endl;
    return res;
}
