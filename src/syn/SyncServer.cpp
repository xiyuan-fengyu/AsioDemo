//
// Created by xiyuan_fengyu on 2018/1/24.
//

#include <boost/asio.hpp>
#include <thread>
#include <iostream>
#include <boost/format.hpp>

using namespace std;
using namespace boost::asio;

typedef shared_ptr<ip::tcp::socket> SocketPtr;

int main() {
    atomic<bool> isWorking(true);

    io_service service;
    ip::tcp::endpoint bindAdd(ip::tcp::v4(), 9090);
    ip::tcp::acceptor acc(service, bindAdd, true);
    vector<SocketPtr> clients;

    thread([&](){
        while (isWorking) {
            SocketPtr client(new ip::tcp::socket(service));
            acc.accept(*client);
            clients.push_back(client);
            cout << boost::format("client %s:%d connected\n") % client->remote_endpoint().address().to_string() % client->remote_endpoint().port() << endl;
            thread([&](SocketPtr curClient){
                while (isWorking) {
                    char data[1024]{};
                    boost::system::error_code errorCode;
                    size_t len = curClient->read_some(buffer(data), errorCode);
                    if (errorCode) {
                        for (auto it = clients.begin(); it < clients.end(); ++it) {
                            if (*it == curClient) {
                                clients.erase(it);
                                break;
                            }
                        }
                        cout << boost::format("client %s:%d disconnected\n") % curClient->remote_endpoint().address().to_string() % curClient->remote_endpoint().port() << endl;
                        break;
                    }
                    else if (len > 0) {
                        cout << boost::format("msg from %s:%d\n%s\n") % curClient->remote_endpoint().address().to_string() % curClient->remote_endpoint().port() % data << endl;
                    }
                }
            }, client).detach();
        }
    }).detach();

    string line;
    while (isWorking && getline(cin, line)) {
        if (line.length() > 0) {
            if (line == "quit") break;
            for (auto &client : clients) {
                boost::system::error_code errorCode;
                client->write_some(buffer(line), errorCode);
                cout << endl;
                if (errorCode) {
                    cerr << errorCode.message() << endl;
                }
            }
        }
    }
    isWorking.store(false);
    return 0;
}