//
// Created by xiyuan_fengyu on 2018/1/24.
//

#include <boost/format.hpp>
#include <boost/asio.hpp>
#include <thread>
#include <iostream>

using namespace std;
using namespace std::placeholders;
using namespace boost::asio;

typedef shared_ptr<ip::tcp::socket> SocketPtr;

int main() {
    atomic<bool> isWorking(true);

    io_service ioService;
    ip::tcp::endpoint bindAdd(ip::tcp::v4(), 9090);
    ip::tcp::acceptor acc(ioService, bindAdd, true);
    vector<SocketPtr> clients;

    thread([&](){
        while (isWorking) {
            SocketPtr client(new ip::tcp::socket(ioService));
            acc.async_accept(*client, bind([&](const SocketPtr &curClient, const boost::system::error_code &errorCode){
                if (errorCode) {
                    cerr << errorCode.message() << endl;
                }
                else {
                    clients.push_back(curClient);
                    cout << boost::format("client %s:%d connected\n") % curClient->remote_endpoint().address().to_string() % curClient->remote_endpoint().port() << endl;

                    while (isWorking) {
                        char data[1024]{};
                        try {
                            if (curClient->read_some(buffer(data)) > 0) {
                                cout << boost::format("msg from %s:%d\n%s\n") % curClient->remote_endpoint().address().to_string() % curClient->remote_endpoint().port() % data << endl;
                            }
                        }
                        catch (exception &e) {
                            cerr << e.what() << endl;

                            for (auto it = clients.begin(); it < clients.end(); ++it) {
                                if (*it == curClient) {
                                    clients.erase(it);
                                    break;
                                }
                            }
                            cout << boost::format("client %s:%d disconnected\n") % curClient->remote_endpoint().address().to_string() % curClient->remote_endpoint().port() << endl;
                            break;
                        }
                    }
                }
            }, client, _1));
            ioService.run();
        }
    }).detach();

    string line;
    while (isWorking && getline(cin, line)) {
        if (line.length() > 0) {
            if (line == "quit") break;
            for (auto &client : clients) {
                try {
                    client->write_some(buffer(line));
                }
                catch (exception &e) {
                    cerr << e.what() << endl;
                }
            }
        }
    }
    isWorking.store(false);
    return 0;
}