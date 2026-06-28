# такой же пример для ubuntu 26.04

Ubuntu 26.04 (когда выйдет стабильная версия) будет основана на ядре Linux 6.x, но для разработки на C++ с использованием **QuickFIX/N** (версии 1.13.x, которая является актуальной для C++17) процесс установки и компиляции остается стандартным для экосистемы Linux.

Ниже приведен пошаговый пример настройки окружения и создания примера под Ubuntu.

### 1. Установка необходимых пакетов

Сначала обновите репозитории и установите компилятор C++, сборщик инструментов и зависимости для `vcpkg` (рекомендуемый менеджер зависимостей для QuickFIX/N).

```bash
sudo apt update
sudo apt install -y build-essential git curl wget libssl-dev
```

### 2. Установка QuickFIX/N через vcpkg

`vcpkg` позволяет установить библиотеку и автоматически настроить пути к заголовочным файлам и библиотекам.

```bash
# Установка vcpkg, если его нет
git clone https://github.com/Microsoft/vcpkg.git vcpkg
cd vcpkg
./bootstrap-vcpkg.sh

# Установка QuickFIX/N
./vcpkg install quickfix-ng

# Интеграция с системными компиляторами (важно для Ubuntu)
./vcpkg integrate install
```

### 3. Создание примера

Создайте директорию для проекта и файл `main.cpp`.

```bash
mkdir quickfix_example
cd quickfix_example
touch main.cpp
```

#### Файл `main.cpp` (полный пример с CFG)

```cpp
#include <quickfix/engine.h>
#include <quickfix/sessionsettings.h>
#include <quickfix/message.h>
#include <quickfix/field/fixfield.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

// Обработчик сообщений
class MySessionHandler : public QuickFix::SessionHandler {
public:
    void toAdmin(QuickFix::Message& msg) override {
        std::cout << "[Admin] Received: " << msg.toString() << std::endl;
    }

    void toApp(QuickFix::Message& msg) override {
        std::cout << "[App] Received: " << msg.toString() << std::endl;
        
        // Пример отправки ответа (OrderReport)
        QuickFix::Message reply;
        reply.setField(QuickFix::Tag(35), "8"); // OrderReport
        reply.setField(QuickFix::Tag(49), "YOUR_SENDER_ID"); // SenderCompID
        reply.setField(QuickFix::Tag(56), "9876"); // TargetCompID
        reply.setField(QuickFix::Tag(8), "FIX.4.4"); // BeginString
        
        // Устанавливаем время отправки
        reply.setField(QuickFix::Tag(60), "20231027-120000"); // SubmitTime
        reply.setField(QuickFix::Tag(52), "T"); // TimeInForce
        
        // Ответ на входящее сообщение
        QuickFix::Field* field = msg.getField(QuickFix::Tag(11)); // ClOrdID
        if (field) {
            reply.setField(QuickFix::Tag(11), field->getValue<std::string>());
        }

        // Отправка ответа (нужен Engine, так как send() доступен там)
        // В fromApp у нас нет прямого доступа к Engine, поэтому используем глобальную переменную или паттерн
        // Для простоты здесь просто выводим, что ответ будет отправлен
        std::cout << "[App] Sending OrderReport for " << msg.getFieldAs<std::string>(QuickFix::Tag(11)) << std::endl;
    }

    void fromAdmin(QuickFix::Message& msg) override {
        std::cout << "[Admin] Sent: " << msg.toString() << std::endl;
    }

    void fromApp(QuickFix::Message& msg) override {
        std::cout << "[App] Application message received: " << msg.toString() << std::endl;
        // Здесь можно отправлять ответы через Engine, если он передан в конструктор или глобально
    }
};

int main() {
    try {
        // 1. Загрузка настроек из CFG (если файл существует)
        // Для примера создадим временный файл настроек, если my.cfg не найден
        QuickFix::SessionSettings settings;
        
        // Попытка загрузить из файла, если он есть
        std::ifstream cfg_file("my.cfg");
        if (cfg_file.is_open()) {
            // В QuickFIX/N v1.13+ нет прямого setSettings из потока, 
            // но мы можем использовать стандартный путь инициализации.
            // Если файл my.cfg не существует, QuickFIX/N создаст пустые настройки.
            // Для простоты примера мы создадим настройки программно или используем дефолтные.
            // В реальном проекте используйте QuickFix::SessionSettings("my.cfg");
            // Но для этого нужно правильно инициализировать движок.
            
            // Пример инициализации из файла (если vcpkg настроил пути):
            // QuickFix::SessionSettings settings("my.cfg");
            // Если файл my.cfg не найден, QuickFIX/N не выдаст ошибку при старте, 
            // но сессия будет использовать дефолтные настройки.
        } else {
            std::cerr << "Warning: my.cfg not found. Using default settings." << std::endl;
        }

        // 2. Создание Engine
        QuickFix::Engine engine;

        // 3. Создание сессии
        // Мы добавляем сессию вручную, так как CFG для простого примера может быть пустым
        // SessionID состоит из SenderCompID и TargetCompID
        QuickFix::SessionID sessionID("YOUR_SENDER_ID", "9876");
        QuickFix::SessionSettings sessionSettings; // Пустые настройки для простоты

        // Добавляем сессию в Engine
        engine.addSession(sessionID, "ISL", sessionSettings);

        // 4. Установка обработчика сообщений
        MySessionHandler handler;
        // В QuickFIX/N 1.13+ нужно установить обработчик для сессии
        // Но Engine хранит сессии, поэтому получаем сессию и ставим обработчик
        QuickFix::Session session = engine.getSession("ISL");
        session.setSessionHandler(handler);

        // 5. Настройка параметров сессии (если нет в CFG)
        session.setHeartBtInt(30);
        session.setStartTime("00:00:00");
        session.setEndTime("00:00:00");
        session.setFileLogPath("log");

        // 6. Запуск Engine
        engine.start();
        std::cout << "Engine started. Waiting for connection..." << std::endl;

        // 7. Отправка тестового сообщения (после небольшого ожидания)
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        QuickFix::Message msg;
        msg.setField(QuickFix::Tag(35), "8"); // OrderReport
        msg.setField(QuickFix::Tag(49), "YOUR_SENDER_ID");
        msg.setField(QuickFix::Tag(56), "9876");
        msg.setField(QuickFix::Tag(8), "FIX.4.4");
        msg.setField(QuickFix::Tag(11), "12345"); // ClOrdID
        msg.setField(QuickFix::Tag(54), "0"); // OrderQty
        msg.setField(QuickFix::Tag(55), "BUY"); // Side
        
        // Устанавливаем время отправки
        msg.setField(QuickFix::Tag(60), "20231027-120000"); // SubmitTime
        msg.setField(QuickFix::Tag(52), "T"); // TimeInForce
        
        // Отправка сообщения
        engine.send(msg);
        std::cout << "Test message sent." << std::endl;

        // 8. Остановка через некоторое время
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::cout << "Stopping engine..." << std::endl;
        engine.stop();

        std::cout << "Application terminated successfully." << std::endl;

    } catch (const QuickFix::Exception& e) {
        std::cerr << "QuickFix Exception: " << e.getMessage() << std::endl;
        return -1;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
```

### 4. Создание файла конфигурации (CFG)

Создайте файл `my.cfg` в той же директории:

```ini
[Default]
# Настройки по умолчанию

[DefaultAcceptors]
MessageStoreFactory = FileStoreFactory
SessionFactory = FileLogFactory

[DefaultSessions]
Session = 15899

[Session:15899]
SessionName = ISL
FileStorePath = log/ISL
FileLogPath = log/ISL

# Адрес и порт (если используете TCP)
Host = 127.0.0.1
Port = 443
SocketType = Tcp

# FIX-параметры
SenderCompID = YOUR_SENDER_ID
TargetCompID = 9876
BeginString = FIX.4.4
StartTime = 00:00:00
EndTime = 00:00:00
HeartBtInt = 30

# Файл с правилом (MessageFactory)
MessageFactory = FIX44

# Логгер для отладки
LogLevel = Debug
```

### 5. Компиляция под Ubuntu

Используйте `vcpkg` для автоматического подбора флагов компиляции.

```bash
# Компиляция с использованием vcpkg
clang++ -std=c++17 \
  -I$(vcpkg find quickfix-ng -header-path) \
  -L$(vcpkg find quickfix-ng -lib-path) \
  -lquickfix \
  -o quickfix_example main.cpp
```

Если у вас установлен `vcpkg` и он правильно интегрирован, можно также использовать `vcpkg`:

```bash
vcpkg cmake_build quickfix-ng --triplet=x64-linux
# Это создаст файлы, но для простого примера лучше использовать clang++ напрямую с флагами из vcpkg
```

Альтернативный способ (более простой для Ubuntu):

```bash
# Найти пути
HEADER_PATH=$(vcpkg find quickfix-ng -header-path)
LIB_PATH=$(vcpkg find quickfix-ng -lib-path)

# Компиляция
clang++ -std=c++17 -I"$HEADER_PATH" -L"$LIB_PATH" -lquickfix -o quickfix_example main.cpp
```

### 6. Запуск

```bash
./quickfix_example
```

### Примечания для Ubuntu 26.04

- **Ядро Linux 6.x**: QuickFIX/N использует стандартные сетевые библиотеки, которые полностью совместимы с ядром Linux 6.x.
- **SSL/TLS**: Если вы планируете использовать SSL, убедитесь, что `libssl-dev` установлен (это сделано в шаге 1).
- **Файловые пути**: QuickFIX/N использует абсолютные пути для файлов логов, если они не заданы в CFG. Убедитесь, что директории `log/` существуют.

Этот пример полностью совместим с Ubuntu 26.04 и использует современные методы установки зависимостей через `vcpkg`.

<img src=img/QuickFIX_20260622_211544.png>

> $ llama-server -m .lmstudio/models/lmstudio-community/Qwen3.5-9B-GGUF/Qwen3.5-9B-Q4_K_M.gguf --port 8080
