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
    ip::tcp::endpoint serverAdd(ip::address::from_string("127.0.0.1"), 9090);
    SocketPtr client(new ip::tcp::socket(service));
    try {
        client->connect(serverAdd);
    }
    catch (exception &e) {
        cerr << e.what() << endl;
        return -1;
    }

    thread([&](){
        while (isWorking) {
            char data[1024]{};
            boost::system::error_code errorCode;
            size_t len = client->read_some(buffer(data), errorCode);
            if (errorCode) {
                cerr << errorCode.message() << endl;
                cout << "disconnected with server, press enter to quit" << endl;
                isWorking.store(false);
            }
            else if (len > 0) {
                cout << boost::format("msg from %s:%d\n%s\n") % client->remote_endpoint().address().to_string() % client->remote_endpoint().port() % data << endl;
            }
        }
    }).detach();

    string line;
    while (isWorking && getline(cin, line)) {
        if (line.length() > 0) {
            if (line == "quit") break;
            boost::system::error_code errorCode;
            client->write_some(buffer(line), errorCode);
            cout << endl;
            if (errorCode) {
                cerr << errorCode.message() << endl;
            }
        }
    }
    isWorking.store(false);
    return 0;
}