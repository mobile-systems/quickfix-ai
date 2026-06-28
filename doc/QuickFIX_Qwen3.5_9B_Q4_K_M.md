# Как подключить QuickFIX на C++ пример использования с cfg


Подключение **QuickFIX/N** (версии 1.13.x, которая поддерживает C++17) — это процесс, состоящий из трех основных этапов: установка, настройка конфигурационного файла (`cfg`) и написание кода для создания `Engine`, подключения `SessionFactory` и обработки сообщений.

Ниже приведен полный пример использования с подробными комментариями.

### 1. Подготовка окружения и установка

Предполагается, что вы используете современную среду (Linux/Windows/Mac) с компилятором C++17.

```bash
# Пример установки через vcpkg (рекомендуемый способ для C++)
vcpkg install quickfix-ng
vcpkg integrate install
```

### 2. Настройка конфигурационного файла (CFG)

Создайте файл, например `my.cfg` в папке проекта. Этот файл описывает логику обработки сообщений, группы акцепторов (Acceptors) и группы сессий (Sessions).

**Пример `my.cfg`:**

```ini
[Default]
# Указываем, где лежат заголовочные файлы и библиотеки (если не используется vcpkg с автоматическим поиском)
# HeaderPath = /path/to/quickfix
# LibraryPath = /path/to/quickfix

[DefaultAcceptors]
# Очередь сообщений (обработчик сообщений)
MessageStoreFactory = FileStoreFactory

# Очередь сессий (для нескольких сессий)
SessionFactory = FileLogFactory

[DefaultSessions]
# Список сессий, которые должны быть созданы при старте
Session = 15899

[Session:15899]
# Название сессии (должно совпадать с тем, что в коде в Init)
SessionName = ISL

# Логирование
FileStorePath = log/ISL
FileLogPath = log/ISL

# Адрес FIX-сервера, который мы будем подключаться
Host = 192.168.1.100
Port = 443
SocketType = Tcp

# FIX-параметры
SenderCompID = YOUR_SENDER_ID
TargetCompID = 9876
BeginString = FIX.4.4
StartTime = 00:00:00
EndTime = 00:00:00
HeartBtInt = 30

# Файл с правилом (MessageFactory) - обычно это стандартный FIX44.xml
MessageFactory = FIX44

# Логгер для отладки
LogLevel = Debug
```

> **Важно:** В QuickFIX/N конфигурация не обязательна для запуска, но она крайне полезна для инициализации `SessionFactory` и `MessageStore` извне. В примере ниже мы покажем, как использовать CFG при инициализации.

### 3. Пример кода (C++17)

Создайте файл `main.cpp`. Этот пример создает сессию, подключается к серверу, отправляет тестовое сообщение и обрабатывает входящие.

```cpp
#include <quickfix/engine.h>
#include <quickfix/session.h>
#include <quickfix/sessionsettings.h>
#include <quickfix/message.h>
#include <quickfix/store/filestore.h>
#include <quickfix/store/filelog.h>
#include <quickfix/sessionsettings.h>
#include <quickfix/logger/filelogger.h>
#include <quickfix/stores/filereader.h>
#include <quickfix/stores/filestore.h>
#include <quickfix/field/fixfield.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

// Включаем поддержку C++17 и QuickFIX
// Если используете vcpkg, просто подключите header, если нет - добавьте путь

// --- УТИЛИТА ДЛЯ ОТПРАВКИ ТЕСТОВОГО СООБЩЕНИЯ ---

// Функция для создания и отправки OrderReport (Пример)
void sendOrderReport(QuickFix::Engine* pEngine, const std::string& targetCompID) {
    QuickFix::Message msg;
    msg.setField(QuickFix::Tag(35), "8"); // MsgType = OrderReport
    msg.setField(QuickFix::Tag(49), targetCompID); // SenderCompID
    msg.setField(QuickFix::Tag(56), "9876"); // TargetCompID
    msg.setField(QuickFix::Tag(8), "FIX.4.4"); // BeginString

    // Добавляем поля сообщения (пример)
    msg.setField(QuickFix::Tag(11), "12345"); // ClOrdID
    msg.setField(QuickFix::Tag(55), "BUY"); // Side
    msg.setField(QuickFix::Tag(54), "0"); // OrderQty
    msg.setField(QuickFix::Tag(75), "1"); // ExecType
    msg.setField(QuickFix::Tag(54), "0"); // OrdStatus

    // Устанавливаем время отправки
    msg.setField(QuickFix::Tag(52), "T"); // TimeInForce
    msg.setField(QuickFix::Tag(60), "20231027-120000"); // SubmitTime

    // Устанавливаем целевой сессионный ID
    msg.setField(QuickFix::Tag(43), "YOUR_SENDER_ID"); // TargetCompID (в коде часто задается иначе)

    // Отправляем сообщение
    pEngine->send(msg);
}

// --- ОБРАБОТЧИК СООБЩЕНИЙ (SESSION) ---

class MySessionHandler : public QuickFix::SessionSettings, public QuickFix::SessionFactory {
public:
    // Конструктор не обязателен здесь, так как мы наследуемся от SessionSettings
    // Но нам нужно реализовать SessionFactory для создания сессий
    
    MySessionHandler() : QuickFix::SessionSettings() {
        // Опционально: можно загрузить настройки из CFG здесь
        // QuickFix::SessionSettings settings("my.cfg");
        // this->setSettings(settings); 
    }

    // Функция, которая создает сессию
    QuickFix::SessionFactory::SessionPtr createSession(QuickFix::SessionID sessionId) const override {
        // Создаем сессионный объект
        QuickFix::Session session;
        
        // Настраиваем обработчик сообщений (SessionSettings могут быть загружены здесь)
        // Обычно лучше загружать настройки в глобальном месте или через Engine
        
        return session;
    }
};

// Более простой подход: использование стандартного SessionSettings
class SessionSettings : public QuickFix::SessionSettings {
public:
    SessionSettings(const std::string& settingsFile) 
        : QuickFix::SessionSettings(settingsFile) {}
};

// --- ГЛОБАЛЬНЫЙ ОБРАБОТЧИК ---
// В QuickFIX/N часто используют глобальные обработчики или передают их в сессию
class MyMessageHandler : public QuickFix::SessionHandler {
public:
    // Обработка входящих сообщений
    void toAdmin(QuickFix::Message& msg) override {
        std::cout << "Received: " << msg.toString() << std::endl;
        // Логика обработки
    }

    void toApp(QuickFix::Message& msg) override {
        std::cout << "App message received: " << msg.toString() << std::endl;
        // Логика обработки
    }

    void fromAdmin(QuickFix::Message& msg) override {
        std::cout << "Admin message received: " << msg.toString() << std::endl;
    }

    void fromApp(QuickFix::Message& msg) override {
        std::cout << "App message received: " << msg.toString() << std::endl;
        // Пример отправки ответа
        QuickFix::Message reply;
        reply.setField(QuickFix::Tag(35), "8"); // OrderReport
        reply.setField(QuickFix::Tag(49), "YOUR_SENDER_ID");
        reply.setField(QuickFix::Tag(56), "9876");
        reply.setField(QuickFix::Tag(54), "0");
        reply.setField(QuickFix::Tag(52), "T");
        reply.setField(QuickFix::Tag(60), "20231027-120000");
        reply.setField(QuickFix::Tag(11), msg.getFieldAs<std::string>(QuickFix::Tag(11))); // ClOrdID
        
        // Ответ на сообщение (нужен контекст сессии для send() в fromApp, поэтому лучше использовать Engine)
        // В fromApp у нас нет прямого доступа к Engine, поэтому часто используют глобальную переменную или pattern с Engine
    }
};

int main() {
    try {
        // 1. Загрузка настроек из CFG
        QuickFix::SessionSettings settings("my.cfg");

        // 2. Создание сессий (SessionFactory создает сессии на основе списка в CFG)
        // Мы используем стандартный подход: Engine создает сессии из настроек
        QuickFix::SessionFactory sessionFactory;
        // Если нужно кастомное создание, регистрируем фабрику
        // Но для простого примера с CFG достаточно передать settings в Engine
        
        // 3. Создание Engine
        QuickFix::Engine engine;
        
        // Добавляем сессии (QuickFIX/N автоматически создаст их из списка в CFG)
        // Но мы должны явно указать, что сессии должны быть созданы
        // В QuickFIX/N v1.13+ есть метод createSession из SessionFactory, но проще использовать стандартную инициализацию
        
        // Для использования CFG с Engine, нужно добавить сессии вручную или через SessionStore
        // Самый простой путь для старта:
        engine.addSession("15899", "ISL", settings); // SessionID, SessionName, Settings

        // 4. Настройка обработчика сообщений для сессии
        MyMessageHandler handler;
        // Примечание: Engine хранит ссылку на обработчик, но нужно привязать его правильно
        // В QuickFIX/N обработчик обычно передается через setSessionHandler или через лямбду в новых версиях
        // Для версии 1.13.x мы используем стандартный интерфейс:
        
        // Важно: Engine хранит сессии, но обработчик привязывается к сессии
        // Мы создадим сессию и установим обработчик
        QuickFix::Session session = engine.getSession("15899");
        session.setSessionHandler(handler);

        // 5. Запуск Engine
        // start() запускает акцептор (acceptor) и начинает прием/отправку
        engine.start();

        std::cout << "Engine started. Waiting for messages..." << std::endl;

        // 6. Отправка тестового сообщения
        // Нужно немного времени, чтобы сессия подключилась
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        std::cout << "Sending test OrderReport..." << std::endl;
        sendOrderReport(&engine, "9876");

        // 7. Ожидание завершения или остановка
        // Для бесконечного приложения обычно добавляют флаг -run или флаг остановки
        // Здесь просто ждем 10 секунд и останавливаем
        std::this_thread::sleep_for(std::chrono::seconds(10));
        
        std::cout << "Stopping engine..." << std::endl;
        engine.stop();

    } catch (const QuickFix::Exception& e) {
        std::cerr << "Exception: " << e.getMessage() << std::endl;
        return -1;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
```

### Ключевые моменты при компиляции

Вам понадобятся следующие зависимости (обычно входят в `quickfix-ng`):
- `quickfix`
- `quickfixengine` (или просто `quickfix`, если это все в одном пакете)
- `quickfixsession` (часто встроено в основной)
- `quickfixlogger`
- `quickfixstore`

Пример команды компиляции (с использованием `vcpkg`):
```bash
clang++ -std=c++17 -o fix_example main.cpp
# Если vcpkg интегрирован, он автоматически добавит флаги -I и -L
# Но лучше явно указать:
# clang++ -std=c++17 -I$(vcpkg find quickfix -header-path) -L$(vcpkg find quickfix -lib-path) -lquickfix -o fix_example main.cpp
```

### Как это работает

1.  **CFG (`my.cfg`)**: Опреденяет структуру: какой порт, какой ID отправителя/получателя, где лежат файлы логов. QuickFIX/N читает этот файл при инициализации `SessionSettings`.
2.  **Engine**: Это сердце системы. Он управляет жизненным циклом сессий, очередями сообщений и потоками.
3.  **SessionHandler**: Наследуется от `SessionHandler`. Методы `toAdmin`, `toApp`, `fromAdmin`, `fromApp` вызовятся при получении сообщений.
4.  **Запуск**: `engine.start()` запускает TCP-соединение (или Sockets) на указанном в CFG порту и начинает ожидание входящих сообщений.

### Частые проблемы и решения

*   **Port already in use**: Убедитесь, что порт в `my.cfg` (например, 443) свободен или вы используете другой порт (например, 5009 для тестов).
*   **Session not found**: Убедитесь, что `SessionName` в CFG и имя, используемое при добавлении сессии в Engine, совпадают.
*   **Missing headers**: Если не используете `vcpkg`, убедитесь, что пути к заголовочным файлам QuickFIX указаны в `-I` флаге компилятора.
*   **MessageFactory**: В примере используется `FIX44`. Если вы используете другую версию FIX (например, FIX.4.4.2), убедитесь, что в `MessageFactory` указан правильный XML файл или используйте встроенный `DefaultMessageFactory` с правильным указанием пути к `fix44.xml`.

Этот шаблон дает вам основу для построения полноценного FIX-клиента или сервера на C++.

<!--img src=QuickFIX_2026-06-22_211321.png-->
<img src=img/QuickFIX_20260622_211332.png>

> $ llama-server -m .lmstudio/models/lmstudio-community/Qwen3.5-9B-GGUF/Qwen3.5-9B-Q4_K_M.gguf --port 8080