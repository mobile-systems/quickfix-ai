# Как подключить QuickFIX на C++ пример использования с cfg


<details>
<summary><img src=Reasoning.png></summary>
<pre>
Мы создадим простой пример подключения к биржевому серверу с использованием QuickFIX на C++.
Для этого нам понадобится:

Установить QuickFIX на системе.
Создать конфигурационный файл (например, fix.cfg).
Написать программу, которая читает конфигурацию и подключается к сессии.
Шаги программы:

Инициализация логирования (опционально, но рекомендуется).
Создание экземпляра сессии (Session) с использованием конфигурации.
Обработка сообщений (MessageCrumbler) и отправка примера сообщения.
Пример конфигурационного файла fix.cfg:

[DEFAULT]
ConnectionType=initiator
SleepTime=10
StartDatetime=20000101-09:00:00
EndDatetime=20000101-17:00:00
ReconnectInterval=60

[SESSION]
SenderCompID=YourSenderCompID
TargetCompID=YourTargetCompID
SocketConnectHost=fix.example.com
SocketConnectPort=5000
UseTLS=true
TLSAcceptAllCiphers=true
TLSCertFile=server.pem
TLSKeyFile=client.key
TLSTrustFile=ca.pem

В этом примере:

ConnectionType указывает, что мы initiator (инициатор).
SleepTime - интервал между отправкой сообщений в milliseconds.
StartDatetime и EndDatetime - время работы сессии (можно удалить для бесконечной работы).
ReconnectInterval - интервал переподключения в секундах.
Для TLS нам потребуется сертификат сервера (TLSCertFile), ключ клиента (TLSKeyFile) и файл с доверенными сертификатами (TLSTrustFile).

Теперь код на C++:

Подключим необходимые заголовочные файлы.
Создадим класс, обрабатывающий сообщения (MessageCrumbler).
Инициализируем сессию с помощью SessionSettings и Session.
Запустим сессию и подождем, пока она не подключится.
Важно: Этот пример предполагает, что у вас установлен и сконфигурирован QuickFIX.

Примечание: Вместо YourSenderCompID и YourTargetCompID нужно указать свои значения, соответствующие вашему сертификату и соглашению с биржей.

Также, для простоты, мы не будем обрабатывать сложные ошибки и просто отправим пример сообщение и будем ждать ответ.

Давайте напишем код:
</pre>
</details>

Вот пример кода на C++ для подключения к FIX-серверу с использованием QuickFIX и конфигурации в файле `fix.cfg`:

**1. Создание конфигурационного файла `fix.cfg`**:
```ini
[DEFAULT]
ConnectionType=initiator
SenderCompID=FIXUser
TargetCompID=ServerID
BeginString=FIX.4.2
SocketConnectHost=127.0.0.1
SocketConnectPort=5000
UseCompression=False
FileStorePath=./store
HeartBtInt=30
ResetOnLogon=False
StartTime=00:00:00
EndTime=23:59:59
```

**2. Код программы `fix_client.cpp`**:
```cpp
#include <QuickFIX/Initiator.h>
#include <QuickFIX/SessionSettings.h>
#include <QuickFIX/FileStore.h>
#include <QuickFIX/SleepManagerStandard.h>
#include <QuickFIX/MessageCrumbler.h>
#include <QuickFIX/Log.h>
#include <QuickFIX/SessionID.h>

using namespace QuickFIX;
using namespace QuickFIX::examples;

// Обработчик сообщений
class MyMessageCrumbler : public MessageCrumbler {
public:
    MyMessageCrumbler() : MessageCrumbler() {}
    virtual void onMessage(mf4::Order o, SessionID &id) {}
    virtual void onCreate(SessionID id) override {
        FIX::Order o;
        o.setField(FIX::ClOrdID("ID-1"));
        o.setField(FIX::Symbol("AAPL"));
        o.setField(FIX::Side(1));
        o.setField(FIX::Qty(100));
        o.setField(FIX::OrdType(1));
        o.setField(FIX::TimeInForce(2));
        o.setField(FIX::TransactTime());
        o.setField(FIX::HandlInst('/'));
        send(o);
    }
};

int main() {
    Initiator initiator(MsgStoreFactory::createFileStoreFactory(), LogFactory::createDefault(),
                        SleepManagerStandard(), "fix.cfg");
    initiator.start();
    MyMessageCrumbler crumbler;
    initiator.addCrumbler(crumbler);
    while (true) {
        SleepManagerStandard::sleep(1000);
    }
    initiator.stop();
    return 0;
}
```

**3. Инструкция по сборке**:
1. Установите QuickFIX:
   ```bash
   git clone https://github.com/robert-urner/QuickFIX
   cd QuickFIX
   mkdir build && cd build
   cmake ..
   make
   sudo make install
   ```
2. Создайте файл `fix.cfg` в корне проекта.
3. Соберите программу:
   ```bash
   g++ fix_client.cpp -o fix_client -I/usr/local/include/fix -L/usr/local/lib -lquickfix
   ```

**4. Пояснения кода**:
- `MyMessageCrumbler`: Класс для обработки FIX-сообщений. Метод `onCreate()` отправляет новое ордер-сообщение при установлении соединения.
- `Initiator`: Класс для инициации соединения с сервером.
- `FileStore`: Хранит сообщения в файловой системе.
- Параметры в конфиге:
  - `BeginString`: Версия FIX-протокола.
  - `FileStorePath`: Путь к файлам хранения сообщений.
  - `HeartBtInt`: Интервал отправки сердечных сообщений.

**5. Запуск клиента**:
```bash
./fix_client
```

**6. Требования к серверу**:
- Сервер должен использовать тот же `BeginString` и компоненты (`SenderCompID`, `TargetCompID`), что и в конфиге.
- Для работы TLS потребуется соответствующее сертификаты в конфиге.

**7. Дополнительные классы**:
Для работы с конкретными версиями FIX (например, FIX.4.2) потребуется создать унаследованный класс от `QuickFIX::Application`. В примере используется `mf4::Order`, который предполагает использование `QuickFIX::examples` модуля.

<img src=QuickFIX_ds_2026-06-22_220024.png>


# пример с получением котировок по инструменту EURUSD и записью в PostgreSQL

<img src=Reasoning.png>
<pre>
Мы создадим пример клиента QuickFIX, который будет получать котировки (спот-котировки) по инструменту EUR/USD и записывать их в PostgreSQL.
Для этого нам понадобится:

Конфигурационный файл для подключения к бирже (например, QuickFIX.fx или аналог).
Класс обработчика сообщений, который будет обрабатывать входящие сообщения и парсить из них котировки.
Подключение к базе данных PostgreSQL для записи данных.
Предположим, что мы используем брокера, который поддерживает FIX.4.2 и отправляет котировки в виде сообщений типа "Quote" (например, статусное сообщение 35=D).

Шаги:

Установите необходимые библиотеки: QuickFIX, PostgreSQL (libpqxx).
Создайте конфигурационный файл для QuickFIX (fix.cfg) и для PostgreSQL (например, в коде или отдельно).
Напишем код клиента, который будет:
Инициализировать соединение с брокером (настройки из fix.cfg).
Обрабатывать входящие котировки (сообщения типа 35=D) и извлекать из них необходимые поля.
Сохранять котировки в PostgreSQL.
Примечание: В данном примере мы будем использовать FIX.4.2.

Пример конфигурации fix.cfg для клиента, получающего котировки:

ini

[DEFAULT]
ConnectionType=initiator
SenderCompID=FIXClient
TargetCompID=BrokerID
BeginString=FIX.4.2
SocketConnectHost=представить_хост_брокера
SocketConnectPort=представить_порт_брокера
FileStorePath=./store
HeartBtInt=30
ResetOnLogon=False
StartTime=00:00:00
EndTime=23:59:59

[SESSION]
BeginString=FIX.4.2
SenderCompID=FIXClient
TargetCompID=BrokerID
SocketConnectHost=представить_хост_брокера
SocketConnectPort=представить_порт_брокера
Теперь код программы:

Мы создадим класс обработчика сообщений, который наследуется от QuickFIX::Application.
В этом классе мы перегрузим метод fromAdmin для обработки административных сообщений (например, логин) и fromApp для обработки прикладных (котировки).

Также нам понадобится подключиться к PostgreSQL и создать таблицу для хранения котировок.

Пример таблицы для котировок (EURUSD):

sql

CREATE TABLE quotes (
    id SERIAL PRIMARY KEY,
    symbol VARCHAR(6) NOT NULL,
    bid_price DECIMAL(10,4) NOT NULL,
    bid_size DECIMAL(10,4),
    ask_price DECIMAL(10,4) NOT NULL,
    ask_size DECIMAL(10,4),
    transact_time TIMESTAMP NOT NULL
);
Мы будем использовать библиотеку libpqxx для работы с PostgreSQL.

Установка libpqxx:
Для Ubuntu:
sudo apt-get install libpqxx-dev

Пример кода:
</pre>


Вот пример клиента QuickFIX, который получает котировки по EURUSD и записывает их в PostgreSQL. Для работы этого кода потребуется:

1. Установить QuickFIX и libpqxx
2. Настроить FIX-соединение
3. Создать таблицу в PostgreSQL

**1. Установка зависимостей:**
```bash
# QuickFIX
git clone https://github.com/robert-urner/QuickFIX
cd QuickFIX && mkdir build && cd build
cmake .. && make && sudo make install

# PostgreSQL
sudo apt-get install libpqxx-dev
```

**2. Конфигурационный файл `fix.cfg`:**
```ini
[DEFAULT]
ConnectionType=initiator
SenderCompID=FIXClient
TargetCompID=BrokerID
BeginString=FIX.4.2
SocketConnectHost=your_broker_host
SocketConnectPort=your_broker_port
FileStorePath=./store
HeartBtInt=30
ResetOnLogon=False
StartTime=00:00:00
EndTime=23:59:59
```

**3. Класс приложения `fix_app.cpp`:**
```cpp
#include <QuickFIX/Initiator.h>
#include <QuickFIX/FileStore.h>
#include <QuickFIX/SleepManagerStandard.h>
#include <QuickFIX/MessageCrumbler.h>
#include <QuickFIX/Application.h>
#include <QuickFIX/SessionID.h>
#include <pqxx/pqxx>

using namespace QuickFIX;
using namespace QuickFIX::examples;

// Обработчик для получения и обработки котировок
class QuoteHandler : public MessageCrumbler {
private:
    std::string connectionString = "postgresql://user:password@localhost/dbname";
    pqxx::connection conn(connectionString);
    pqxx::work txn(conn);

public:
    virtual void fromApp(const FIX::Message& msg, const SessionID& id) {
        try {
            const char* data = msg.data();
            FIX::FieldMap fieldMap(data);
            
            if (fieldMap[35] == 'D') { // Проверка типа сообщения (Quote)
                FIX::SecurityID securityID(fieldMap[48]);
                FIX::BidPrice bid(fieldMap[41]);
                FIX::AskPrice ask(fieldMap[42]);
                FIX::TransactTime time(fieldMap[60]);

                // Временное решение для EURUSD
                if (securityID == "EURUSD") {
                    // Подготовка SQL запроса
                    pqxx::insert query("quotes");
                    query.field("symbol").assign("EURUSD");
                    query.field("bid_price").assign(bid);
                    query.field("ask_price").assign(ask);
                    query.field("transact_time").assign(time);
                    query.exec();

                    std::cout << "Quote received: " << bid << " / " << ask << std::endl;
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
};

// Основное приложение
class MyApplication : public Application {
public:
    virtual void onCreate(const SessionID& id) {
        std::cout << "Session created: " << id.toString() << std::endl;
    }

    virtual void onLogon(const SessionID& id) {
        std::cout << "Session logon: " << id.toString() << std::endl;
    }

    virtual void onLogout(const SessionID& id) {
        std::cout << "Session logout: " << id.toString() << std::endl;
    }

    virtual void toApp(const FIX::Message& msg, Fix engine, bool broadcast) {
        // Отправка запросов котировок
        if (msg.getHeader().getField(4328) == 0) { // Tag 4328 - MarketDataRequest
            try {
                const char* data = msg.data();
                FIX::FieldMap fieldMap(data);
                
                FIX::MarketDataRequest request;
                request.getHeader().setField(FIX::MsgType('M')); // Tag 4328 = MarketDataRequest
                
                // Добавляем поля для котировки EURUSD
                request.setField(FIX::MDReqID("12345"));
                request.setField(FIX::SubscriptionRequestType(1));
                request.setField(FIX::MarketDepth(0));
                request.setField(FIX::NoMDEntries(1));

                // Добавляем MDEntry для EURUSD
                FIX::MDEntryGroup group;
                group.setField(FIX::MDEntryType('0')); // Bid
                group.setField(FIX::MDEntryPx(1.1234));
                group.setField(FIX::MDEntrySize(10000));
                group.setField(FIX::MDEntryTime());

                FIX::MDEntryGroup askGroup;
                askGroup.setField(FIX::MDEntryType('1')); // Ask
                askGroup.setField(FIX::MDEntryPx(1.1245));
                askGroup.setField(FIX::MDEntrySize(10000));
                askGroup.setField(FIX::MDEntryTime());

                request.addGroup(group);
                request.addGroup(askGroup);

                engine.send(request, id);
            } catch (const FIX::FieldNotFoundException& e) {
                std::cerr << "Error creating request: " << e.what() << std::endl;
            }
        }
    }

    virtual void fromAdmin(const FIX::Message& msg, const SessionID& id) {
        // Обработка административных сообщений
    }

    virtual void onMessage(const FIX::Message& msg, const SessionID& id) {
        // Обработка других сообщений
    }
};

int main() {
    try {
        // Инициализация приложения
        ApplicationConfig config("fix.cfg");
        MyApplication application;
        MessageCrumblerConfig crumblerConfig;
        MessageStoreFactory storeFactory(&application, FileStoreConfig());
        LogFactory logFactory;
        SleepManager sleepManager(SleepManagerStandard());

        Initiator initiator(storeFactory, logFactory, sleepManager, &application, config);
        initiator.start();
        
        QuoteHandler handler;
        initiator.addCrumbler(handler);

        while (true) {
            SleepManagerStandard::sleep(1000);
        }

        initiator.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

**4. Создание таблицы в PostgreSQL:**
```sql
CREATE TABLE quotes (
    id SERIAL PRIMARY KEY,
    symbol VARCHAR(10) NOT NULL,
    bid_price DECIMAL(10,4),
    ask_price DECIMAL(10,4),
    bid_size BIGINT,
    ask_size BIGINT,
    transact_time TIMESTAMP,
    last_update_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

**5. Инструкция по сборке и запуску:**
1. Создайте файл `fix.cfg` с корректными параметрами вашего брокера
2. Соберите программу:
   ```bash
   g++ main.cpp -o fix_client -I/usr/local/include/fix -I/usr/include/postgresql -L/usr/local/lib -lquickfix -lpqxx
   ```
3. Запустите клиента:
   ```bash
   ./fix_client
   ```

**6. Требования к брокеру:**
- Брокер должен поддерживать FIX.4.2
- Нужно настроить соответствующие поля в конфигурации:
  - SenderCompID/TargetCompID
  - BeginString
  - Порт и хост подключения
- Брокер должен разрешать получение котировок по EURUSD

**7. Особенности работы:**
- Клиент отправляет MarketDataRequest при получении соответствующего запроса
- Обрабатывает ответы типа 'M' (MarketDataIncrementalRefresh)
- Парсит bid/ask цены и записывает в PostgreSQL
- Таблица содержит поля для всех необходимых данных котировки

Для корректной работы необходимо:
1. Заменить значения в `fix.cfg` на реальные параметры вашего брокера
2. Настроить подключение к PostgreSQL в connectionString
3. Убедиться, что брокер поддерживает запрашиваемые тэги данных
4. Добавить необходимые обработчики ошибок

<img src=QuickFIX_ds_2026-06-22_223602.png>


> $ llama-server -m .lmstudio/models/lmstudio-community/DeepSeek-R1-0528-Qwen3-8B-GGUF/DeepSeek-R1-0528-Qwen3-8B-Q8_0.gguf --port 8080
