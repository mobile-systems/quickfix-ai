[ocheretin@localhost workspace]$ ./llama.cpp/build/bin/llama-cli --model models/Qwen3.5-9B-GGUF/Qwen3.5-9B-Q4_K_M.gguf --reasoning off

Loading model...  


▄▄ ▄▄
██ ██
██ ██  ▀▀█▄ ███▄███▄  ▀▀█▄    ▄████ ████▄ ████▄
██ ██ ▄█▀██ ██ ██ ██ ▄█▀██    ██    ██ ██ ██ ██
██ ██ ▀█▄██ ██ ██ ██ ▀█▄██ ██ ▀████ ████▀ ████▀
                                    ██    ██
                                    ▀▀    ▀▀

build      : b9569-d9320477d
model      : Qwen3.5-9B-Q4_K_M.gguf
modalities : text

available commands:
  /exit or Ctrl+C     stop or exit
  /regen              regenerate the last response
  /clear              clear the chat history
  /read <file>        add a text file
  /glob <pattern>     add text files using globbing pattern


# Как подключить QuickFIX на C++ пример использования с cfg

Подключение и использование **QuickFIX** на C++ — это задача по созданию надежного торгового или коммуникационного приложения. QuickFIX — это низкоуровневая библиотека для работы с протоколом FIX (Financial Information eXchange), которая отдает всю логику сети и сериализации вам, оставляя только бизнес-логику на вашей стороне.

Ниже приведен полный пример реализации: от настройки конфигурации (`config.cfg`) до кода C++, который реализует обработку сообщений.

### 1. Структура проекта

Для начала подготовьте директорию проекта. Вам понадобятся файлы конфигурации и исходный код.

Структура:
```text
my_project/
├── config.cfg
├── fix_engine.cpp
├── fix_engine.h
├── CMakeLists.txt (опционально, или makefile)
└── main.cpp
```

---

### 2. Конфигурационный файл (`config.cfg`)

QuickFIX использует XML-подобный синтаксис (на самом деле это простой текст) для описания подключений, сессий и логики.

**Создайте файл `config.cfg`:**

```xml
<fix>
  <!-- Настройки самого QuickFIX -->
  <Application>
    <!-- Укажите путь к вашему классу приложения -->
    <MyApp fixengine="fix_engine.cpp"/>
  </Application>
  
  <Session>
    <!-- Идентификатор сессии -->
    <MsgType>
      <Value>8=</Value>
    </MsgType>
    <TargetCompID>YOUR_TARGET_ID</TargetCompID>
    <BeginString>FIX.4.4</BeginString>
    <StartTime>00:00:00</StartTime>
    <EndTime>23:59:59</EndTime>
    <HeartBtInt>30</HeartBtInt>
    <DataDictionary>
      <FilePath>../src/data_dictionary.xml</FilePath>
    </DataDictionary>
    <!-- Ссылка на ваш класс сессии -->
    <MySession fixengine="fix_engine.cpp"/>
  </Session>
</fix>
```

> **Важно:** В реальном продакшене вы часто загружаете `data_dictionary.xml` динамически или используете встроенные в библиотеку дикта, но для примера покажем путь к файлу.

---

### 3. Логика приложения (`fix_engine.h` и `.cpp`)

Здесь мы определяем классы, которые наследуются от базовых классов QuickFIX (`Application` и `Session`), а затем инициализируем движок.

#### Файл `fix_engine.h`

```cpp
#ifndef FIX_ENGINE_H
#define FIX_ENGINE_H

#include "fix/SessionSet.h"
#include "fix/SocketInitiator.h"
#include "fix/Initiator.h"
#include "fix/ConfigFile.h"
#include "fix/Message.h"
#include "fix/Field.h"
#include "fix/SessionID.h"
#include "fix/MessageFactory.h"
#include <string>
#include <iostream>

// 1. Обработка входящих сообщений (Application)
class MyApp : public quickfix::Application {
public:
    MyApp() : quickfix::Application() {}

    // Обработка входящего сообщения (например, Order)
    int onMessage(quickfix::Message& msg, quickfix::DataDictionary& dtd) override {
        std::cout << "Входящее сообщение: " << msg.GetField(quickfix::Tag::MsgType) << std::endl;

        // Пример: Извлечение поля OrderID
        quickfix::Field orderID = msg.getField(quickfix::Tag::OrderID);
        if (orderID.isSet()) {
            std::cout << "OrderID: " << orderID.getStringValue() << std::endl;
        } else {
            std::cout << "OrderID не установлен" << std::endl;
        }

        return quickfix::RF_OK;
    }

    // Обработка ошибок (например, ошибка связи)
    int toAdmin(quickfix::Message& msg, quickfix::DataDictionary& dtd,
                quickfix::SessionID& sessionId, const quickfix::FieldMap& msgFields) override {
        std::cout << "Ошибка сессии: " << sessionId.toString() << std::endl;
        return quickfix::RF_OK;
    }

    // Подтверждение отправки (отправителя)
    int toQuote(quickfix::Message& msg, quickfix::DataDictionary& dtd,
                quickfix::SessionID& sessionId, const quickfix::FieldMap& msgFields) override {
        std::cout << "Отправлено сообщение" << std::endl;
        return quickfix::RF_OK;
    }
};

// 2. Логика сессии (Session)
class MySession : public quickfix::Session {
public:
    MySession(const quickfix::ConfigFile& config, const quickfix::SessionID& id)
        : quickfix::Session(config, id) {}
};

// 3. Главный класс, запускающий движок
class FixEngine {
public:
    FixEngine(const std::string& configPath) {
        loadConfig(configPath);
        createSessionSet();
        initSocket();
        startEngine();
    }

    ~FixEngine() {
        stop();
    }

private:
    quickfix::ConfigFile m_config;
    quickfix::SessionSet m_sessions;
    quickfix::SocketInitiator m_initiator;
    int m_stopRequested = 0;

    void loadConfig(const std::string& path) {
        if (!m_config.fromFile(path)) {
            std::cerr << "Не удалось загрузить конфигурацию: " << path << std::endl;
        }
    }

    void createSessionSet() {
        // Создаем экземпляр нашего приложения и сессии
        MyApp myApp;
        MySession mySession(m_config, quickfix::SessionID("TARGET_ID", "SESSION_ID")); // ID должны совпадать с cfg

        m_sessions.addSession(mySession);
        m_sessions.addApplication(myApp);
    }

    void initSocket() {
        // Инициализация socket (порт и хост)
        quickfix::SocketInitiator::init(9999, "127.0.0.1"); // Порт и хост
    }

    void startEngine() {
        m_initiator.setSessionSet(m_sessions);
        m_initiator.start();
    }

    void stop() {
        m_stopRequested = 1;
        m_initiator.stop();
    }
};

#endif // FIX_ENGINE_H
```

---

### 4. Точка входа (`main.cpp`)

Этот файл создает экземпляр движка и запускает его.

```cpp
#include "fix_engine.h"
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
    try {
        // Путь к конфигурационному файлу (относительный или абсолютный)
        std::string configPath = "config.cfg";
        
        // Создание движка
        FixEngine engine(configPath);

        std::cout << "QuickFIX запущен. Ожидание событий..." << std::endl;

        // Бесконечный цикл для работы с обработкой событий (если нужен контроль)
        // В современных версиях QuickFIX (v1.11+) инициализатор сам управляет циклом.
        // Здесь мы просто даем ему время работать.
        while (true) {
            // Можно проверять флаг остановки, если добавить обработчик сигналов
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

    } catch (const quickfix::Exception& e) {
        std::cerr << "Ошибка QuickFIX: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Стандартная ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

---

### 5. Сборка проекта

Для сборки вам понадобится `cmake`. Ниже пример `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyQuickFixProject)

# Поиск QuickFIX (установленного через apt, vcpkg или скачанного вручную)
# Если установлен через apt (libquickfix-dev):
find_package(QuickFIX REQUIRED)

# Создание исполняемого файла
add_executable(my_fix_app main.cpp fix_engine.cpp)

# Ссылка на библиотеку QuickFIX
target_link_libraries(my_fix_app ${QUICKFIX_LIBRARIES})

# Если используете пакеты apt, также нужно включить pthread для socket
target_link_libraries(my_fix_app pthread)
```

Команды сборки:
```bash
mkdir build && cd build
cmake ..
make
```

---

### 6. Ключевые моменты и нюансы

1.  **DataDictionary**: QuickFIX критически зависит от правильного файла `data_dictionary.xml`. Без него он не сможет распарсить поля. Обычно этот файл берется из официального сайта FIX или поставляется вместе с библиотекой. Убедитесь, что путь в `config.cfg` верный.
2.  **SocketInitiator vs Acceptor**: В примере используется `SocketInitiator`. Это значит, что ваше приложение будет **клиентом** (отправлять запросы). Если вы хотите быть сервером (принимать подключения), используйте `SocketAcceptor`. Логика в `main` и `config` меняется незначительно (меняется класс инициализатора).
3.  **Обработка ошибок**: Методы `toAdmin`, `toQuote` и `onMessage` возвращают статус. `RF_REJECT` отклоняет сообщение, `RF_REJECT_MESSAGE` — отклоняет само сообщение целиком.
4.  **Наследование**: QuickFIX требует строгого наследования от классов `Application` и `Session`. Вы не можете создать их напрямую, только подклассы.
5.  **Синхронизация**: `SocketInitiator` запускается асинхронно, но методы `onMessage` выполняются синхронно в потоке приложения. Не блокируйте этот поток на длителькие операции.

### Как протестировать?

Чтобы протестировать пример, вам нужно:
1.  Установить зависимости (`libquickfix-dev` для Debian/Ubuntu или соответствующий пакет для вашей ОС).
2.  Скомпилировать код.
3.  Запустить ваше приложение.
4.  Подключить к нему тестовый клиент (например, **QuickFIXn** для Java или **QuickFIX** для Python/C#), который отправит вам тестовое сообщение (например, `Logon` или `NewOrderSingle`), чтобы увидеть вывод в консоли C++.

[ Prompt: 14,7 t/s | Generation: 4,3 t/s ]

# пример с получением котировок по инструменту EURUSD и записью в PostgreSQL

Реализация получения котировок (Market Data) в FIX обычно осуществляется через **Market Data Request (MDR)** сообщения. Для парсинга цены используется поле `MDPrice` (тег 499 или 599 в зависимости от типа сообщения) или `MDPriceDelta`.

Однако, важно понимать архитектурный нюанс: **QuickFIX сам по себе не умеет подключаться к PostgreSQL**. Для этого мы используем стандартный C++ шаблон **Producer-Consumer** с использованием очереди (Queue) или разделяемой памяти.

Ниже приведен полный пример архитектуры:
1.  **FIX Engine**: Слушает рынок, получает тик, конвертирует его в объект `TickData`.
2.  **Message Queue**: Блокирует поток FIX, пока база данных обрабатывает данные (избегает блокировки сети).
3.  **Database Writer**: Частый поток, который читает из очереди и вставляет данные в PostgreSQL.

### 1. Предварительные требования

Вам понадобятся:
*   QuickFIX (C++)
*   libpqxx (библиотека PostgreSQL для C++)
*   CMake

**Подключение libpqxx в `CMakeLists.txt`:**
```cmake
find_package(pqxx REQUIRED)
target_link_libraries(my_fix_app pqxx)
```

### 2. Структура данных и Очередь

Сначала определим структуру данных для тика и очередь.

```cpp
// tick_data.h
#ifndef TICK_DATA_H
#define TICK_DATA_H

#include <string>
#include <ctime>

struct TickData {
    std::string instrument; // Например: "EURUSD"
    double bid;
    double ask;
    long long timestamp; // Время в микросекундах

    TickData(std::string inst, double b, double a, long long t)
        : instrument(inst), bid(b), ask(a), timestamp(t) {}
};

// Простая реализация очереди (в реальном проекте лучше использовать std::queue + mutex)
#include <queue>
#include <mutex>

template <typename T>
class SimpleQueue {
public:
    void push(T item) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(item);
    }

    bool pop(T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) return false;
        item = queue_.front();
        queue_.pop();
        return true;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!queue_.empty()) queue_.pop();
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
};

#endif
```

### 3. Логика приложения QuickFIX (`fix_app.cpp` / `fix_app.h`)

Этот класс наследуется от `quickfix::Application`. Мы реализуем обработку входящих сообщений Market Data.

```cpp
// fix_app.h
#ifndef FIX_APP_H
#define FIX_APP_H

#include "quickfix/Session.h"
#include "quickfix/SessionSet.h"
#include "quickfix/Message.h"
#include "quickfix/Field.h"
#include "quickfix/Initiator.h"
#include "quickfix/SocketInitiator.h"
#include "quickfix/ConfigFile.h"
#include "fix_engine.h" // Включаем определение FixEngine если нужно, или создаем ниже
#include <functional>

#include "tick_data.h"
#include "SimpleQueue.h"
#include <pqxx/pqxx>

// Глобальная очередь для обмена данными
// В реальном проекте лучше передавать по указателю в конструктор, но для примера сделаем глобально или через класс
class GlobalDBWriter {
public:
    void writeTick(const TickData& tick) {
        // Здесь будет код записи в БД
        // Используем лямбду или отдельный поток
    }
};

class MarketDataApp : public quickfix::Application {
public:
    // Указатель на очередь извне
    SimpleQueue<TickData>* tickQueue = nullptr;

    MarketDataApp(SimpleQueue<TickData>* q) : tickQueue(q) {}

    // Обработка входящего Market Data (например, Market Data Request Response или Snapshot)
    int onMessage(quickfix::Message& msg, quickfix::DataDictionary& dtd) override {
        quickfix::Field symbolField = msg.getField(quickfix::Tag::Symbol); // Тег 55
        quickfix::Field mdrTypeField = msg.getField(quickfix::Tag::MDMktDataMsgType); // Тег 274 (пример)
        
        if (symbolField.isSet()) {
            std::string symbol = symbolField.getStringValue();
            
            // Извлечение цены (пример: тег 499 MDPrice или 599)
            // Примечание: Реальные тэги зависят от спецификации (FIX 4.4 vs FIX 4.2 SP2 и т.д.)
            // Для примера берем поле 499 (MDPrice)
            quickfix::Field priceField = msg.getField(quickfix::Tag::MDPrice); 
            
            // Если цена есть, создаем тик
            if (priceField.isSet()) {
                double price = priceField.getDoubleValue();
                
                // Если это Bid/Ask, обычно приходят двумя сообщениями или в одном с тэгами 57 (Bid) и 58 (Ask)
                // Для упрощения считаем, что приходит Ask (или Bid). 
                // В реальной жизни нужно проверять тег 57 (BidPrice) и 58 (AskPrice)
                
                long long now = std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::system_clock::now().time_since_epoch()
                ).count();

                // Создаем структуру тика
                // Внимание: Здесь логика извлечения Bid/Ask должна быть адаптирована под конкретный тег
                // Допустим, мы получаем Bid и Ask отдельно или из одного сообщения
                TickData tick(symbol, price, price, now); 

                // ВАЖНО: Чтобы получить Bid и Ask отдельно, нужно смотреть на тэги:
                // 57 (BidPrice), 58 (AskPrice), 59 (BidSize), 60 (AskSize)
                // Если тег 499 установлен, это обычно цена обновления.
                
                // Пример более реалистичный:
                quickfix::Field bidField = msg.getField(quickfix::Tag::BidPrice);
                quickfix::Field askField = msg.getField(quickfix::Tag::AskPrice);
                
                double finalBid = 0, finalAsk = 0;
                if (bidField.isSet()) finalBid = bidField.getDoubleValue();
                if (askField.isSet()) finalAsk = askField.getDoubleValue();

                if (finalBid == 0) finalBid = price; // Дефолт, если только одна цена
                if (finalAsk == 0) finalAsk = price;

                TickData realTick(symbol, finalBid, finalAsk, now);
                
                // Добавляем в очередь (блокирует поток FIX пока не добавит, но быстро)
                tickQueue->push(realTick);
                std::cout << "Получен тик: " << symbol << " Bid: " << realTick.bid << " Ask: " << realTick.ask << std::endl;
            }
        }
        return quickfix::RF_OK;
    }

    // Обработка ошибок
    int toAdmin(quickfix::Message& msg, quickfix::DataDictionary& dtd, 
                quickfix::SessionID& sessionId, const quickfix::FieldMap& msgFields) override {
        std::cout << "Admin: " << msg.toString() << std::endl;
        return quickfix::RF_OK;
    }
};

#endif
```

### 4. Основной файл (`main.cpp`)

Здесь мы соединяем FIX Engine, Очередь и PostgreSQL. Мы создаем два потока: один для работы с сетью, другой — для базы данных.

```cpp
#include "fix_app.h"
#include "fix_engine.h" // Предполагается, что здесь объявлен класс FixEngine
#include <thread>
#include <chrono>
#include <iostream>
#include <string>

// Функция записи в PostgreSQL
void writeToPostgres(const TickData& tick) {
    // Подключение к БД
    std::string conn_str = "dbname=fix_db user=postgres password=secret host=localhost port=5432";
    
    try {
        pqxx::connection conn(conn_str);
        // Автоматическая транзакция
        pqxx::work tx(conn);
        
        // Создание таблицы, если её нет (для примера)
        // В продакшене таблицу создают заранее
        tx.exec(R"(
            CREATE TABLE IF NOT EXISTS ticks (
                id SERIAL PRIMARY KEY,
                instrument VARCHAR(255) NOT NULL,
                bid DOUBLE PRECISION NOT NULL,
                ask DOUBLE PRECISION NOT NULL,
                timestamp BIGINT NOT NULL
            )
        )");

        // Вставка данных
        // Используем параметризованные запросы для безопасности
        std::string sql = "INSERT INTO ticks (instrument, bid, ask, timestamp) VALUES ($1, $2, $3, $4)";
        pqxx::result res = tx.exec(sql, 
            tick.instrument, 
            tick.bid, 
            tick.ask, 
            tick.timestamp);
            
        std::cout << "Записано в БД: " << tick.instrument << " | " << tick.bid << " | " << tick.ask << std::endl;
        
        tx.commit();

    } catch (const pqxx::database_error& e) {
        std::cerr << "Ошибка БД: " << e.what() << std::endl;
    } catch (const pqxx::generic_error& e) {
        std::cerr << "Ошибка БД: " << e.what() << std::endl;
    }
}

int main() {
    try {
        // 1. Инициализация очереди
        SimpleQueue<TickData> tickQueue;

        // 2. Создание приложения QuickFIX
        MarketDataApp app(&tickQueue);

        // 3. Настройка и запуск инициатора (клиента)
        // Создаем фиктивный движок или используем SocketInitiator напрямую
        quickfix::SessionSet sessions;
        quickfix::ConfigFile config("config.cfg"); // Путь к вашему config.cfg
        
        // Добавляем сессию (нужен SessionID, обычно совпадает с CFG)
        // Для примера создадим сессию вручную, так как в CFG путь к DataDictionary важен
        quickfix::SessionID sessionID("YOUR_TARGET_ID", "SESSION_ID"); 
        
        quickfix::Session session(config, sessionID);
        sessions.addSession(session);
        sessions.addApplication(app);

        quickfix::SocketInitiator initiator;
        initiator.setSessionSet(sessions);
        
        // Подключение к бирже (например, IQFeed или другой провайдер)
        // Формат: "host:port"
        initiator.start("market_data_server", "9999"); 

        std::cout << "FIX Engine запущен. Ожидание данных..." << std::endl;

        // 4. Запуск потока записи в БД (Consumer)
        // Этот поток будет забирать данные из очереди и писать в ПГ
        std::thread dbThread([&tickQueue]() {
            // Пробуем писать с интервалом или по факту.
            // Чтобы не перегружать БД при высокой частоте, используем sleep
            // Но лучше использовать блокирующую попытку извлечь из очереди
            while (true) {
                TickData tick;
                // Попробуем достать тик из очереди
                // Если очередь пуста, ждем немного
                bool gotData = tickQueue.pop(tick);
                
                if (gotData) {
                    // Записываем асинхронно
                    // Можно сделать в отдельном потоке внутри этой функции, 
                    // но для простоты делаем здесь (блокирует поток DB, но не FIX)
                    // В продакшене лучше вынести запись в отдельный поток внутри функции pop
                    writeToPostgres(tick);
                } else {
                    // Ждем, пока придут данные
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
        });

        // Основной поток ждет завершения FIX (или выхода Ctrl+C)
        // В реальном приложении здесь нужно обрабатывать KeyboardInterrupt
        std::cout << "Получатель БД запущен. Запустите FIX-клиент..." << std::endl;

        // Бесконечный цикл для мониторинга (или до сигнала остановки)
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

    } catch (const quickfix::Exception& e) {
        std::cerr << "Ошибка QuickFIX: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Стандартная ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

### 5. Настройка `config.cfg`

Чтобы это сработало, создайте `config.cfg`, указав путь к словарю данных.

```xml
<fix>
  <Application>
    <fixengine>main.cpp</fixengine> <!-- Указываем имя класса, если используется reflection, но обычно в C++ класс создается явно -->
    <MyApp fixengine="main.cpp"/>
  </Application>
  
  <Session>
    <MsgType>
      <Value>MD=</Value> <!-- Тип сообщения Market Data -->
    </MsgType>
    <TargetCompID>YourExchangeID</TargetCompID>
    <BeginString>FIX.4.4</BeginString>
    <StartTime>00:00:00</StartTime>
    <EndTime>23:59:59</EndTime>
    <HeartBtInt>30</HeartBtInt>
    <!-- Путь к DataDictionary для парсинга -->
    <DataDictionary>
      <FilePath>src/data_dictionary.xml</FilePath>
    </DataDictionary>
    <MySession fixengine="main.cpp"/>
  </Session>
</fix>
```

### 6. Как это работает (Архитектура потоков)

1.  **Поток A (FIX Network Thread)**:
    *   `SocketInitiator` работает в этом потоке.
    *   Когда приходит сообщение Market Data, вызывается `onMessage`.
    *   Внутри `onMessage` мы извлекаем цену и добавляем `TickData` в `tickQueue`.
    *   **Критически важно**: Добавление в `std::queue` происходит очень быстро (микросекунды). Поток FIX не блокируется ожиданием записи в БД.

2.  **Поток B (Database Consumer Thread)**:
    *   Этот поток бесконечно ждет в `tickQueue.pop()`.
    *   Как только тик появляется, он извлекается и отправляется в `writeToPostgres`.
    *   Если запись в БД занимает время (например, 50 мс), поток FIX продолжает работать и получать новые котировки без задержки.

3.  **ПостgreSQL**:
    *   Использует `libpqxx` для надежной работы с транзакциями.
    *   Использует параметризованные запросы (`$1`, `$2`) для предотвращения SQL-инъекций.

### Важные замечания для продакшена

1.  **Парсинг Bid/Ask**: В примере использована упрощенная логика. Реальный Market Data часто приходит в виде `Snapshot` (мгновенная картина), где есть и Bid, и Ask в одном сообщении. Вам нужно правильно отфильтровать тег `MDMktDataMsgType` (например, `SNAPSHOT` или `SNAPSHOT_RESTORE`) и извлечь поля `BidPrice` (57) и `AskPrice` (58).
2.  **Синхронизация**: `SimpleQueue` использует `std::mutex`. Это безопасно, но если частота тиков очень высока (10k+ в секунду), `std::queue` может стать узким местом. Для высоких частот используют `RingBuffer` без блокировок или разделяемую память.
3.  **DataDictionary**: Убедитесь, что файл `data_dictionary.xml` соответствует версии FIX (FIX 4.2, 4.3, 4.4), которую вы используете. Без него QuickFIX не сможет распознать тэги 57, 58, 59, 60.
4.  **Тестирование**: Для теста можно использовать клиент `iqfeed` или написать простой скрипт на Python/Java, который будет эмулировать поток Market Data, отправляя сообщения на ваш порт.

[ Prompt: 25,3 t/s | Generation: 3,6 t/s ]