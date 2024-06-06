//
// Created by rob22 on 01.06.2024.
//

#include "../include/db_server.h"


std::atomic<bool> db_server::stop_signal = false;

int main()
{
    auto server = std::make_shared<db_server>("db_server", true);

    int port = 8080;

    std::thread server_thread([&server, &port](){
	server->do_work(port);
    });

    server_thread.detach();

    make_requests(port);

   // test_dispose_func(port);

    if (server_thread.joinable()) {
	server_thread.join();
    }

    return 0;
}
