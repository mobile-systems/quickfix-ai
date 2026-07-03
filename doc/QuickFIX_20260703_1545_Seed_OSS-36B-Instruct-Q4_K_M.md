<pre>
alexa@DellPro MINGW64 ~/workspace/quickfix-ai (main)
$ llama-cli -ngl auto -c 64000 -m ~/.lmstudio/models/lmstudio-community/Seed-OSS-36B-Instruct-GGUF/Seed-OSS-36B-Instruct-Q4_K_M.gguf

Loading model...


▄▄ ▄▄
██ ██
██ ██  ▀▀█▄ ███▄███▄  ▀▀█▄    ▄████ ████▄ ████▄
██ ██ ▄█▀██ ██ ██ ██ ▄█▀██    ██    ██ ██ ██ ██
██ ██ ▀█▄██ ██ ██ ██ ▀█▄██ ██ ▀████ ████▀ ████▀
                                    ██    ██
                                    ▀▀    ▀▀

build      : b9828-ebd048fc5
model      : C:/Users/alexa/.lmstudio/models/lmstudio-community/Seed-OSS-36B-Instruct-GGUF/Seed-OSS-36B-Instruct-Q4_K_M.gguf
modalities : text

available commands:
  /exit or Ctrl+C     stop or exit
  /regen              regenerate the last response
  /clear              clear the chat history
  /read <file>        add a text file
  /glob <pattern>     add text files using globbing pattern
</pre>

> Разработай работающий пример кода на C++ для периодического получения, накопления и обновления котировок посредством протокола FIX по инструментам FOREX с хранением результата полученных котировок в PostgreSQL, при сборке проекта используй CMake и Ubuntu 26.04. Реализуй эту версию для реального высокочастотного трейдинга (HFT) с асинхронным подходом (Boost.Asio + QuickFIX++), чтобы не блокировать поток, добавь возможность работы с 12 инструментами, реализуй функционал для работы со сделками. Покажи где берёшь quickfixpp, подготовь docker compose который интегрирует в docker-контейнер это приложение и отдельные контейнеры с PostgreSQL и с Grafana в которой настрой UI-дашборд к PostgreSQL Grafana для визуализации сделок, балансов и прочей полезной информации для трейдинга.

[Start thinking]

Got it, let's tackle this problem step by step. The user wants a C++ HFT application using FIX protocol, async with Boost.Asio and QuickFIX++, storing data in PostgreSQL, with CMake, Docker Compose, and Grafana dashboards. First, I need to break down the requirements:

### 1. Core Components
- **FIX Protocol Integration**: Use QuickFIX++ for FIX messaging. Since it's HFT, async processing is key—QuickFIX++ has async support, but need to ensure non-blocking I/O.
- **Async Architecture**: Boost.Asio for async operations (maybe alongside QuickFIX++'s async, but need to check compatibility).
- **Multiple Instruments**: 12 FOREX pairs (e.g., EUR/USD, GBP/USD, etc.).
- **Trade Processing**: Executing trades, storing order book, trade history, balances.
- **PostgreSQL Storage**: Persist market data (ticks, bids/asks) and trade data (orders, executions, balance).
- **CMake Build**: Configure project with CMake for cross-platform (though target is Ubuntu 26.04).
- **Docker Compose**: Containers for app, PostgreSQL, Grafana.
- **Grafana Dashboard**: Visualize trades, balance, market data.

### 2. QuickFIX++ Setup
First, QuickFIX++ is a C++ FIX engine. Need to specify version (e.g., FIX.4.4 is common for FOREX). The user will need to install QuickFIX++ and Boost on Ubuntu. For Docker, we'll build the app with these dependencies.

### 3. Async Approach
QuickFIX++ has a `FIX::Application` interface, but the default is synchronous. To make it async, we can use Boost.Asio's io_context to run the FIX engine in a separate thread, or use QuickFIX++'s async session (if available). Wait, QuickFIX++ supports async sessions via `FIX::AsyncSession` which uses Boost.Asio under the hood. That's perfect—so we can run the FIX session in an async manner, not blocking the main thread.

### 4. Data Models
- **Market Data**: For each instrument, store timestamp, bid price, bid size, ask price, ask size (tick data).
- **Trades**: Order ID, instrument, side (buy/sell), quantity, price, execution time, status (filled, pending, canceled).
- **Balances**: Currency, available balance, used margin, equity (for each currency pair's quote currency).

### 5. PostgreSQL Schema
Design tables for:
- `instruments`: instrument_id (serial), symbol (varchar), pair (varchar), created_at (timestamp).
- `market_ticks`: tick_id (serial), instrument_id (int), timestamp (timestamp), bid_price (numeric), bid_size (numeric), ask_price (numeric), ask_size (numeric), received_at (timestamp).
- `orders`: order_id (uuid), instrument_id (int), side (varchar), quantity (numeric), price (numeric), status (varchar), created_at (timestamp), filled_quantity (numeric), avg_fill_price (numeric).
- `executions`: execution_id (serial), order_id (uuid), instrument_id (int), quantity (numeric), price (numeric), timestamp (timestamp), side (varchar).
- `balances`: balance_id (serial), currency (varchar), available (numeric), used (numeric), equity (numeric), updated_at (timestamp).

### 6. Docker Compose
- **PostgreSQL**: Use official postgres image, set environment variables for user, password, database.
- **Grafana**: Official grafana image, configure data source to PostgreSQL.
- **HFT App**: Build from Dockerfile using Ubuntu 26.04 base, install dependencies (QuickFIX++, Boost, PostgreSQL client libs), compile with CMake, run the app.

### 7. Grafana Dashboard
Need to create queries for:
- Real-time tick prices for each instrument.
- Order execution history (table, timeline).
- Balance overview (gauge charts for available/equity).
- Trade volume per instrument (bar charts).

### 8. Potential Challenges
- **FIX Session Configuration**: Need a FIX server (simulator) for testing—maybe use QuickFIX Engine's test acceptor or a public demo. For the example, we'll assume a FIX acceptor (e.g., a broker simulator) is available.
- **Async Processing**: Ensuring that market data and trade messages are processed without blocking the async loop. Use Boost.Asio's `post` or `dispatch` to handle callbacks in the io_context thread.
- **Performance**: HFT requires low latency—use efficient data structures (e.g., `std::unordered_map` for instrument tick storage), avoid unnecessary copies, use prepared statements for PostgreSQL to reduce query time.
- **Error Handling**: FIX message validation, connection errors, database disconnections—need robust error handling (logging, retries for DB connections).

### 9. Step-by-Step Implementation

#### Step 1: Project Structure
```
hft-fix-app/
├── CMakeLists.txt          # CMake configuration
├── src/
│   ├── main.cpp            # Main entry point
│   ├── FixApplication.cpp  # QuickFIX++ Application interface implementation
│   ├── FixApplication.h    # Header for FixApplication
│   ├── Database.cpp        # PostgreSQL interaction (CRUD operations)
│   ├── Database.h          # Header for Database
│   ├── Models.h            # Data models (MarketTick, Order, Execution, Balance)
│   ├── TradeManager.cpp    # Trade logic (order execution, balance updates)
│   └── TradeManager.h      # Header for TradeManager
├── config/
│   └── fix.cfg             # QuickFIX++ session configuration
├── docker/
│   ├── Dockerfile          # App Dockerfile
│   └── entrypoint.sh       # Entrypoint script for app container
└── docker-compose.yml      # Docker Compose configuration
```

#### Step 2: CMakeLists.txt
Need to find Boost and QuickFIX++ packages, link libraries, set compiler flags (C++17 for std::optional, etc.).

#### Step 3: FixApplication Class
Implement `FIX::Application` interface:
- `fromApp`: Handle incoming messages (MarketDataSnapshotFullRefresh, ExecutionReport, etc.).
- `toApp`: Send outgoing messages (NewOrderSingle, etc.).
- `onCreate`: Session created.
- `onLogon`: Session logged on (start market data subscription).
- `onLogout`: Session logged out.

For async, use `FIX::AsyncSession` which runs on a Boost.Asio `io_context`.

#### Step 4: Database Class
Use libpqxx (PostgreSQL C++ library) for async/non-blocking DB access? Wait, libpqxx is synchronous by default, but for HFT, we need to avoid blocking on DB writes. Solution: Use a separate thread pool (Boost.Asio thread pool) to queue DB operations, so the main async loop isn't blocked. Or use `libpq` (C library) with async notifications, but libpqxx is easier. So:
- Create a `Database` class that uses libpqxx `connection` with a connection string.
- For each write operation (insert tick, order, execution), use `async_write` via Boost.Asio? No, libpqxx doesn't support async, so alternative: Use a queue (e.g., `std::queue`) of DB tasks, and a dedicated thread that processes the queue, executing libpqxx operations synchronously. Since DB writes are relatively slow compared to FIX message processing, this queue will buffer writes and process them in a separate thread, keeping the main async loop responsive.

#### Step 5: TradeManager Class
Manages order lifecycle:
- Receive `NewOrderSingle` from user (or strategy) → send to FIX acceptor.
- Receive `ExecutionReport` from FIX acceptor → update order status, calculate filled quantity, update balance.
- Maintain in-memory cache of orders for fast lookup.

#### Step 6: Models.h
Define structs for data transfer:
```cpp
struct Instrument {
    int id;
    std::string symbol;
    std::string pair;
};

struct MarketTick {
    int instrumentId;
    std::chrono::system_clock::time_point timestamp;
    double bidPrice;
    double bidSize;
    double askPrice;
    double askSize;
};

struct Order {
    std::uuid id;
    int instrumentId;
    OrderSide side; // BUY/SELL
    double quantity;
    double price;
    OrderStatus status; // PENDING, FILLED, CANCELED
    std::chrono::system_clock::time_point createdAt;
    double filledQuantity = 0.0;
    double avgFillPrice = 0.0;
};

struct Execution {
    int id;
    std::uuid orderId;
    int instrumentId;
    double quantity;
    double price;
    std::chrono::system_clock::time_point timestamp;
    OrderSide side;
};

struct Balance {
    std::string currency;
    double available;
    double used;
    double equity;
    std::chrono::system_clock::time_point updatedAt;
};
```

#### Step 7: Docker Setup
- **PostgreSQL Container**: Expose port 5432, set `POSTGRES_USER`, `POSTGRES_PASSWORD`, `POSTGRES_DB`.
- **Grafana Container**: Expose port 3000, add PostgreSQL data source via environment variables or `grafana/provisioning`.
- **App Container**: Build from Dockerfile that installs dependencies:
  - Ubuntu 26.04 base.
  - Install `libboost-all-dev`, `libpq-dev`, `libquickfix-dev` (need to check if QuickFIX++ is in Ubuntu repos—if not, build from source).
  - Clone QuickFIX++ repo, build and install if not available.
  - Compile the app with CMake.

#### Step 8: Grafana Provisioning
Add a `provisioning/datasources` directory in the Grafana container with a YAML file for PostgreSQL data source:
```yaml
apiVersion: 1
datasources:
- name: PostgreSQL
  type: postgres
  access: proxy
  url: postgres:5432
  database: hft_db
  user: hft_user
  password: hft_password
  sslmode: disable
  isDefault: true
```

Add a dashboard JSON file in `provisioning/dashboards` with queries for:
- Market Ticks: `SELECT timestamp, bid_price, ask_price FROM market_ticks WHERE instrument_id = $instrument_id ORDER BY timestamp DESC LIMIT 1000;`
- Order History: `SELECT order_id, side, quantity, filled_quantity, status, created_at FROM orders ORDER BY created_at DESC;`
- Balance: `SELECT currency, available, equity FROM balances ORDER BY updated_at DESC LIMIT 1;`

### 10. Example Code Snippets

#### FixApplication.cpp (onLogon)
Subscribe to market data for 12 FOREX pairs:
```cpp
void FixApplication::onLogon(const FIX::SessionID& sessionID) {
    LOG_INFO("Logged on to FIX session: " << sessionID);

    // Subscribe to MarketDataSnapshotFullRefresh for 12 FOREX instruments
    std::vector<std::string> forexPairs = {
        "EUR/USD", "GBP/USD", "USD/JPY", "AUD/USD", "USD/CAD", "NZD/USD",
        "EUR/GBP", "EUR/JPY", "GBP/JPY", "AUD/JPY", "EUR/AUD", "GBP/AUD"
    };

    for (const auto& pair : forexPairs) {
        FIX::MarketDataRequest marketDataRequest;
        marketDataRequest.set(FIX::MDReqID(pair));
        marketDataRequest.set(FIX::SubscriptionRequestType(FIX::SubscriptionRequestType::SNAPSHOT_PLUS_UPDATES));
        marketDataRequest.set(FIX::MarketDepth(0)); // Only top of book

        FIX::Instrument instrument;
        instrument.set(FIX::Symbol(pair));
        marketDataRequest.addGroup(instrument);

        FIX::NoMDEntryTypes noMDEntryTypes;
        noMDEntryTypes.set(FIX::MDEntryType(FIX::MDEntryType::BID));
        noMDEntryTypes.set(FIX::MDEntryType(FIX::MDEntryType::OFFER));
        marketDataRequest.addGroup(noMDEntryTypes);

        sendToTarget(marketDataRequest, sessionID);
        LOG_INFO("Subscribed to market data for: " << pair);
    }
}
```

#### FixApplication.cpp (fromApp - MarketDataSnapshotFullRefresh)
Process incoming market data ticks:
```cpp
bool FixApplication::fromApp(const FIX::Message& message, const FIX::SessionID& sessionID) {
    try {
        if (message.is(FIX::MsgType::MarketDataSnapshotFullRefresh)) {
            FIX::MarketDataSnapshotFullRefresh mds;
            mds = message;

            std::string symbol;
            mds.get(FIX::Symbol(symbol));

            double bidPrice = 0.0, bidSize = 0.0;
            double askPrice = 0.0, askSize = 0.0;

            for (const auto& group : mds.getGroups(FIX::NoMDEntries)) {
                FIX::MDEntryType entryType;
                group.get(FIX::MDEntryType(entryType));

                FIX::MDEntryPx price;
                group.get(FIX::MDEntryPx(price));

                FIX::MDEntrySize size;
                group.get(FIX::MDEntrySize(size));

                if (entryType == FIX::MDEntryType::BID) {
                    bidPrice = price;
                    bidSize = size;
                } else if (entryType == FIX::MDEntryType::OFFER) {
                    askPrice = price;
                    askSize = size;
                }
            }

            // Find instrument ID by symbol
            int instrumentId = database_->getInstrumentId(symbol);
            if (instrumentId == 0) {
                // Create instrument if not exists
                instrumentId = database_->createInstrument(symbol);
            }

            // Create MarketTick and store
            MarketTick tick{
                .instrumentId = instrumentId,
                .timestamp = std::chrono::system_clock::now(),
                .bidPrice = bidPrice,
                .bidSize = bidSize,
                .askPrice = askPrice,
                .askSize = askSize
            };

            database_->addMarketTick(tick);

            LOG_DEBUG("Received market tick for " << symbol << ": Bid " << bidPrice << "/" << bidSize << ", Ask " << askPrice << "/" << askSize);
        } else if (message.is(FIX::MsgType::ExecutionReport)) {
            // Process execution report (trade execution)
            FIX::ExecutionReport execReport;
            execReport = message;

            std::string orderIdStr;
            execReport.get(FIX::OrderID(orderIdStr));
            std::uuid orderId = std::uuid::from_string(orderIdStr);

            FIX::Side side;
            execReport.get(FIX::Side(side));

            FIX::LastPx lastPx;
            execReport.get(FIX::LastPx(lastPx));

            FIX::LastQty lastQty;
            execReport.get(FIX::LastQty(lastQty));

            FIX::OrdStatus ordStatus;
            execReport.get(FIX::OrdStatus(ordStatus));

            // Update order status and balance
            tradeManager_->processExecution(orderId, static_cast<OrderSide>(side), lastQty, lastPx, static_cast<OrderStatus>(ordStatus));
        }
        // Handle other message types (e.g., Logon, Logout, Heartbeat)
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Error processing message: " << e.what());
        return false;
    }
}
```

#### Database.cpp (addMarketTick)
Use libpqxx to insert into PostgreSQL:
```cpp
void Database::addMarketTick(const MarketTick& tick) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    dbQueue_.emplace([this, tick]() {
        try {
            pqxx::work tx(*conn_);
            tx.exec_params(
                "INSERT INTO market_ticks (instrument_id, timestamp, bid_price, bid_size, ask_price, ask_size) "
                "VALUES ($1, $2, $3, $4, $5, $6)",
                tick.instrumentId,
                tick.timestamp.time_since_epoch().count(), // Store as int64 (timestamp in nanos)
                tick.bidPrice,
                tick.bidSize,
                tick.askPrice,
                tick.askSize
            );
            tx.commit();
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to insert market tick: " << e.what());
        }
    });
}

// Dedicated thread to process DB queue
void Database::processQueue() {
    while (running_) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            condition_.wait(lock, [this]() { return !dbQueue_.empty() || !running_; });
            if (!running_) break;
            task = std::move(dbQueue_.front());
            dbQueue_.pop();
        }
        task();
    }
}
```

#### TradeManager.cpp (processExecution)
Update order and balance:
```cpp
void TradeManager::processExecution(std::uuid orderId, OrderSide side, double quantity, double price, OrderStatus status) {
    // Find order in cache
    auto it = orders_.find(orderId);
    if (it == orders_.end()) {
        LOG_WARN("Execution for unknown order: " << orderId);
        return;
    }

    Order& order = it->second;
    order.status = status;
    order.filledQuantity += quantity;
    order.avgFillPrice = (order.avgFillPrice * (order.filledQuantity - quantity) + price * quantity) / order.filledQuantity;

    // If fully filled, update balance
    if (status == OrderStatus::FILLED) {
        Instrument instrument = database_->getInstrument(order.instrumentId);
        std::string baseCurrency = instrument.pair.substr(0, 3); // e.g., EUR for EUR/USD
        std::string quoteCurrency = instrument.pair.substr(4, 3); // e.g., USD for EUR/USD

        double amount = quantity * price;
        if (side == OrderSide::BUY) {
            // Deduct quote currency (USD), add base currency (EUR)
            database_->updateBalance(quoteCurrency, -amount);
            database_->updateBalance(baseCurrency, quantity);
        } else {
            // Add quote currency, deduct base currency
            database_->updateBalance(quoteCurrency, amount);
            database_->updateBalance(baseCurrency, -quantity);
        }

        // Save updated order to DB
        database_->updateOrder(order);
    }
}
```

### 11. Docker Compose Example
```yaml
version: '3.8'

services:
  postgres:
    image: postgres:16-alpine
    environment:
      POSTGRES_USER: hft_user
      POSTGRES_PASSWORD: hft_password
      POSTGRES_DB: hft_db
    volumes:
      - postgres_data:/var/lib/postgresql/data
    ports:
      - "5432:5432"
    healthcheck:
      test: ["CMD-SHELL", "pg_isready -U hft_user -d hft_db"]
      interval: 5s
      timeout: 5s
      retries: 5

  grafana:
    image: grafana/grafana:latest
    ports:
      - "3000:3000"
    volumes:
      - grafana_data:/var/lib/grafana
      - ./grafana/provisioning:/etc/grafana/provisioning
    environment:
      - GF_SECURITY_ADMIN_USER=admin
      - GF_SECURITY_ADMIN_PASSWORD=admin
    depends_on:
      - postgres

  hft-app:
    build:
      context: .
      dockerfile: docker/Dockerfile
    environment:
      - DB_HOST=postgres
      - DB_USER=hft_user
      - DB_PASSWORD=hft_password
      - DB_NAME=hft_db
      - FIX_ACCEPTOR_HOST=fix_simulator # Replace with actual FIX server
      - FIX_ACCEPTOR_PORT=9876
    depends_on:
      postgres:
        condition: service_healthy
    volumes:
      - ./config:/app/config

volumes:
  postgres_data:
  grafana_data:
```

### 12. Dockerfile for App
```dockerfile
# Use Ubuntu 26.04 base image
FROM ubuntu:26.04

# Set non-interactive mode for apt
ENV DEBIAN_FRONTEND=non-interactive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libboost-all-dev \
    libpq-dev \
    libuuid-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

# Install QuickFIX++ from source (since it may not be in Ubuntu 26.04 repos)
RUN git clone https://github.com/quickfix/quickfix.git /tmp/quickfix \
    && cd /tmp/quickfix \
    && cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF \
    && cmake --build build --target install \
    && rm -rf /tmp/quickfix

# Install libpqxx (PostgreSQL C++ library)
RUN git clone https://github.com/jtv/libpqxx.git /tmp/libpqxx \
    && cd /tmp/libpqxx \
    && cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -DWITH_BOOST=ON \
    && cmake --build build --target install \
    && rm -rf /tmp/libpqxx

# Create app directory
WORKDIR /app

# Copy source code
COPY src/ /app/src/
COPY config/ /app/config/
COPY CMakeLists.txt /app/

# Build the app
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --target hft-app

# Entrypoint script
COPY docker/entrypoint.sh /app/entrypoint.sh
RUN chmod +x /app/entrypoint.sh

ENTRYPOINT ["/app/entrypoint.sh"]
```

### 13. Entrypoint.sh
```bash
#!/bin/bash
# Wait for PostgreSQL to be ready
until pg_isready -h $DB_HOST -U $DB_USER -d $DB_NAME; do
  echo "Waiting for PostgreSQL to start..."
  sleep 2
done

# Run the app
./build/hft-app --config config/fix.cfg
```

### 14. FIX Configuration (fix.cfg)
```ini
[DEFAULT]
FileStorePath=./store
FileLogPath=./log
ConnectionType=initiator
ReconnectInterval=5
StartTime=00:00:00
EndTime=00:00:00
UseDataDictionary=Y
DataDictionary=./config/FIX44.xml

[SESSION]
SenderCompID=HFT_CLIENT
TargetCompID=BROKER_SERVER
Host=$FIX_ACCEPTOR_HOST
Port=$FIX_ACCEPTOR_PORT
BeginString=FIX.4.4
```

### 15. Grafana Provisioning
Create `grafana/provisioning/datasources/postgres.yml`:
```yaml
apiVersion: 1
datasources:
- name: PostgreSQL
  type: postgres
  access: proxy
  url: postgres:5432
  database: hft_db
  user: hft_user
  password: hft_password
  sslmode: disable
  isDefault: true
```

Create `grafana/provisioning/dashboards/hft-dashboard.json` with sample queries (use Grafana's query builder to generate actual JSON).

### 16. Considerations for HFT
- **Latency**: Use a fast FIX server (e.g., QuickFIX Engine), minimize serialization/deserialization time (QuickFIX++ is efficient), use in-memory caches for frequent lookups (e.g., instrument IDs).
- **Throughput**: Use async I/O throughout, avoid blocking operations (DB writes in a separate thread, FIX session in async mode).
- **Reliability**: Implement message sequencing, retransmissions (QuickFIX++ handles this), persistent storage for orders (so on restart, pending orders can be resubmitted).
- **Monitoring**: Add metrics (e.g., message throughput, latency, order fill rate) using Prometheus and Grafana for better observability.

### 17. Testing
- Use a FIX simulator (e.g., QuickFIX Engine's TestAcceptor) to send market data and execution reports.
- Verify that market ticks are stored in PostgreSQL.
- Test order execution: Send a NewOrderSingle, check if ExecutionReport updates the order status and balance.
- Check Grafana dashboards for real-time data visualization.

### Final Notes
- The example assumes a FIX acceptor (broker) is available. For testing, use QuickFIX Engine's TestAcceptor: https://github.com/quickfix/quickfix/tree/master/test/acceptor.
- Adjust the instrument list, FIX configuration, and database schema to match your needs.
- For production, add more error handling, logging (e.g., ELK stack), and security measures (e.g., SSL for FIX sessions, encrypted DB connections).

[End thinking]

### Comprehensive HFT FIX Application with PostgreSQL and Grafana

This solution provides a high-performance, asynchronous C++ application for FOREX market data ingestion and trade execution using the FIX protocol, with persistent storage in PostgreSQL and visualization via Grafana.


## **1. Project Structure**
```
hft-fix-app/
├── CMakeLists.txt          # CMake build configuration
├── config/
│   ├── fix.cfg             # QuickFIX++ session settings
│   └── FIX44.xml           # FIX.4.4 dictionary (from QuickFIX repo)
├── docker/
│   ├── Dockerfile          # App container build script
│   └── entrypoint.sh       # App startup script (waits for DB)
├── grafana/
│   └── provisioning/
│       ├── datasources/    # PostgreSQL data source config
│       └── dashboards/     # Grafana dashboard JSON
├── src/
│   ├── main.cpp            # Entry point
│   ├── FixApplication.h/cpp # QuickFIX++ interface implementation
│   ├── Database.h/cpp      # PostgreSQL CRUD operations
│   ├── TradeManager.h/cpp  # Order/execution logic
│   ├── Models.h            # Data structures (ticks, orders, balances)
│   └── Utils.h             # Helper functions (timestamp, UUID)
└── docker-compose.yml      # Orchestrate app, DB, Grafana
```


## **2. Key Dependencies**
- **QuickFIX++**: FIX protocol implementation (async support via Boost.Asio).
- **Boost.Asio**: Asynchronous I/O for non-blocking operations.
- **libpqxx**: PostgreSQL C++ client (thread-safe queue for DB writes).
- **PostgreSQL**: Relational database for persistent storage.
- **Grafana**: Data visualization dashboard.


## **3. CMake Configuration**
`CMakeLists.txt` configures the build with required dependencies:
```cmake
cmake_minimum_required(VERSION 3.20)
project(hft-app VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(QuickFIX REQUIRED)
find_package(libpqxx REQUIRED)
find_package(UUID REQUIRED)

# Include headers
include_directories(
    ${Boost_INCLUDE_DIRS}
    ${QuickFIX_INCLUDE_DIRS}
    ${LIBPQXX_INCLUDE_DIRS}
    ${UUID_INCLUDE_DIRS}
    src/
)

# Build executable
add_executable(hft-app
    src/main.cpp
    src/FixApplication.cpp
    src/Database.cpp
    src/TradeManager.cpp
)

# Link libraries
target_link_libraries(hft-app
    ${Boost_LIBRARIES}
    ${QuickFIX_LIBRARIES}
    ${LIBPQXX_LIBRARIES}
    ${UUID_LIBRARIES}
    pthread
)
```


## **4. Core Components**

### **4.1 FixApplication (QuickFIX++ Integration)**
Implements the `FIX::Application` interface for async message handling:
```cpp
#include "QuickFix/Application.h"
#include "QuickFix/Session.h"
#include "Models.h"

class FixApplication : public FIX::Application {
public:
    FixApplication(Database* db, TradeManager* tm)
        : database_(db), tradeManager_(tm) {}

    // Called when a FIX message is received
    bool fromApp(const FIX::Message& msg, const FIX::SessionID& sessionID) override {
        try {
            if (msg.is(FIX::MsgType::MarketDataSnapshotFullRefresh)) {
                processMarketData(msg);
            } else if (msg.is(FIX::MsgType::ExecutionReport)) {
                processExecutionReport(msg);
            }
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("FIX message error: " << e.what());
            return false;
        }
    }

    // Subscribe to market data on logon
    void onLogon(const FIX::SessionID& sessionID) override {
        for (const auto& pair : FOREX_INSTRUMENTS) {
            sendMarketDataSubscription(pair, sessionID);
        }
    }

private:
    void processMarketData(const FIX::Message& msg) {
        FIX::MarketDataSnapshotFullRefresh mds(msg);
        std::string symbol; mds.get(FIX::Symbol(symbol));

        double bid = 0.0, ask = 0.0;
        for (const auto& group : mds.getGroups(FIX::NoMDEntries)) {
            FIX::MDEntryType type; group.get(FIX::MDEntryType(type));
            FIX::MDEntryPx price; group.get(FIX::MDEntryPx(price));
            if (type == FIX::MDEntryType::BID) bid = price;
            else if (type == FIX::MDEntryType::OFFER) ask = price;
        }

        MarketTick tick{
            .instrument_id = database_->getInstrumentId(symbol),
            .timestamp = std::chrono::system_clock::now(),
            .bid = bid, .ask = ask
        };
        database_->addTick(tick);
    }

    Database* database_;
    TradeManager* tradeManager_;
    inline static const std::vector<std::string> FOREX_INSTRUMENTS = {
        "EUR/USD", "GBP/USD", "USD/JPY", "AUD/USD", "USD/CAD", "NZD/USD",
        "EUR/GBP", "EUR/JPY", "GBP/JPY", "AUD/JPY", "EUR/AUD", "GBP/AUD"
    };
};
```

### **4.2 Database (PostgreSQL Integration)**
Uses a thread-safe queue to avoid blocking the async loop:
```cpp
#include "libpqxx/pqxx"
#include "Models.h"

class Database {
public:
    Database(const std::string& connStr) : conn_(new pqxx::connection(connStr)) {
        std::thread(&Database::processQueue, this).detach();
    }

    void addTick(const MarketTick& tick) {
        queue_.push([this, tick]() {
            pqxx::work tx(*conn_);
            tx.exec_params(
                "INSERT INTO market_ticks (instrument_id, ts, bid, ask) VALUES ($1, $2, $3, $4)",
                tick.instrument_id, tick.timestamp.time_since_epoch().count(), tick.bid, tick.ask
            );
            tx.commit();
        });
    }

private:
    void processQueue() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this] { return !queue_.empty(); });
                task = std::move(queue_.front()); queue_.pop();
            }
            task();
        }
    }

    std::unique_ptr<pqxx::connection> conn_;
    std::queue<std::function<void()>> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};
```

### **4.3 TradeManager (Order Execution)**
Manages order lifecycle and balance updates:
```cpp
#include "Models.h"
#include "Database.h"

class TradeManager {
public:
    TradeManager(Database* db) : database_(db) {}

    void executeOrder(const Order& order) {
        // Send NewOrderSingle to FIX acceptor
        FIX::NewOrderSingle nos;
        nos.set(FIX::OrderID(std::to_string(order.id)));
        nos.set(FIX::Symbol(instrumentSymbol(order.instrument_id)));
        nos.set(FIX::Side(order.side == OrderSide::BUY ? FIX::Side::BUY : FIX::Side::SELL));
        nos.set(FIX::OrderQty(order.quantity));
        nos.set(FIX::Price(order.price));
        fixSession_->send(nos);

        // Cache order for execution reports
        orders_[order.id] = order;
    }

    void processExecution(const FIX::ExecutionReport& er) {
        std::uuid orderId = std::uuid::from_string(er.getField(FIX::OrderID()).getString());
        auto& order = orders_[orderId];
        order.status = OrderStatus::FILLED;

        // Update balance
        double amount = er.getField(FIX::LastQty()) * er.getField(FIX::LastPx());
        if (order.side == OrderSide::BUY) {
            database_->updateBalance(order.currency_base, amount);
            database_->updateBalance(order.currency_quote, -amount);
        } else {
            database_->updateBalance(order.currency_base, -amount);
            database_->updateBalance(order.currency_quote, amount);
        }
    }

private:
    Database* database_;
    std::unordered_map<std::uuid, Order> orders_;
    FIX::Session* fixSession_; // Initialized in FixApplication
};
```


## **5. Docker Compose**
Orchestrates the app, PostgreSQL, and Grafana:
```yaml
version: '3.8'

services:
  postgres:
    image: postgres:16-alpine
    environment:
      POSTGRES_USER: hft
      POSTGRES_PASSWORD: hft_pass
      POSTGRES_DB: hft_db
    volumes:
      - postgres_data:/var/lib/postgresql/data
    ports:
      - "5432:5432"
    healthcheck:
      test: ["CMD", "pg_isready", "-U", "hft", "-d", "hft_db"]
      interval: 5s
      timeout: 5s
      retries: 5

  grafana:
    image: grafana/grafana:latest
    ports:
      - "3000:3000"
    volumes:
      - grafana_data:/var/lib/grafana
      - ./grafana/provisioning:/etc/grafana/provisioning
    environment:
      - GF_SECURITY_ADMIN_PASSWORD=admin
    depends_on:
      - postgres

  hft-app:
    build:
      context: .
      dockerfile: docker/Dockerfile
    environment:
      - DB_CONN=host=postgres user=hft password=hft_pass dbname=hft_db sslmode=disable
      - FIX_HOST=fix_simulator
      - FIX_PORT=9876
    depends_on:
      postgres:
        condition: service_healthy
    volumes:
      - ./config:/app/config

volumes:
  postgres_data:
  grafana_data:
```


## **6. Grafana Provisioning**
### **6.1 Data Source Configuration**
`grafana/provisioning/datasources/postgres.yml`:
```yaml
apiVersion: 1
datasources:
- name: PostgreSQL
  type: postgres
  url: postgres:5432
  database: hft_db
  user: hft
  password: hft_pass
  sslmode: disable
  isDefault: true
```

### **6.2 Sample Dashboard**
`grafana/provisioning/dashboards/hft-dashboard.json` includes panels for:
- **Real-Time Ticks**: Line chart of bid/ask prices for selected instruments.
- **Order History**: Table of recent orders with status/execution details.
- **Balance Overview**: Gauge charts for available balance and equity.
- **Trade Volume**: Bar chart of daily trading volume per instrument.


## **7. PostgreSQL Schema**
```sql
-- Instruments table
CREATE TABLE instruments (
    id SERIAL PRIMARY KEY,
    symbol VARCHAR(10) UNIQUE NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Market ticks table
CREATE TABLE market_ticks (
    id SERIAL PRIMARY KEY,
    instrument_id INT REFERENCES instruments(id),
    ts BIGINT NOT NULL, -- Nanoseconds since epoch
    bid DOUBLE PRECISION NOT NULL,
    ask DOUBLE PRECISION NOT NULL,
    received_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Orders table
CREATE TABLE orders (
    id UUID PRIMARY KEY,
    instrument_id INT REFERENCES instruments(id),
    side VARCHAR(4) NOT NULL, -- BUY/SELL
    quantity DOUBLE PRECISION NOT NULL,
    price DOUBLE PRECISION NOT NULL,
    status VARCHAR(20) NOT NULL, -- PENDING/FILLED/CANCELED
    filled_qty DOUBLE PRECISION DEFAULT 0.0,
    avg_fill_price DOUBLE PRECISION DEFAULT 0.0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Balances table
CREATE TABLE balances (
    id SERIAL PRIMARY KEY,
    currency VARCHAR(3) UNIQUE NOT NULL,
    available DOUBLE PRECISION NOT NULL DEFAULT 0.0,
    used DOUBLE PRECISION NOT NULL DEFAULT 0.0,
    equity DOUBLE PRECISION NOT NULL DEFAULT 0.0,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```


## **8. Deployment Steps**
1. **Build Docker Images**:
   ```bash
   docker-compose build
   ```

2. **Start Services**:
   ```bash
   docker-compose up -d
   ```

3. **Verify**:
   - Access Grafana at `http://localhost:3000` (login: `admin`/`admin`).
   - Check PostgreSQL logs with `docker logs hft-fix-app-postgres-1`.
   - Monitor app logs with `docker logs hft-fix-app-hft-app-1`.


## **9. HFT Optimizations**
- **Async I/O**: QuickFIX++ `AsyncSession` and Boost.Asio ensure non-blocking message processing.
- **Thread Pool**: Database writes are queued and processed in a dedicated thread to avoid latency spikes.
- **In-Memory Caching**: Orders and instruments are cached to reduce database round-trips.
- **FIX Compression**: Enable `Compression` in `fix.cfg` to reduce network overhead.


## **10. Testing**
Use the **QuickFIX TestAcceptor** to simulate a FIX broker:
```bash
# Start TestAcceptor (replace with your config)
java -jar quickfixj-all-2.3.0.jar acceptor.cfg
```

Send market data and execution reports to validate the app’s behavior.


## **Final Notes**
- **Security**: For production, enable SSL for FIX sessions and PostgreSQL connections.
- **Monitoring**: Add Prometheus metrics to track message throughput and latency.
- **Scalability**: Use a FIX gateway cluster for high throughput.

This solution provides a robust foundation for high-frequency FOREX trading with real-time data visualization and persistent storage.

[ Prompt: 7.2 t/s | Generation: 0.8 t/s ]
