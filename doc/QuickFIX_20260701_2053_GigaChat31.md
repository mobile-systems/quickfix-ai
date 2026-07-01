<pre>
alexa@DellPro MINGW64 ~/workspace/quickfix-ai (main)
$ llama-cli --ctx-size 128000 -m ~/.lmstudio/models/ai-sage/GigaChat3.1-10B-A1.8B-GGUF/GigaChat3.1-10B-A1.8B-q8_0.gguf

Loading model...


▄▄ ▄▄
██ ██
██ ██  ▀▀█▄ ███▄███▄  ▀▀█▄    ▄████ ████▄ ████▄
██ ██ ▄█▀██ ██ ██ ██ ▄█▀██    ██    ██ ██ ██ ██
██ ██ ▀█▄██ ██ ██ ██ ▀█▄██ ██ ▀████ ████▀ ████▀
                                    ██    ██
                                    ▀▀    ▀▀

build      : b9828-ebd048fc5
model      : C:/Users/alexa/.lmstudio/models/ai-sage/GigaChat3.1-10B-A1.8B-GGUF/GigaChat3.1-10B-A1.8B-q8_0.gguf
modalities : text

available commands:
  /exit or Ctrl+C     stop or exit
  /regen              regenerate the last response
  /clear              clear the chat history
  /read <file>        add a text file
  /glob <pattern>     add text files using globbing pattern
</pre>

> Разработай работающий пример кода на C++ для периодического получения, накопления и обновления котировок посредством протокола FIX по инструментам FOREX с хранением результата полученных котировок в PostgreSQL, при сборке проекта используй CMake и Ubuntu 26.04. Реализуй эту версию для реального высокочастотного трейдинга (HFT) с асинхронным подходом (Boost.Asio + QuickFIX++), чтобы не блокировать поток, добавь возможность работы с 12 инструментами, реализуй функционал для работы со сделками. Покажи где берёшь quickfixpp, подготовь docker compose который интегрирует в docker-контейнер это приложение и отдельные контейнеры с PostgreSQL и с Grafana в которой настрой UI-дашборд к PostgreSQL Grafana для визуализации сделок, балансов и прочей полезной информации для трейдинга.

Вот подробное руководство по реализации высокочастотного трейдерского бэкенда на C++ с использованием Boost.Asio и QuickFIX++, базы данных PostgreSQL и Grafana.

### 1. Архитектура приложения
- **QuickFIX++** — библиотека для работы с протоколом FIX/FAST. Используется для получения и отправки заявок/котировок.
- **Boost.Asio** — асинхронный I/O для обработки большого потока данных без блокировки.
- **PostgreSQL** — хранилище для сделок, балансов и котировок.
- **Grafana** — визуализация дашбордов по данным из PostgreSQL.

---

### 2. Проект с использованием CMake
Создадим структуру проекта:

```
/hft_app
├── CMakeLists.txt
├── src
│   ├── main.cpp
│   ├── fix_session.cpp
│   ├── fix_handler.cpp
│   ├── trade_handler.cpp
│   ├── db_manager.cpp
│   ├── signal_gen.cpp
└── docker-compose.yml
```

#### **CMakeLists.txt**
```cmake
cmake_minimum_required(VERSION 3.15)
project(HftApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTs system thread)
find_package(QuickFIX REQUIRED)
find_package(PostgreSQL REQUIRED)

add_executable(hft_app src/main.cpp src/fix_session.cpp src/fix_handler.cpp src/trade_handler.cpp src/db_manager.cpp)

target_link_libraries(hft_app PRIVATE Boost::boost Boost::quickfix)
```

---

### 3. Реализация ключевых компонентов

#### **src/fix_session.cpp**
```cpp
#include <boost/asio/ip/tcp.hpp>
#include "quickfix.cpp"
#include "db_manager.h"
#include <iostream>

using boost::asio::ip::tcp;

class FixSession : public std::enable_shared_from_this<FixSession> {
public:
    FixSession(boost::asio::io_context& io_context,
               const std::string& ip, const int port,
               const std::string& appID)
        : socket_(io_context), resolver_(io_context),
          app_(this), sessionID_(appID) {
        doResolve(ip, port);
    }

    void start() {
        doConnect();
    }

    void stop() {
        socket_.close();
    }

private:
    tcp::socket socket_;
    tcp::resolver resolver_;
    FIX::Session app_;
    std::string sessionID_;

    void doResolve(const std::string& ip, const int port) {
        auto endpoints = resolver_.resolve(ip, std::to_string(port));
        boost::asio::async_connect(socket_, endpoints, [this](boost::system::error_code ec, tcp::endpoint) {
            if (!ec) doInitialize();
        });
    }

    void doInitialize() {
        FIX::SessionSettings settings;
        FIX::Session::init(app_, settings, nullptr, true);
        app_.start();
    }
};
```

#### **src/trade_handler.cpp**
```cpp
#include <boost/asio.hpp>
#include "quickfix.cpp"
#include "db_manager.h"

class TradeHandler : public FIX::Application, public FIX::Log {
public:
    TradeHandler(boost::asio::io_context& io) : io_(io) {
        FIX::SessionSettings settings;
        FIX::Session::init(this, settings, this, true);
    }

    void fromMessage(const FIX::Message& m, bool complete) override {
        if (m.getHeader().getField(FIX::MsgType()) == FIX::MsgType::EXECUTION_REPORT) {
            int orderID = 0;
            m.getField(orderID);
            std::string side = m.getField(FIX::Side()); // 1=Buy, 2=Sell
            std::string symbol = m.getField(FIX::Symbol());
            int quantity = 0;
            m.getField(quantity);

            // Обновление баланса (упрощенно)
            db_.updateBalance(symbol, side == "1" ? +quantity : -quantity);

            // Запись сделки в БД
            db_.logTrade(m);
        }
    }

    void onCreate(FIX::SessionID sessionID) override {}
    void onLogon(FIX::SessionID sessionID) override { std::cout << "Logon: " << sessionID.toString() << std::endl; }
    void onLogout(FIX::SessionID sessionID) override {}

private:
    boost::asio::io_context& io_;
    DBManager db_;
};
```

#### **src/db_manager.h / .cpp**
```cpp
#include <pqxx/pqxx>
#include <string>

class DBManager {
public:
    DBManager(const std::string& connStr) : db_(connStr) {}

    void updateBalance(const std::string& symbol, double delta) {
        pqxx::work txn(db_);
        txn.exec_params("UPDATE balances SET balance = balance + $1 WHERE symbol = $2", delta, symbol);
        txn.commit();
    }

    void logTrade(const FIX::Message& msg) {
        // Запись сделки в БД
        // (Здесь нужно парсить FIX-сообщение)
    }

private:
    pqxx::connection db_;
};
```

---

### 4. Docker Compose

Создадим `docker-compose.yml`:
```yaml
version: "3.8"
services:
  hft-app:
    build: ./hft_app
    ports:
      - "3000:8080"
      - "4000:4000"
      - "5432:5432"
    depends_on:
      - db
      - grafana

  db:
    image: postgres:15
    environment:
      POSTGRES_USER: hft_user
      POSTGRES_PASSWORD: securepass
      POSTGRES_DB: hft_db
    ports:
      - "5432:5432"

  grafana:
    image: grafana/grafana:latest
    ports:
      - "3000:3000"
    volumes:
      - grafana-data:/var/lib/grafana
    command: -config /etc/grafana/grafana.ini
    ports:
      - "3004:3004"
    environment:
      - GF_SERVER_ROOT_URL=http://localhost:3000
      - GF_DATABASE_PATH=/var/lib/grafana
    depends_on:
      - db

volumes:
  grafana-data:
```

**Примечание:** Для 12 инструментов нужно добавить таблицу в `db_manager.h`:
```sql
CREATE TABLE trades (
    id SERIAL PRIMARY KEY,
    symbol VARCHAR(10),
    side VARCHAR(1), // B/S
    quantity INT,
    price DECIMAL(10,5),
    trade_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
CREATE TABLE balances (
    symbol VARCHAR(10) PRIMARY KEY,
    balance DECIMAL(20,5)
);
```

---

### 5. Grafana Dashboard (SQL Query)
1. В Grafana добавляем PostgreSQL как источник данных.
2. Создаем дашборд:
   - **Сделки:** `SELECT * FROM trades ORDER BY trade_time DESC LIMIT 100`
   - **Балансы:** `SELECT * FROM balances`
   - **Сводка по инструментам:** Агрегация по `symbol` (сумма сделок)

---

### 6. Сборка и запуск
```sh
cd hft_app
mkdir build
cd build
cmake ..
make
# Запуск Docker Compose
docker-compose up -d
```

### ⚠️ Важные моменты для HFT:
1. **Быстродействие:** Для реальных HFT нужно использовать `io_uring` (Ubuntu 26.04 поддерживает) или `libaio`.
2. **Сериализация FIX:** QuickFIX++ работает с `char*`, что не оптимально для C++17. Можно переписать в `std::string` или использовать `fmt`.
3. **Стратегия сделок:** Реализуйте `TradeHandler` с учетом FIFO, отказов, повторных заявок.
4. **Масштабируемость:** В реальном проекте стоит запускать несколько `FixSession` на разных корутинах `io_context`.

Этот код — фундамент. Для боевой системы его нужно доработать: добавить логирование, обработку исключений, кэширование котировок и оптимизацию работы с БД.

[ Prompt: 96.6 t/s | Generation: 9.8 t/s ]
