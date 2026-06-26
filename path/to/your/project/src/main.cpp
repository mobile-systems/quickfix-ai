#include <iostream>
#include <string>
#include <quickfix/Application.h>
#include <quickfix/Message.h>
#include <quickfix/Session.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/FieldMap.h>

using namespace FIX;

class MyApplication : public Application {
public:
    void onCreate(const SessionID& sessionID) override {
        std::cout << "Session created" << std::endl;
    }

    void onLogon(const SessionID& sessionID) override {
        std::cout << "Session logged on" << std::endl;
    }

    void onLogout(const SessionID& sessionID) override {
        std::cout << "Session logged off" << std::endl;
    }

    void toAdmin(Message& message, const SessionID& sessionID) override {}

    void fromAdmin(const Message& message, const SessionID& sessionID) override {}

    void toApp(Message& message, const SessionID& sessionID) override {}

    void fromApp(const Message& message, const SessionID& sessionID) override {
        if (message.getHeader().getField(35) == "W") {
            // Handle market data message
            std::string symbol = message.getField(55);
            std::string bid = message.getField(200);
            std::string ask = message.getField(201);
            std::string time = message.getField(268);

            // Insert market data into PostgreSQL database
            // Your code to connect to the database and execute an INSERT query
        }
    }
};

int main() {
    try {
        Application* myApp = new MyApplication;
        SessionSettings settings("config.cfg");
        FileStoreFactory storeFactory(settings);
        LogFactory logFactory(settings);
        initiator = SocketInitiator(*myApp, storeFactory, settings, logFactory);
        initiator.start();
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}