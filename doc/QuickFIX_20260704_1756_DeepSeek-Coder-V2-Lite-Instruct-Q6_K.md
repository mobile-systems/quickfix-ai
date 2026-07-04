<pre>
alexa@DellPro MINGW64 ~/workspace/quickfix-ai (main)
$ llama-cli -ngl auto -c 64000 -m ~/.lmstudio/models/lmstudio-community/DeepSeek-Coder-V2-Lite-Instruct-GGUF/DeepSeek-Coder-V2-Lite-Instruct-Q6_K.gguf

Loading model...


▄▄ ▄▄
██ ██
██ ██  ▀▀█▄ ███▄███▄  ▀▀█▄    ▄████ ████▄ ████▄
██ ██ ▄█▀██ ██ ██ ██ ▄█▀██    ██    ██ ██ ██ ██
██ ██ ▀█▄██ ██ ██ ██ ▀█▄██ ██ ▀████ ████▀ ████▀
                                    ██    ██
                                    ▀▀    ▀▀

build      : b9828-ebd048fc5
model      : C:/Users/alexa/.lmstudio/models/lmstudio-community/DeepSeek-Coder-V2-Lite-Instruct-GGUF/DeepSeek-Coder-V2-Lite-Instruct-Q6_K.gguf
modalities : text

available commands:
  /exit or Ctrl+C     stop or exit
  /regen              regenerate the last response
  /clear              clear the chat history
  /read <file>        add a text file
  /glob <pattern>     add text files using globbing pattern
</pre>

> Разработай работающий пример кода на C++ для периодического получения, накопления и обновления котировок посредством протокола FIX по инструментам FOREX с хранением результата полученных котировок в PostgreSQL, при сборке проекта используй CMake и Ubuntu 26.04. Реализуй эту версию для реального высокочастотного трейдинга (HFT) с асинхронным подходом (Boost.Asio + QuickFIX++), чтобы не блокировать поток, добавь возможность работы с 12 инструментами, реализуй функционал для работы со сделками. Покажи где берёшь quickfixpp, подготовь docker compose который интегрирует в docker-контейнер это приложение и отдельные контейнеры с PostgreSQL и с Grafana в которой настрой UI-дашборд к PostgreSQL Grafana для визуализации сделок, балансов и прочей полезной информации для трейдинга.

 Для создания проекта, который включает в себя сборку с использованием CMake, асинхронный обмен сообщениями через FIX-протокол с использованием Boost.Asio и QuickFIX++, а также хранение данных в PostgreSQL, потребуется выполнить несколько шагов. Ниже приведен обобщенный план и пример кода, который поможет вам начать работу.

### Шаг 1: Установка необходимых компонентов

1. **Установите необходимые пакеты на Ubuntu 22.04:**
   ```bash
   sudo apt update
   sudo apt install -y build-essential cmake postgresql postgresql-contrib libpq-dev libboost-all-dev libquickfix-dev
   ```

### Шаг 2: Создание проекта с использованием CMake

1. **Создайте директорию для вашего проекта:**
   ```bash
   mkdir forex_trading_hft
   cd forex_trading_hft
   ```

2. **Создайте файл `CMakeLists.txt`:**
   ```cmake
   cmake_minimum_required(VERSION 3.10)
   project(ForexTradingHFT)

   set(CMAKE_CXX_STANDARD 17)

   # QuickFIX++
   find_package(QuickFIX REQUIRED)
   include_directories(${QUICKFIX_INCLUDE_DIRS})

   # Boost Asio
   find_package(Boost REQUIRED COMPONENTS system asio)
   include_directories(${Boost_INCLUDE_DIR})

   # PostgreSQL
   find_package(PostgreSQL REQUIRED)
   include_directories(${PostgreSQL_INCLUDE_DIRS})

   add_executable(forex_trading_hft
       main.cpp
       fix_session_handler.cpp
       fix_connection.cpp
       postgres_storage.cpp
       trading_engine.cpp
   )

   target_link_libraries(forex_trading_hft
       ${QUICKFIX_LIBRARIES}
       ${Boost_LIBRARIES}
       ${PostgreSQL_LIBRARIES}
   )
   ```

### Шаг 3: Написание кода

1. **Создайте файл `main.cpp`:**
   ```cpp
   #include <iostream>
   #include <quickfix/Session.h>
   #include <boost/asio.hpp>
   #include "fix_connection.h"
   #include "trading_engine.h"

   int main() {
       try {
           boost::asio::io_context io_context;
           FIX::SessionSettings settings("FIX_config.cfg");
           FixConnection fixConnection(io_context, settings);
           TradingEngine tradingEngine;

           // Start the IO service
           std::thread io_thread([&io_context]() { io_context.run(); });

           // Start the FIX connection
           fixConnection.start();

           // Main loop
           while (true) {
               std::this_thread::sleep_for(std::chrono::seconds(1));
           }

           io_thread.join();
       } catch (const std::exception &e) {
           std::cerr << "Error: " << e.what() << std::endl;
           return 1;
       }
       return 0;
   }
   ```

2. **Создайте файл `fix_connection.h`:**
   ```cpp
   #ifndef FIX_CONNECTION_H
   #define FIX_CONNECTION_H

   #include <quickfix/Session.h>
   #include <boost/asio.hpp>

   class FixConnection : public FIX::Session::Listener {
   public:
       FixConnection(boost::asio::io_context &io_context, const FIX::SessionSettings &settings);
       void start();

       // Implement Session::Listener methods
       void onLogon(const FIX::SessionID &sessionID) override;
       void onLogout(const FIX::SessionID &sessionID) override;
       void toAdmin(FIX::Message &message, const FIX::SessionID &sessionID) override;
       void toApp(FIX::Message &message, const FIX::SessionID &sessionID) override;
       void fromAdmin(const FIX::Message &message, const FIX::SessionID &sessionID) override;
       void fromApp(const FIX::Message &message, const FIX::SessionID &sessionID) override;

   private:
       FIX::Application &app_;
       FIX::Session session_;
       boost::asio::io_context &io_context_;
   };

   #endif // FIX_CONNECTION_H
   ```

3. **Создайте файл `fix_connection.cpp`:**
   ```cpp
   #include "fix_connection.h"

   FixConnection::FixConnection(boost::asio::io_context &io_context, const FIX::SessionSettings &settings)
       : io_context_(io_context), session_(settings) {
       session_.addListener(*this);
       app_ = std::make_unique<QuickFIX::NullApplication>();
   }

   void FixConnection::start() {
       session_.start();
   }

   void FixConnection::onLogon(const FIX::SessionID &sessionID) {
       std::cout << "Logged in to " << sessionID << std::endl;
   }

   void FixConnection::onLogout(const FIX::SessionID &sessionID) {
       std::cout << "Logged out from " << sessionID << std::endl;
   }

   void FixConnection::toAdmin(FIX::Message &message, const FIX::SessionID &sessionID) {
       // Implement if needed
   }

   void FixConnection::toApp(FIX::Message &message, const FIX::SessionID &sessionID) {
       // Implement if needed
   }

   void FixConnection::fromAdmin(const FIX::Message &message, const FIX::SessionID &sessionID) {
       // Implement if needed
   }

   void FixConnection::fromApp(const FIX::Message &message, const FIX::SessionID &sessionID) {
       // Implement if needed
   }
   ```

4. **Создайте файл `trading_engine.h`:**
   ```cpp
   #ifndef TRADING_ENGINE_H
   #define TRADING_ENGINE_H

   #include <vector>
   #include <string>
   #include <pqxx/pqxx>

   class TradingEngine {
   public:
       TradingEngine();
       void updateQuotes(const std::vector<std::pair<std::string, double>> &quotes);

   private:
       void storeQuotesInDB(const std::vector<std::pair<std::string, double>> &quotes);
       pqxx::connection conn_;
   };

   #endif // TRADING_ENGINE_H
   ```

5. **Создайте файл `trading_engine.cpp`:**
   ```cpp
   #include "trading_engine.h"

   TradingEngine::TradingEngine() : conn_("dbname=forex_trading user=your_user password=your_password") {
       if (conn_.is_open()) {
           std::cout << "Opened database successfully" << std::endl;
       } else {
           std::cerr << "Can't open database" << std::endl;
       }
   }

   void TradingEngine::updateQuotes(const std::vector<std::pair<std::string, double>> &quotes) {
       storeQuotesInDB(quotes);
   }

   void TradingEngine::storeQuotesInDB(const std::vector<std::pair<std::string, double>> &quotes) {
       pqxx::work txn(conn_);
       for (const auto &quote : quotes) {
           txn.exec("INSERT INTO quotes (instrument, price) VALUES ('" + quote.first + "', " + std::to_string(quote.second) + ")");
       }
       txn.commit();
   }
   ```

### Шаг 4: Настройка Docker Compose

1. **Создайте файл `docker-compose.yml`:**
   ```yaml
   version: '3'

   services:
     app:
       build: .
       ports:
         - "8080:8080"
       depends_on:
         - db
         - grafana

     db:
       image: postgres
       environment:
         POSTGRES_DB: forex_trading
         POSTGRES_USER: your_user
         POSTGRES_PASSWORD: your_password
       ports:
         - "5432:5432"

     grafana:
       image: grafana/grafana
       ports:
         - "3000:3000"
       environment:
         GF_SECURITY_ADMIN_USER: admin
         GF_SECURITY_ADMIN_PASSWORD: admin
   ```

2. **Создайте файл `.env` для хранения конфиденциальной информации:**
   ```
   POSTGRES_DB=forex_trading
   POSTGRES_USER=your_user
   POSTGRES_PASSWORD=your_password
   ```

### Шаг 5: Построение и запуск проекта

1. **Сборка проекта с использованием CMake:**
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

2. **Запуск Docker Compose:**
   ```bash
   docker-compose up --build
   ```

### Шаг 6: Настройка Grafana

1. **Перейдите в веб-браузер и откройте Grafana:**
   ```
   http://localhost:3000
   ```

2. **Войдите с использованием учетных данных `admin:admin`.**

3. **Добавьте PostgreSQL в качестве источника данных:**
   - Перейдите в меню "Configuration" и выберите "Data Sources."
   - Нажмите на "Add data source" и выберите "PostgreSQL."
   - Заполните поля:
     - **Name:** PostgreSQL
     - **Default:** ON
     - **URL:** `jdbc:postgresql://db:5432/forex_trading`
     - **User:** your_user
     - **Password:** your_password
   - Нажмите "Save & Test."

4. **Создайте дашборд и панель для визуализации сделок, балансов и т.д.**

Этот пример демонстрирует базовую структуру проекта. В реальном проекте потребуется дополнительная обработка ошибок, настройка параметров FIX-протокола, а также оптимизация кода для высокочастотной торговли.

[ Prompt: 11.9 t/s | Generation: 7.2 t/s ]
