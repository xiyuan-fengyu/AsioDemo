//
// Created by xiyuan_fengyu on 2018/1/25.
//

#ifndef ASIODEMO_ASYNCSERVER_HPP
#define ASIODEMO_ASYNCSERVER_HPP

#include <boost/format.hpp>
#include <boost/asio.hpp>
#include <thread>
#include <iostream>
#include <unordered_map>

using namespace std;
using namespace std::placeholders;
using namespace boost::asio;

typedef shared_ptr<ip::tcp::socket> SocketPtr;

class AsyncServer;
class Reader;

class Reader {
    friend class AsyncServer;

protected:
    virtual void read(const SocketPtr &client) {}
};

class LengthFieldReader : public Reader {
    int lengthFieldWidth;

protected:

    void read(const SocketPtr &client) override {
        char lengthField[lengthFieldWidth];

    }

public:
    explicit LengthFieldReader(int lengthFieldWidth = 8) : lengthFieldWidth(lengthFieldWidth) {}

};

class AsyncServer {

    friend class Reader;

    unique_ptr<io_service> ioService;
    ip::tcp::acceptor acceptor;
    vector<SocketPtr> clients {};
    atomic<bool> isWorking {true};

    void(*listenerOnClientDisonnected)(const SocketPtr &) {};
    void(*listenerOnClientConnected)(const SocketPtr &) {};

    void accept() {
        if (!isWorking) return;
        SocketPtr client(new ip::tcp::socket(*ioService));
        acceptor.async_accept(*client, bind([this](const SocketPtr &curClient, const boost::system::error_code &errorCode){
            accept();
            if (errorCode) {
                cerr << errorCode.message() << endl;
                if (listenerOnClientDisonnected) {
                    listenerOnClientConnected(curClient);
                }
            }
            else {
                clients.push_back(curClient);
                cout << boost::format("client %s:%d connected\n") % curClient->remote_endpoint().address().to_string() % curClient->remote_endpoint().port() << endl;
                if (listenerOnClientConnected) {
                    listenerOnClientConnected(curClient);
                }

            }
        }, client, _1));
    }

public:
    explicit AsyncServer(unsigned short port) :
            ioService(new io_service()),
            acceptor(ip::tcp::acceptor(*ioService, ip::tcp::endpoint(ip::tcp::v4(), port), true))
    {
        thread([this]{
            accept();
            ioService->run();
        }).detach();
    }

    bool stillWorking() {
        return isWorking;
    }

    void shutdown() {
        isWorking.store(false);
    }

    virtual ~AsyncServer() {
        shutdown();
        for (auto &client : clients) {
            client->close();
        }
    }

    void onClientDisconnected(void(*listener)(const SocketPtr &client)) {
        listenerOnClientDisonnected = listener;
    }

    void onClientConnected(void(*listener)(const SocketPtr &client)) {
        listenerOnClientConnected = listener;
    }

};

#endif //ASIODEMO_ASYNCSERVER_HPP
