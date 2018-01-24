//
// Created by xiyuan_fengyu on 2018/1/24.
//

#include <iostream>
#include <thread>

#include <boost/format.hpp>
#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio;

typedef shared_ptr<ip::tcp::socket> SocketPtr;

int main() {
    atomic<bool> isWorking(true);

    io_service ioService;
    ip::tcp::endpoint serverAdd(ip::address::from_string("127.0.0.1"), 9090);
    SocketPtr client(new ip::tcp::socket(ioService));
    thread([&](){
        client->async_connect(serverAdd, [&](const boost::system::error_code &errorCode){
            if (errorCode) {
                cerr << errorCode.message() << endl;
            }
            else {
                cout << boost::format("connect to %s:%d success\n") % client->remote_endpoint().address().to_string() % client->remote_endpoint().port() << endl;
                while (isWorking) {
                    try {
                        char msg[1024]{};
                        client->read_some(buffer(msg));
                        cout << boost::format("msg from %s:%d\n%s\n") % client->remote_endpoint().address().to_string() % client->remote_endpoint().port() % msg << endl;
                    }
                    catch (exception &e) {
                        cerr << e.what() << endl;
                        cout << "disconnected with server, press enter to quit" << endl;
                        isWorking.store(false);
                    }
                }
            }
        });
        ioService.run();
    }).detach();

    string line;
    while (isWorking && getline(cin, line)) {
        if (line.length() > 0) {
            if (line == "quit") break;
            try {
                client->write_some(buffer(line));
                cout << endl;
            }
            catch (exception &e) {
                cerr << e.what() << endl;
                break;
            }
        }
    }

    return 0;
}