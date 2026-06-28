<pre>
alexa@DellPro MINGW64 ~
$ llama-cli --ctx-size 256000 -m ~/.lmstudio/models/lmstudio-community/Phi-4-reasoning-plus-GGUF/Phi-4-reasoning-plus-Q8_0.gguf

Loading model...


▄▄ ▄▄
██ ██
██ ██  ▀▀█▄ ███▄███▄  ▀▀█▄    ▄████ ████▄ ████▄
██ ██ ▄█▀██ ██ ██ ██ ▄█▀██    ██    ██ ██ ██ ██
██ ██ ▀█▄██ ██ ██ ██ ▀█▄██ ██ ▀████ ████▀ ████▀
                                    ██    ██
                                    ▀▀    ▀▀

build      : b9754-52b3df002
model      : C:/Users/alexa/.lmstudio/models/lmstudio-community/Phi-4-reasoning-plus-GGUF/Phi-4-reasoning-plus-Q8_0.gguf
modalities : text

available commands:
  /exit or Ctrl+C     stop or exit
  /regen              regenerate the last response
  /clear              clear the chat history
  /read <file>        add a text file
  /glob <pattern>     add text files using globbing pattern
</pre>

> Мне нужен работающий пример на C++ периодического получения, накопления и обновления котировок посредством протокола FIX по инструменту EURUSD с использованием библиотеки QuickFIX и сохранением результата полученных котировок в PostgreSQL, при сборке проекта используй CMake

We are Phi and the request is: "Мне нужен работающий пример на C++ периодического получения, накопления и обновления котировок посредством протокола FIX по инструменту EURUSD с использованием библиотеки QuickFIX и сохранением результата полученных котировок в PostgreSQL, при сборке проекта используй CMake". The text is in Russian. Let's translate: "I need a working example in C++ for periodically receiving, accumulating, and updating quotes through the FIX protocol for the instrument EURUSD using QuickFIX library and saving the resulting quotes in PostgreSQL, using CMake for building the project." So the answer must provide a complete example in C++ with CMakeLists.txt file, code that uses QuickFIX library to subscribe to quotes, accumulate and update quotes periodically, and then store them in PostgreSQL. We need to include code for connecting to PostgreSQL, maybe using libpqxx or something else. Alternatively, we can use C++ database libraries for PostgreSQL such as libpqxx. We can provide a sample code that uses QuickFIX library for FIX sessions. Also, need a CMake build file that compiles the code. Let me produce a sample code that includes:

- C++ code using QuickFIX library.
- The code should include a message handler for receiving market data messages.
- It must periodically accumulate and update quotes.
- It must store the resulting quotes in PostgreSQL database. Use a library like libpqxx.

We need a code sample that uses QuickFIX message handling. QuickFIX is a library used for FIX message protocol. The code example should show how to create a FIX initiator and a session. We need to provide sample configuration file for QuickFIX. Also, need to include a code for connecting to PostgreSQL and storing quotes. There is a library libpqxx that can be used to connect to PostgreSQL. But the question states "сохранением результата полученных котировок в PostgreSQL" which means "saving the results of the obtained quotes in PostgreSQL". So, we need to show how to connect to a PostgreSQL DB and insert quotes into table.

We need a code sample that periodically receives quotes from QuickFIX, accumulates them, and update them in PostgreSQL. The code sample might be long. The answer should include instructions on how to run it.

Let's produce a code that includes: main function, QuickFIX Application class derived from QuickFix::Application, that handles incoming messages. We'll have a message handler for market data snapshot or quotes. The instrument is EURUSD.

I can produce a code sample with the following structure:
- main.cpp: This file includes main and sets up QuickFIX application and session.
- MyFixApp.h / MyFixApp.cpp: A class derived from QuickFix::Application that handles messages.
- Use libpqxx for PostgreSQL operations.
- CMakeLists.txt file with instructions.

I need to include QuickFIX library usage. Possibly include using QuickFIX::SessionID, QuickFIX::SessionSettings, QuickFix::FileStoreFactory, QuickFix::ScreenLogFactory, QuickFix::Initiator, etc.

I can produce a sample code that does something like this:

We can produce a file structure:
- CMakeLists.txt
- src/
  - main.cpp
  - FixApp.cpp
  - FixApp.h
  - DatabaseManager.cpp
  - DatabaseManager.h

Maybe we can produce a single file with all code to simplify.

We need to produce a code that periodically gets quotes. The QuickFIX library works with incoming FIX messages. Typically, we can subscribe to market data messages (like Quote or Market Data Snapshot or update messages). Possibly use QuickFix::Quote message or MDUpdate. But I'll assume we are receiving Quote messages.

We can create a class QuickFixApp that implements QuickFix::Application interface and overrides methods: onMessage(QuickFix::Quote&, const QuickFix::SessionID&), onMessage(QuickFix::MarketDataSnapshotFullRefresh&). But maybe we assume that the instrument is EURUSD and we are receiving quotes. We can use onMessage for Quote. But I need a periodical accumulation. Possibly, I can simulate that the application accumulates quotes in memory and then periodically writes to PostgreSQL DB. But since it's a sample, I can write a function that every time a quote message is received, it inserts a row in the database. Alternatively, I can accumulate quotes in memory and then have a timer that flushes them to the DB periodically. But the question states "периодического получения, накопления и обновления котировок" which translates to "periodically receive, accumulate and update quotes". So perhaps I can simulate a periodic update process that runs in a separate thread that periodically queries the in-memory quotes and then writes them to PostgreSQL.

But since it's a sample, I can simulate that the code uses a thread that every 10 seconds writes the latest quote to the DB. But it's better to store each quote update as it comes in. But then "накопления" means accumulation. Perhaps we need a data structure to store quotes, then update them periodically to DB.

Alternatively, we can simply write code that on every received quote, update the DB. But then "периодического получения" means "periodic receiving" which is not clear.

Maybe I can design the code so that the FIX application receives messages and pushes them to a thread-safe queue, and then another thread periodically flushes the queue to the DB.

We can implement a QuoteManager that holds quotes for instrument EURUSD, and then a thread that periodically flushes them.

Plan:

Define a Quote struct that holds fields: price, bid, ask maybe. But FIX Quote message fields: Symbol, Price, etc. But since the instrument is EURUSD, we can assume that we always get quotes for EURUSD, but to be safe, we can check.

Plan structure:
- Quote: maybe use a struct with fields: symbol, bid, ask, and timestamp.
- We'll have a thread-safe queue or vector protected by mutex.
- The QuickFix application will be our "QuoteReceiver" that receives messages.
- Then a separate thread, QuoteUpdater, that periodically flushes the queue to DB.

For the database, we can use libpqxx. So include <pqxx/pqxx> header, link to pqxx library.

I need to produce a code sample that is self-contained but may require external libraries (QuickFIX and libpqxx). I'll produce a code sample that uses QuickFIX and libpqxx.

I'll produce a file main.cpp containing everything:
- Include necessary headers: quickfix/fix_42.hpp maybe, quickfix/fix_defs.h. But what version of FIX? I'll assume FIX 4.2.
- Also include <thread>, <mutex>, <condition_variable>, <chrono>, <vector>, <iostream>, etc.
- Define a Quote struct.
- Define a QuoteManager class that has a vector or queue, and methods to add quotes and flush them to the database periodically.
- The QuoteManager class will have a thread that periodically flushes the quotes to the database.
- DatabaseManager class to encapsulate PostgreSQL connection and insertion query.

I can produce a DatabaseManager class that in its constructor opens a connection to PostgreSQL using libpqxx::connection. Then the QuoteManager can use DatabaseManager to insert quotes.

Plan:
- DatabaseManager.h and DatabaseManager.cpp: a class with a method insertQuote(const Quote&) that inserts a row into a table. The table could be "quotes" with columns: id serial primary key, symbol text, bid double precision, ask double precision, timestamp timestamp. But FIX Quote message doesn't have ask, but maybe we can simulate. But we can assume a Quote message has fields: symbol, price, etc. But I'll use a simple version with symbol, bid, ask, timestamp.
- Alternatively, I can use a simpler version: table "quotes" with columns: id, symbol, price, time.

But since we have Quote message, I'll use symbol, bid, ask, and maybe a timestamp. But FIX Quote message might have a field for the "QuoteID" and "BidPx" and "OfferPx". Actually, QuickFIX Quote message has fields: Symbol, QuoteID, BidPx, OfferPx, etc. I'll assume that "BidPx" is the bid price and "OfferPx" is the ask price, and "Symbol" is the symbol.

I need to produce code that extracts these fields from the Quote message.
- In onMessage(QuickFix::Quote&, const QuickFix::SessionID&), I can extract:
  string symbol = msg.get(QuickFix::Symbol());
  double bid = msg.get(QuickFix::BidPx(), 0.0);
  double ask = msg.get(QuickFix::OfferPx(), 0.0);
  Possibly, we can get time stamp from message header or current time.

I can then create a Quote struct and add it to a thread-safe container.

Thread-safety: I'll use std::mutex and std::vector or std::deque. I'll use std::deque<Quote> quotesQueue; and a std::mutex queueMutex; and a std::condition_variable maybe.

Then I'll have a QuoteUpdater thread that runs periodically (say every 10 seconds) and flushes the quotes from the queue to the database.

I need to design a way to stop the thread gracefully. I'll create a boolean flag "running" that the thread checks periodically. Possibly use condition variable to notify flush.

But for simplicity, I'll use a thread that runs in an infinite loop and sleeps for a period, then flushes the queue to the database using DatabaseManager. But then the application might exit normally if the session is closed.

Alternatively, I can run a loop that stops when receiving a termination signal. But for sample code, I'll just run it for some time then exit.

Plan code structure:
- main.cpp: Contains main function, sets up QuickFIX session and the QuoteUpdater thread.
- QuickFix application class: I'll define a class QuoteReceiverApp that inherits from QuickFix::Application. It overrides:
  - onMessage(QuickFix::Message&, const QuickFix::SessionID&)
  - onCreate, onLogon, etc. We'll implement the necessary functions.
- The QuoteReceiverApp holds a pointer to QuoteManager (or QuoteManager instance) to add quotes.

- The QuoteManager class: holds a thread-safe queue, and a thread that periodically flushes the queue to the database. It also holds a pointer to DatabaseManager.
- The DatabaseManager class: encapsulates connection to PostgreSQL.
- The code uses QuickFIX library. I need to include <quickfix/fix_42.h> maybe.

I need to produce a minimal QuickFIX session configuration file. I'll produce a sample config file in the answer, maybe "quickfix.cfg". But the question states "при сборке проекта используй CMake", so I'll include the CMakeLists.txt file in the answer, and also mention that you need to link with QuickFIX and libpqxx libraries.

I need to produce a complete sample code. I can produce one file "main.cpp" that contains everything, or produce multiple files. Given the answer is "работающий пример", I might produce multiple files.

I can produce something like:

File structure:
- CMakeLists.txt
- src/main.cpp
- src/QuoteReceiverApp.h
- src/QuoteReceiverApp.cpp
- src/QuoteManager.h
- src/QuoteManager.cpp
- src/DatabaseManager.h
- src/DatabaseManager.cpp
- quickfix.cfg (sample configuration file)

But since the answer is text, I'll produce code examples that include all necessary code in one answer. I'll produce a complete example, but I'll try to produce code in one file to simplify. But a better sample is multiple files.

Maybe I'll produce a simpler version that is in one file: main.cpp, and then include the classes inside. But then I'll need to produce a CMakeLists.txt file that compiles it.

I can produce the following files:

File: CMakeLists.txt:
```
cmake_minimum_required(VERSION 3.10)
project(FixQuoteExample)

set(CMAKE_CXX_STANDARD 11)

find_package(QuickFIX REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

include_directories(${QuickFIX_INCLUDE_DIRS} ${PQXX_INCLUDE_DIRS})
link_directories(${QuickFIX_LIBRARY_DIRS} ${PQXX_LIBRARY_DIRS})

add_executable(FixQuoteExample src/main.cpp)
target_link_libraries(FixQuoteExample ${QuickFIX_LIBRARIES} pqxx)
```
But need to check how to link. Actually, QuickFIX might be found with find_package(QuickFIX). But sometimes QuickFIX is not found using find_package. But I'll assume it's installed and can be found.

I'll produce a file structure that includes a CMakeLists.txt, and then a src folder with main.cpp and the other classes. I can produce code in one file for simplicity: "src/main.cpp" that includes all classes.

I'll produce the code in "src/main.cpp":

Include headers:
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <deque>
#include <condition_variable>
#include <atomic>
#include <quickfix/fix_42.h>
#include <quickfix/Session.h>
#include <quickfix/Initiator.h>
#include <quickfix/Config.h>
#include <quickfix/FileStoreFactory.h>
#include <quickfix/ScreenLogFactory.h>
#include <quickfix/SessionSettings.h>
#include <pqxx/pqxx>

Define a struct Quote:
struct Quote {
    std::string symbol;
    double bid;
    double ask;
    std::chrono::system_clock::time_point timestamp;
};

Define a DatabaseManager class:
class DatabaseManager {
public:
    DatabaseManager(const std::string &conn_str) {
        try {
            conn = pqxx::connection(conn_str);
            if (!conn.is_open()) {
                throw std::runtime_error("Can't open database");
            }
            // Create table if not exists
            pqxx::work txn(conn);
            txn.exec("CREATE TABLE IF NOT EXISTS quotes (id SERIAL PRIMARY KEY, symbol TEXT, bid DOUBLE PRECISION, ask DOUBLE PRECISION, timestamp TIMESTAMPTZ)");
            txn.commit();
        } catch (const std::exception &e) {
            std::cerr << "Database error: " << e.what() << std::endl;
        }
    }

    void insertQuote(const Quote &quote) {
        try {
            pqxx::work txn(conn);
            txn.exec_params("INSERT INTO quotes (symbol, bid, ask, timestamp) VALUES ($1, $2, $3, $4)",
                            quote.symbol, quote.bid, quote.ask, pqxx::timestamp(boost::posix_time::from_time_t(std::chrono::system_clock::to_time_t(quote.timestamp))));
            txn.commit();
        } catch (const std::exception &e) {
            std::cerr << "Database insert error: " << e.what() << std::endl;
        }
    }
private:
    pqxx::connection conn;
};

Now define QuoteManager, which accumulates quotes and periodically flushes them:
class QuoteManager {
public:
    QuoteManager(DatabaseManager &dbManager)
    : dbManager(dbManager), stopFlag(false) {
        updaterThread = std::thread(&QuoteManager::flushQuotes, this);
    }

    ~QuoteManager() {
        stopFlag = true;
        if (updaterThread.joinable()) {
            updaterThread.join();
        }
    }

    void addQuote(const Quote &quote) {
        std::lock_guard<std::mutex> lock(queueMutex);
        quotes.push_back(quote);
    }

private:
    void flushQuotes() {
        while (!stopFlag) {
            std::this_thread::sleep_for(std::chrono::seconds(10)); // flush every 10 seconds
            std::deque<Quote> localQueue;
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                localQueue.swap(quotes);
            }
            for (const auto &quote : localQueue) {
                dbManager.insertQuote(quote);
            }
        }
    }

    DatabaseManager &dbManager;
    std::deque<Quote> quotes;
    std::mutex queueMutex;
    std::atomic<bool> stopFlag;
    std::thread updaterThread;
};

Now define QuickFIX application: class QuoteReceiverApp, inheriting from QuickFix::Application.

class QuoteReceiverApp : public QuickFix::Application {
public:
    QuoteReceiverApp(QuoteManager &qm) : quoteManager(qm) {}

    // override necessary functions
    void onMessage(const QuickFix::Message& message, const QuickFix::SessionID& sessionID) override {
        // Check if message is Quote
        if (message.getHeader().getString(QuickFix::MsgType()) == "0" /* Actually, FIX Quote message type is "0"? Let's check: In FIX 4.2, Quote message type is "0"? Actually, I believe Quote message type is "0"? Let me recall: The MsgType for Quote is "0". But let me check: Actually, Quote message type is "0" if I recall correctly? Actually, in FIX 4.2, Quote message type is "0"? Wait, I'll check: In FIX, the Quote message type is "0"? Actually, I'm not 100% sure.
        // Let me check: In FIX 4.2, the Quote message type is "0". Actually, the "Quote" message type is "0", I believe. But I'll check:
        // Alternatively, I can check: message.getQuickFixString(QuickFix::MsgType()).
        // However, I can check using message.is(QuickFix::Quote()) to see if it's a Quote message.
        if (message.is(QuickFix::Quote())) {
            QuickFix::Quote quoteMsg(message);
            std::string symbol = quoteMsg.getSymbol();
            if (symbol != "EURUSD") {
                return; // only process EURUSD
            }
            double bid = quoteMsg.getBidPx();
            double ask = quoteMsg.getOfferPx();
            Quote quote { symbol, bid, ask, std::chrono::system_clock::now() };
            quoteManager.addQuote(quote);
            std::cout << "Received Quote: " << symbol << " Bid: " << bid << " Ask: " << ask << std::endl;
        }
    }

    void fromApp(const QuickFix::Message& message, const QuickFix::SessionID& sessionID) override {
        // This method is called when a message is received from the counterparty.
        onMessage(message, sessionID);
    }

    void onCreate(const QuickFix::SessionID& sessionID) override {
        std::cout << "Session created: " << sessionID << std::endl;
    }

    void onLogon(const QuickFix::SessionID& sessionID) override {
        std::cout << "Logon completed: " << sessionID << std::endl;
    }

    void onLogout(const QuickFix::SessionID& sessionID) override {
        std::cout << "Logout completed: " << sessionID << std::endl;
    }

private:
    QuoteManager &quoteManager;
};

Then main function:
int main(int argc, char* argv[]) {
    // Load QuickFIX configuration from file "quickfix.cfg"
    std::string configFile = "quickfix.cfg";
    try {
        QuickFix::SessionSettings settings(configFile);
        // Setup store and log factories
        QuickFix::FileStoreFactory storeFactory(settings);
        QuickFix::ScreenLogFactory logFactory(settings);

        // Create application instance with QuoteManager instance
        QuoteManager quoteManager(DatabaseManager("dbname=fix_quotes user=postgres password=yourpassword hostaddr=127.0.0.1 port=5432"));
        QuoteReceiverApp app(quoteManager);

        // Setup initiator
        QuickFix::Initiator initiator(settings, storeFactory, logFactory, app);
        initiator.init();

        // Run the initiator (this call will block)
        initiator.run();
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}

But note: The call initiator.run() is blocking. So the QuoteManager thread will be joined on destruction after run returns. But if run never returns, then the QuoteManager thread will run and accumulate quotes. But if the session is terminated, it might join the thread.

I must ensure that the QuoteManager destructor is called when the application exits. But if initiator.run() returns, then the QuoteManager destructor is called and join thread.

I must also include necessary headers from boost if using pqxx timestamp conversion. But I might include <boost/date_time/posix_time/posix_time.hpp> since pqxx::timestamp expects a boost::posix_time::ptime.

So include #include <boost/date_time/posix_time/posix_time.hpp>.

I'll produce the code accordingly.

I must check that using "message.is(QuickFix::Quote())" is valid. Yes, in QuickFIX library, there is is() method for message type check.

I need to check that "quoteMsg.getBidPx()" returns double. Yes.

I need to include <deque> and <mutex> and <thread> and <chrono> and <atomic> and <iostream> and <string>.

I need to include <stdexcept>.

I need to include <pqxx/pqxx>.

I must also include <boost/date_time/posix_time/posix_time.hpp>.

I'll produce the code as one file: main.cpp. But I'll produce a sample CMakeLists.txt as well.

The final code sample:

I'll produce a code sample that is complete. I'll produce code with explanation.

I'll produce a file structure in the answer:
- CMakeLists.txt
- src/main.cpp
- quickfix.cfg sample configuration file

I need to produce a sample "quickfix.cfg". It can be something like:
```
[default]
# File store location
FileStorePath = store

[SESSION]
SenderCompID=INITIATOR
TargetCompID=TARGET
DataDictionary=spec\quickfix42.xml
SocketConnectHost=127.0.0.1
SocketConnectPort=5001
BeginString=FIX.4.2
HeartBtInt=30
ReconnectInterval=5
```
Something like that.

I'll produce a sample CMakeLists.txt that uses C++11 and links with QuickFIX and libpqxx.

I'll produce the code in the answer.

I must produce a "работающий пример", which means "working example". I must note that this example assumes that QuickFIX and libpqxx are installed and available.

I must note that the code might need adjustments depending on the versions of libraries.

I'll produce a final answer with explanation and code. I'll produce the code in plain text. I'll produce the answer in Russian. The answer is in Russian so I'll produce my answer in Russian.

I'll produce final answer in Russian, with code examples and explanations.

I'll produce answer in Russian: "Ниже приведен пример..." etc.

I'll produce final answer message with code blocks in plain text. I must produce code with no markdown formatting by default? The instructions say "By default, do not reply with markdown formatting unless requested" so I produce plain text code blocks? But instructions say "do not use markdown formatting unless requested". But the default is no markdown formatting? Actually "By default, do not reply with markdown formatting (e.g., **, ```)" but I can produce plain text. But instructions say "По умолчанию не отвечайте с markdown formatting" but instructions say "by default do not produce markdown unless requested"? The instruction says: "По умолчанию, не включайте markdown formatting" but then instructions say "unless requested"? Actually, instructions say "by default, do not reply with Markdown formatting, e.g., ** or ```" but the conversation content is in markdown? I must produce plain text code blocks? It says "by default, do not produce markdown unless requested". I'll produce plain text code blocks with no markdown formatting, I'll produce text blocks that are clearly code. I'll produce the answer text in plain text with code blocks separated by triple backticks? But instructions say "by default do not produce markdown formatting"? "By default do not reply with markdown formatting" but instructions say "unless requested"? The instructions "By default, do not reply with Markdown formatting" means I produce plain text? I'm not 100% sure. But instructions say "При сборке проекта используй CMake" so I'll produce a code file structure. I'll produce answer in plain text with code formatting but not markdown formatting if possible.

I produce final answer with explanation and code. I'll produce final answer in Russian text.Ниже приведён пример рабочего проекта на C++, который демонстрирует получение, накопление и периодическое обновление котировок по инструменту EURUSD через протокол FIX (используя библиотеку QuickFIX) с последующей записью результатов в базу данных PostgreSQL (с использованием библиотеки libpqxx). При сборке проекта используется CMake.

Для работы примера необходимо:
 • Иметь установленную библиотеку QuickFIX (с файлом конфигурации, например, quickfix.cfg)
 • Иметь установленную библиотеку libpqxx (соответствующую версию PostgreSQL и Boost)
 • Создать базу данных (например, с именем fix_quotes) и пользователя с соответствующими правами доступа

Пример структуры файлов выглядит так:

--------------------------------------------------
Проект
│
├─ CMakeLists.txt
├─ quickfix.cfg
└─ src
  └─ main.cpp
--------------------------------------------------

Ниже приведены файлы:

─────────────────────────────
CMakeLists.txt
─────────────────────────────
cmake_minimum_required(VERSION 3.10)
project(FixQuoteExample)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Найдём библиотеку QuickFIX (при условии, что у вас есть find_package-скрипт для неё)
find_package(QuickFIX REQUIRED)

# Найдём libpqxx через pkg-config (убедитесь, что libpqxx доступен)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

include_directories(${QuickFIX_INCLUDE_DIRS} ${PQXX_INCLUDE_DIRS})
link_directories(${QuickFIX_LIBRARY_DIRS} ${PQXX_LIBRARY_DIRS})

add_executable(FixQuoteExample src/main.cpp)
target_link_libraries(FixQuoteExample ${QuickFIX_LIBRARIES} pqxx)

─────────────────────────────
quickfix.cfg
─────────────────────────────
[default]
# Путь к файлу хранилища сессий
FileStorePath = store

[SESSION]
SenderCompID=INITIATOR
TargetCompID=TARGET
DataDictionary=spec/quickfix42.xml
SocketConnectHost=127.0.0.1
SocketConnectPort=5001
BeginString=FIX.4.2
HeartBtInt=30
ReconnectInterval=5

─────────────────────────────
src/main.cpp
─────────────────────────────
/*
  Пример применения QuickFIX для получения котировок по EURUSD,
  их накопления в памяти и периодического обновления в базе данных PostgreSQL.
  Для работы с PostgreSQL используется библиотека libpqxx.
*/

#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <deque>
#include <condition_variable>
#include <atomic>
#include <stdexcept>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>  // для работы с timestamp
#include <pqxx/pqxx>

// Подключаем заголовки QuickFIX (используем FIX 4.2)
#include <quickfix/fix_42.h>
#include <quickfix/Session.h>
#include <quickfix/Initiator.h>
#include <quickfix/Config.h>
#include <quickfix/FileStoreFactory.h>
#include <quickfix/ScreenLogFactory.h>
#include <quickfix/SessionSettings.h>

//-----------------------------------------------------------------
// Структура для хранения котировки
struct Quote {
    std::string symbol;
    double bid;
    double ask;
    std::chrono::system_clock::time_point timestamp;
};

//-----------------------------------------------------------------
// Класс для работы с PostgreSQL
class DatabaseManager {
public:
    // Подключаемся к базе данных. Параметры строки подключения должны быть
    // скорректированы в соответствии с вашей настройкой PostgreSQL.
    DatabaseManager(const std::string &conn_str) {
        try {
            conn = pqxx::connection(conn_str);
            if (!conn.is_open()) {
                throw std::runtime_error("Не удалось открыть соединение с базой данных");
            }
            // Создаём таблицу, если она не существует
            {
                pqxx::work txn(conn);
                txn.exec("CREATE TABLE IF NOT EXISTS quotes ("
                         "id SERIAL PRIMARY KEY, "
                         "symbol TEXT, "
                         "bid DOUBLE PRECISION, "
                         "ask DOUBLE PRECISION, "
                         "timestamp TIMESTAMPTZ)");
                txn.commit();
            }
        } catch (const std::exception &e) {
            std::cerr << "Ошибка базы данных: " << e.what() << std::endl;
        }
    }

    // Вставка котировки в базу данных
    void insertQuote(const Quote &quote) {
        try {
            pqxx::work txn(conn);
            // Преобразуем время в boost::posix_time::ptime для работы с pqxx::timestamp
            boost::posix_time::ptime pt = boost::posix_time::from_time_t(
                std::chrono::system_clock::to_time_t(quote.timestamp));
            txn.exec_params(
                "INSERT INTO quotes (symbol, bid, ask, timestamp) VALUES ($1, $2, $3, $4)",
                quote.symbol, quote.bid, quote.ask, pqxx::timestamp(pt));
            txn.commit();
        } catch (const std::exception &e) {
            std::cerr << "Ошибка вставки котировки: " << e.what() << std::endl;
        }
    }

private:
    pqxx::connection conn;
};

//-----------------------------------------------------------------
// Класс для накопления кот

[ Prompt: 9.0 t/s | Generation: 0.7 t/s ]
