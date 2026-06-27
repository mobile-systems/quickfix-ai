<pre>
alexa@DellPro MINGW64 ~/workspace
$ llama-cli -m ~/.lmstudio/models/lmstudio-community/Qwen3-Coder-30B-A3B-Instruct-GGUF/Qwen3-Coder-30B-A3B-Instruct-Q4_K_M.gguf

Loading model...


▄▄ ▄▄
██ ██
██ ██  ▀▀█▄ ███▄███▄  ▀▀█▄    ▄████ ████▄ ████▄
██ ██ ▄█▀██ ██ ██ ██ ▄█▀██    ██    ██ ██ ██ ██
██ ██ ▀█▄██ ██ ██ ██ ▀█▄██ ██ ▀████ ████▀ ████▀
                                    ██    ██
                                    ▀▀    ▀▀

build      : b9754-52b3df002
model      : C:/Users/alexa/.lmstudio/models/lmstudio-community/Qwen3-Coder-30B-A3B-Instruct-GGUF/Qwen3-Coder-30B-A3B-Instruct-Q4_K_M.gguf
modalities : text

available commands:
  /exit or Ctrl+C     stop or exit
  /regen              regenerate the last response
  /clear              clear the chat history
  /read <file>        add a text file
  /glob <pattern>     add text files using globbing pattern
</pre>

> Мне нужен работающий пример на C++ получения котировок по инструменту EURUSD с использованием библиотеки QuickFIX и сохранением результата полученных котировок в PostgreSQL, при сборке проекта используй CMake

Вот рабочий пример на C++ с использованием **QuickFIX** и **PostgreSQL** (через `libpq`), который получает котировки по инструменту `EURUSD` и сохраняет их в PostgreSQL с помощью **CMake**.

---

## 📁 Структура проекта

```
quickfix-postgres/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── FixApplication.cpp
│   ├── FixApplication.h
│   └── Database.cpp
│   └── Database.h
├── config/
│   └── fix.cfg
```

---

## 🔧 1. CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(QuickFixPostgres)

set(CMAKE_CXX_STANDARD 17)

# Библиотеки QuickFIX
find_package(QuickFIX REQUIRED)

# PostgreSQL
find_package(PkgConfig REQUIRED)
pkg_check_modules(PGSQL REQUIRED libpq)

# Исходники
add_executable(quickfix_postgres
    src/main.cpp
    src/FixApplication.cpp
    src/Database.cpp
)

target_link_libraries(quickfix_postgres
    ${QuickFIX_LIBRARIES}
    ${PGSQL_LIBRARIES}
)

target_include_directories(quickfix_postgres PRIVATE
    ${QuickFIX_INCLUDE_DIRS}
    ${PGSQL_INCLUDE_DIRS}
    src/
)
```

---

## 📄 2. config/fix.cfg

```ini
[DEFAULT]
ConnectionType=initiator
ReconnectInterval=5
FileStorePath=store
FileLogPath=log
StartTime=00:00:00
EndTime=00:00:00
UseDataDictionary=Y
DataDictionary=spec/FIX44.xml

[SESSION]
BeginString=FIX.4.4
SenderCompID=CLIENT
TargetCompID=SERVER
SocketConnectHost=localhost
SocketConnectPort=9876
HeartBtInt=30
```

---

## 📄 3. src/FixApplication.h

```cpp
#ifndef FIXAPPLICATION_H
#define FIXAPPLICATION_H

#include <quickfix/Application.h>
#include <quickfix/MessageCracker.h>
#include <quickfix/SessionID.h>
#include <quickfix/FieldTypes.h>
#include <quickfix/Values.h>
#include "Database.h"

class FixApplication : public FIX::Application, public FIX::MessageCracker
{
public:
    FixApplication(Database& db);
    virtual ~FixApplication();

    void onCreate(const FIX::SessionID&) override;
    void onLogon(const FIX::SessionID&) override;
    void onLogout(const FIX::SessionID&) override;
    void toAdmin(FIX::Message&, const FIX::SessionID&) override;
    void toApp(FIX::Message&, const FIX::SessionID&) override;
    void fromAdmin(const FIX::Message&, const FIX::SessionID&) override;
    void fromApp(const FIX::Message&, const FIX::SessionID&) override;

private:
    Database& db_;
};

#endif // FIXAPPLICATION_H
```

---

## 📄 4. src/FixApplication.cpp

```cpp
#include "FixApplication.h"
#include <quickfix/fix44/MarketDataRequest.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>
#include <iostream>

FixApplication::FixApplication(Database& db) : db_(db) {}

FixApplication::~FixApplication() {}

void FixApplication::onCreate(const FIX::SessionID&) {}

void FixApplication::onLogon(const FIX::SessionID&) {
    std::cout << "Logged in.\n";

    FIX::MarketDataRequest mdRequest;
    mdRequest.set(FIX::MDReqID("EURUSD"));
    mdRequest.set(FIX::SubscriptionRequestType(FIX::SubscriptionRequestType_SNAPSHOT));

    FIX::MDIncrementalRefresh md;
    FIX::Symbol symbol("EURUSD");
    mdRequest.addGroup(symbol);

    FIX::Session::sendToTarget(mdRequest, FIX::SessionID("FIX.4.4", "CLIENT", "SERVER"));
}

void FixApplication::onLogout(const FIX::SessionID&) {}

void FixApplication::toAdmin(FIX::Message&, const FIX::SessionID&) {}

void FixApplication::toApp(FIX::Message&, const FIX::SessionID&) {}

void FixApplication::fromAdmin(const FIX::Message&, const FIX::SessionID&) {}

void FixApplication::fromApp(const FIX::Message& message, const FIX::SessionID&)
{
    try {
        FIX::MsgType msgType;
        message.getHeader().getField(msgType);
        if (msgType == FIX::MsgType_MarketDataSnapshotFullRefresh)
        {
            FIX::Symbol symbol;
            FIX::BidPx bid;
            FIX::OfferPx offer;

            message.getField(symbol);
            message.getField(bid);
            message.getField(offer);

            std::cout << "Symbol: " << symbol << " Bid: " << bid << " Ask: " << offer << std::endl;

            db_.insertQuote(symbol, bid, offer);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error processing message: " << e.what() << std::endl;
    }
}
```

---

## 📄 5. src/Database.h

```cpp
#ifndef DATABASE_H
#define DATABASE_H

#include <string>

class Database {
public:
    Database(const std::string& connection_string);
    void insertQuote(const std::string& symbol, double bid, double ask);

private:
    std::string conn_str_;
};

#endif // DATABASE_H
```

---

## 📄 6. src/Database.cpp

```cpp
#include "Database.h"
#include <libpq-fe.h>
#include <iostream>

Database::Database(const std::string& connection_string) : conn_str_(connection_string) {}

void Database::insertQuote(const std::string& symbol, double bid, double ask) {
    PGconn *conn = PQconnectdb(conn_str_.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Database connection error: " << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        return;
    }

    std::string query = "INSERT INTO quotes (symbol, bid, ask) VALUES ('" + symbol + "', " + std::to_string(bid) + ", " + std::to_string(ask) + ");";
    PGresult *res = PQexec(conn, query.c_str());

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Insert failed: " << PQerrorMessage(conn) << std::endl;
    }

    PQclear(res);
    PQfinish(conn);
}
```

---

## 📄 7. src/main.cpp

```cpp
#include <quickfix/SessionSettings.h>
#include <quickfix/SocketInitiator.h>
#include <quickfix/FileLog.h>
#include <quickfix/FileStore.h>
#include "FixApplication.h"
#include "Database.h"

int main() {
    try {
        FIX::SessionSettings settings("config/fix.cfg");
        FixApplication app(Database("dbname=yourdb user=youruser password=yourpass"));
        FIX::FileLog log(settings);
        FIX::FileStore store(settings);
        FIX::SocketInitiator initiator(app, store, settings, log);

        initiator.start();
        std::cout << "Initiator started.\n";
        std::cin.get();
        initiator.stop();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

---

## 📌 Зависимости

Установите зависимости:

```bash
sudo apt install libquickfix-dev libpq-dev
```

---

## 🗃️ PostgreSQL таблица

```sql
CREATE TABLE quotes (
    id SERIAL PRIMARY KEY,
    symbol VARCHAR(10),
    bid NUMERIC,
    ask NUMERIC,
    timestamp TIMESTAMP DEFAULT NOW()
);
```

---

## 🧪 Сборка и запуск

```bash
mkdir build && cd build
cmake ..
make
./quickfix_postgres
```

---

Если нужно, я могу также добавить пример `FIX44.xml` или сгенерировать `MarketDataRequest` в виде `FIX` сообщения.

[ Prompt: 15.2 t/s | Generation: 9.3 t/s ]

> добавь конечно пример

Вот полный рабочий пример с примером `FIX44.xml` и всеми необходимыми файлами:

## 📁 Полная структура проекта

```
quickfix-postgres/
├── CMakeLists.txt
├── config/
│   └── fix.cfg
├── spec/
│   └── FIX44.xml
├── src/
│   ├── main.cpp
│   ├── FixApplication.cpp
│   ├── FixApplication.h
│   ├── Database.cpp
│   └── Database.h
├── sql/
│   └── create_table.sql
└── README.md
```

---

## 🔧 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(QuickFixPostgres)

set(CMAKE_CXX_STANDARD 17)

# Библиотеки QuickFIX
find_package(QuickFIX REQUIRED)

# PostgreSQL
find_package(PkgConfig REQUIRED)
pkg_check_modules(PGSQL REQUIRED libpq)

# Исходники
add_executable(quickfix_postgres
    src/main.cpp
    src/FixApplication.cpp
    src/Database.cpp
)

target_link_libraries(quickfix_postgres
    ${QuickFIX_LIBRARIES}
    ${PGSQL_LIBRARIES}
)

target_include_directories(quickfix_postgres PRIVATE
    ${QuickFIX_INCLUDE_DIRS}
    ${PGSQL_INCLUDE_DIRS}
    src/
)

# Копируем конфигурационные файлы в build
install(DIRECTORY config/ DESTINATION .)
install(DIRECTORY spec/ DESTINATION .)
```

---

## 📄 config/fix.cfg

```ini
[DEFAULT]
ConnectionType=initiator
ReconnectInterval=5
FileStorePath=store
FileLogPath=log
StartTime=00:00:00
EndTime=00:00:00
UseDataDictionary=Y
DataDictionary=spec/FIX44.xml

[SESSION]
BeginString=FIX.4.4
SenderCompID=CLIENT
TargetCompID=SERVER
SocketConnectHost=localhost
SocketConnectPort=9876
HeartBtInt=30
```

---

## 📄 spec/FIX44.xml

```xml
<?xml version="1.0" encoding="UTF-8"?>
<FIX major="4" minor="4">
  <header>
    <field name="BeginString" number="8" type="STRING" />
    <field name="BodyLength" number="9" type="INT" />
    <field name="MsgType" number="35" type="STRING" />
    <field name="SenderCompID" number="49" type="STRING" />
    <field name="TargetCompID" number="56" type="STRING" />
    <field name="OnBehalfOfCompID" number="115" type="STRING" />
    <field name="DeliverToCompID" number="128" type="STRING" />
    <field name="MsgSeqNum" number="34" type="INT" />
    <field name="SenderSubID" number="50" type="STRING" />
    <field name="TargetSubID" number="57" type="STRING" />
    <field name="OnBehalfOfSubID" number="116" type="STRING" />
    <field name="DeliverToSubID" number="129" type="STRING" />
    <field name="PossDupFlag" number="43" type="BOOLEAN" />
    <field name="PossResend" number="97" type="BOOLEAN" />
    <field name="SendingTime" number="52" type="UTCTIMESTAMP" />
    <field name="OrigSendingTime" number="122" type="UTCTIMESTAMP" />
    <field name="XmlDataLen" number="347" type="INT" />
    <field name="XmlData" number="348" type="DATA" />
    <field name="MessageEncoding" number="347" type="STRING" />
    <field name="LastMsgSeqNumProcessed" number="369" type="INT" />
    <field name="HopCompID" number="628" type="STRING" />
    <field name="HopSendingTime" number="629" type="UTCTIMESTAMP" />
    <field name="HopRefID" number="630" type="INT" />
  </header>
  <messages>
    <message name="Heartbeat" msgtype="0" msgcat="admin">
      <field name="TestReqID" required="N" />
    </message>
    <message name="TestRequest" msgtype="1" msgcat="admin">
      <field name="TestReqID" required="Y" />
    </message>
    <message name="Reject" msgtype="3" msgcat="admin">
      <field name="RefSeqNum" required="Y" />
      <field name="RefMsgType" required="N" />
      <field name="SessionRejectReason" required="N" />
      <field name="Text" required="N" />
      <field name="EncodedTextLen" required="N" />
      <field name="EncodedText" required="N" />
    </message>
    <message name="SequenceReset" msgtype="4" msgcat="admin">
      <field name="NewSeqNo" required="Y" />
    </message>
    <message name="Logout" msgtype="5" msgcat="admin">
      <field name="Text" required="N" />
      <field name="EncodedTextLen" required="N" />
      <field name="EncodedText" required="N" />
    </message>
    <message name="Logon" msgtype="A" msgcat="admin">
      <field name="EncryptMethod" required="Y" />
      <field name="HeartBtInt" required="Y" />
      <field name="RawDataLength" required="N" />
      <field name="RawData" required="N" />
      <field name="ResetSeqNumFlag" required="N" />
      <field name="NextExpectedMsgSeqNum" required="N" />
    </message>
    <message name="MarketDataRequest" msgtype="V" msgcat="app">
      <field name="MDReqID" required="Y" />
      <field name="SubscriptionRequestType" required="Y" />
      <field name="MarketDepth" required="N" />
      <field name="MDUpdateType" required="N" />
      <field name="AggregatedBook" required="N" />
      <field name="OpenCloseSettleFlag" required="N" />
      <field name="Scope" required="N" />
      <field name="MDImplicitDelete" required="N" />
      <component name="Instrument" required="Y" />
      <group name="NoMDEntryTypes" required="N">
        <field name="MDEntryType" required="Y" />
      </group>
      <group name="NoRelatedSym" required="N">
        <component name="Instrument" required="Y" />
      </group>
    </message>
    <message name="MarketDataSnapshotFullRefresh" msgtype="W" msgcat="app">
      <field name="MDReqID" required="N" />
      <component name="Instrument" required="Y" />
      <field name="LastSpotRate" required="N" />
      <field name="LastForwardPoints" required="N" />
      <field name="BidPx" required="N" />
      <field name="OfferPx" required="N" />
      <field name="MDEntrySize" required="N" />
    </message>
  </messages>
</FIX>
```

---

## 📄 src/FixApplication.h

```cpp
#ifndef FIXAPPLICATION_H
#define FIXAPPLICATION_H

#include <quickfix/Application.h>
#include <quickfix/MessageCracker.h>
#include <quickfix/SessionID.h>
#include <quickfix/FieldTypes.h>
#include <quickfix/Values.h>
#include "Database.h"

class FixApplication : public FIX::Application, public FIX::MessageCracker
{
public:
    FixApplication(Database& db);
    virtual ~FixApplication();

    void onCreate(const FIX::SessionID&) override;
    void onLogon(const FIX::SessionID&) override;
    void onLogout(const FIX::SessionID&) override;
    void toAdmin(FIX::Message&, const FIX::SessionID&) override;
    void toApp(FIX::Message&, const FIX::SessionID&) override;
    void fromAdmin(const FIX::Message&, const FIX::SessionID&) override;
    void fromApp(const FIX::Message&, const FIX::SessionID&) override;

private:
    Database& db_;
    bool logged_in_;
};

#endif // FIXAPPLICATION_H
```

---

## 📄 src/FixApplication.cpp

```cpp
#include "FixApplication.h"
#include <quickfix/fix44/MarketDataRequest.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>
#include <iostream>

FixApplication::FixApplication(Database& db) : db_(db), logged_in_(false) {}

FixApplication::~FixApplication() {}

void FixApplication::onCreate(const FIX::SessionID&) {}

void FixApplication::onLogon(const FIX::SessionID&) {
    std::cout << "Logged in.\n";
    logged_in_ = true;

    // Отправляем запрос на получение котировок
    FIX::MarketDataRequest mdRequest;
    mdRequest.set(FIX::MDReqID("EURUSD"));
    mdRequest.set(FIX::SubscriptionRequestType(FIX::SubscriptionRequestType_SNAPSHOT));
    mdRequest.set(FIX::MarketDepth(0));

    // Добавляем символ EURUSD
    FIX::Instrument instrument;
    instrument.set(FIX::Symbol("EURUSD"));
    mdRequest.addGroup(instrument);

    FIX::Session::sendToTarget(mdRequest, FIX::SessionID("FIX.4.4", "CLIENT", "SERVER"));
}

void FixApplication::onLogout(const FIX::SessionID&) {
    std::cout << "Logged out.\n";
    logged_in_ = false;
}

void FixApplication::toAdmin(F
```

[ Prompt: 4.0 t/s | Generation: 8.3 t/s ]

> почему ты остановилась на начале вывода функции toAdmin класса FixApplication?

|223.32.318.204 E srv    send_error: task id = 4018, error: request (4127 tokens) exceeds the available context size (4096 tokens), try increasing it          Error: request (4127 tokens) exceeds the available context size (4096 tokens), try increasing it


[ Prompt: 0.0 t/s | Generation: 0.0 t/s ]
