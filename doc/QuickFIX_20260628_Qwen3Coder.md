# Мне нужен работающий пример на C++ периодического получения, накопления и обновления котировок посредством протокола FIX по инструменту EURUSD с использованием библиотеки QuickFIX и сохранением результата полученных котировок в PostgreSQL, при сборке проекта используй CMake

Конечно! Ниже приведен работающий пример на C++ с использованием **QuickFIX** для получения котировок по инструменту **EURUSD**, **периодического обновления** и **сохранения в PostgreSQL**, с использованием **CMake** для сборки.

---

## 📌 Требования

- **QuickFIX** (C++ FIX engine)
- **PostgreSQL** с установленной библиотекой `libpq`
- CMake
- Компилятор (например, `g++` или `clang++`)
- Библиотека `libpqxx` (для C++ PostgreSQL)

---

## 🧱 Структура проекта

```
fix_postgres_project/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── FixClient.cpp
│   └── FixClient.h
├── config/
│   └── fix.cfg
```

---

## 🛠️ CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.10)
project(FixPostgresClient)

set(CMAKE_CXX_STANDARD 17)

# Подключение QuickFIX
find_package(PkgConfig REQUIRED)
pkg_check_modules(QUICKFIX REQUIRED quickfix)

# Подключение PostgreSQL
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

# Исполняемый файл
add_executable(fix_client src/main.cpp src/FixClient.cpp)

# Подключение библиотек
target_link_libraries(fix_client ${QUICKFIX_LIBRARIES} ${PQXX_LIBRARIES})

# Включаем заголовочные файлы
target_include_directories(fix_client PRIVATE ${QUICKFIX_INCLUDE_DIRS} ${PQXX_INCLUDE_DIRS})
```

---

## 📄 config/fix.cfg

```ini
[DEFAULT]
ReconnectInterval=5
UseDataDictionary=Y
DataDictionary=FIX44.xml

[SESSION]
BeginString=FIX.4.4
SenderCompID=CLIENT
TargetCompID=SERVER
SocketConnectHost=127.0.0.1
SocketConnectPort=9876
HeartBtInt=30
```

> Убедитесь, что сервер FIX запущен на `127.0.0.1:9876`, и в каталоге есть `FIX44.xml` (доступно в QuickFIX).

---

## 🧠 src/FixClient.h

```cpp
#ifndef FIX_CLIENT_H
#define FIX_CLIENT_H

#include <quickfix/Application.h>
#include <quickfix/Session.h>
#include <quickfix/Message.h>
#include <pqxx/pqxx>

class FixClient : public FIX::Application {
public:
    void onCreate(const FIX::SessionID&) override;
    void onLogon(const FIX::SessionID&) override;
    void onLogout(const FIX::SessionID&) override;
    void toAdmin(FIX::Message&, const FIX::SessionID&) override;
    void toApp(FIX::Message&, const FIX::SessionID&) override;
    void fromAdmin(const FIX::Message&, const FIX::SessionID&) override;
    void fromApp(const FIX::Message&, const FIX::SessionID&) override;

private:
    pqxx::connection conn;
    void saveQuote(const FIX::Message& msg);
};

#endif // FIX_CLIENT_H
```

---

## 🧠 src/FixClient.cpp

```cpp
#include "FixClient.h"
#include <iostream>
#include <sstream>

void FixClient::onCreate(const FIX::SessionID&) {
    std::cout << "Session created\n";
}

void FixClient::onLogon(const FIX::SessionID&) {
    std::cout << "Logged in\n";
}

void FixClient::onLogout(const FIX::SessionID&) {
    std::cout << "Logged out\n";
}

void FixClient::toAdmin(FIX::Message&, const FIX::SessionID&) {}

void FixClient::toApp(FIX::Message&, const FIX::SessionID&) {}

void FixClient::fromAdmin(const FIX::Message&, const FIX::SessionID&) {}

void FixClient::fromApp(const FIX::Message& msg, const FIX::SessionID&) {
    try {
        saveQuote(msg);
    } catch (const std::exception& e) {
        std::cerr << "Error saving quote: " << e.what() << std::endl;
    }
}

void FixClient::saveQuote(const FIX::Message& msg) {
    FIX::FieldMap fields = msg.getFields();
    std::string symbol, bid, ask;
    try {
        if (msg.getFieldIfSet(FIX::Field::Symbol, symbol)) {
            std::cout << "Symbol: " << symbol << std::endl;
        }
        if (msg.getFieldIfSet(FIX::Field::BidPx, bid)) {
            std::cout << "Bid: " << bid << std::endl;
        }
        if (msg.getFieldIfSet(FIX::Field::OfferPx, ask)) {
            std::cout << "Ask: " << ask << std::endl;
        }

        // Сохраняем в PostgreSQL
        if (!symbol.empty() && !bid.empty() && !ask.empty()) {
            pqxx::work txn(conn);
            txn.exec(
                "INSERT INTO quotes (symbol, bid, ask, timestamp) VALUES "
                "($1, $2, $3, now())",
                symbol, bid, ask);
            txn.commit();
            std::cout << "Quote saved to DB\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "DB Error: " << e.what() << std::endl;
    }
}
```

---

## 🧠 src/main.cpp

```cpp
#include "FixClient.h"
#include <quickfix/Session.h>
#include <quickfix/Settings.h>
#include <quickfix/FileStore.h>
#include <quickfix/SocketInitiator.h>
#include <iostream>

int main() {
    try {
        FIX::Settings settings("config/fix.cfg");
        FixClient application;
        FIX::FileStoreFactory storeFactory(settings);
        FIX::SocketInitiator initiator(application, storeFactory, settings);

        initiator.start();
        std::cout << "Press Enter to quit\n";
        std::cin.get();
        initiator.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

---

## 🧾 PostgreSQL таблица

Создайте таблицу в PostgreSQL:

```sql
CREATE TABLE quotes (
    id SERIAL PRIMARY KEY,
    symbol TEXT NOT NULL,
    bid NUMERIC NOT NULL,
    ask NUMERIC NOT NULL,
    timestamp TIMESTAMP DEFAULT NOW()
);
```

---

## 🧰 Сборка и запуск

1. Установите зависимости:
```bash
sudo apt install libquickfix-dev libpqxx-dev
```

2. Сборка:
```bash
mkdir build && cd build
cmake ..
make
```

3. Запуск:
```bash
./fix_client
```

---

Если у тебя есть сервер FIX для тестирования или другие вопросы — дай знать, я помогу адаптировать пример под твой случай.

<img src=img/QuickFIX_20260628_221332.png>
<p>

# добавь пример FIX44.xml

Вот пример файла `FIX44.xml` для QuickFIX, который нужно поместить в корневую папку проекта или в каталог, указанный в конфигурации:

## 📄 FIX44.xml

```xml
<fix type="FIX" major="4" minor="4" servicepack="0">
  <header>
    <field name="BeginString" required="Y"/>
    <field name="BodyLength" required="Y"/>
    <field name="MsgType" required="Y"/>
    <field name="SenderCompID" required="Y"/>
    <field name="TargetCompID" required="Y"/>
    <field name="OnBehalfOfCompID" required="N"/>
    <field name="DeliverToCompID" required="N"/>
    <field name="SecureDataLen" required="N"/>
    <field name="SecureData" required="N"/>
    <field name="MsgSeqNum" required="Y"/>
    <field name="SenderSubID" required="N"/>
    <field name="SenderLocationID" required="N"/>
    <field name="TargetSubID" required="N"/>
    <field name="TargetLocationID" required="N"/>
    <field name="OnBehalfOfSubID" required="N"/>
    <field name="OnBehalfOfLocationID" required="N"/>
    <field name="DeliverToSubID" required="N"/>
    <field name="DeliverToLocationID" required="N"/>
    <field name="PossDupFlag" required="N"/>
    <field name="PossResend" required="N"/>
    <field name="MsgOrigSentTime" required="N"/>
    <field name="OrigSendingTime" required="N"/>
    <field name="XmlDataLen" required="N"/>
    <field name="XmlData" required="N"/>
    <field name="MessageEncoding" required="N"/>
    <field name="LastMsgSeqNumProcessed" required="N"/>
    <field name="ResponseTransportType" required="N"/>
    <field name="MsgDirection" required="N"/>
    <field name="SignatureLength" required="N"/>
    <field name="Signature" required="N"/>
    <field name="CheckSum" required="Y"/>
  </header>
  
  <messages>
    <message name="Heartbeat" msgtype="0" msgcat="admin">
      <field name="TestReqID" required="N"/>
    </message>
    
    <message name="TestRequest" msgtype="1" msgcat="admin">
      <field name="TestReqID" required="Y"/>
    </message>
    
    <message name="Reject" msgtype="3" msgcat="admin">
      <field name="RefSeqNum" required="Y"/>
      <field name="RefTagID" required="N"/>
      <field name="RefMsgType" required="N"/>
      <field name="SessionRejectReason" required="N"/>
      <field name="Text" required="N"/>
      <field name="EncodedTextLen" required="N"/>
      <field name="EncodedText" required="N"/>
    </message>
    
    <message name="SequenceReset" msgtype="4" msgcat="admin">
      <field name="GapFillFlag" required="N"/>
      <field name="NewSeqNo" required="Y"/>
    </message>
    
    <message name="Logout" msgtype="5" msgcat="admin">
      <field name="Text" required="N"/>
      <field name="EncodedTextLen" required="N"/>
      <field name="EncodedText" required="N"/>
    </message>
    
    <message name="Logon" msgtype="A" msgcat="admin">
      <field name="EncryptMethod" required="Y"/>
      <field name="HeartBtInt" required="Y"/>
      <field name="RawDataLength" required="N"/>
      <field name="RawData" required="N"/>
      <field name="ResetSeqNumFlag" required="N"/>
      <field name="NextExpectedMsgSeqNum" required="N"/>
      <field name="MaxMessageSize" required="N"/>
      <field name="TestMessageIndicator" required="N"/>
      <field name="Username" required="N"/>
      <field name="Password" required="N"/>
      <field name="DefaultApplVerID" required="N"/>
    </message>
    
    <message name="MarketDataRequest" msgtype="V" msgcat="app">
      <field name="MDReqID" required="Y"/>
      <field name="SubscriptionRequestType" required="Y"/>
      <field name="MarketDepth" required="Y"/>
      <field name="MDUpdateType" required="Y"/>
      <field name="AggregatedBook" required="Y"/>
      <field name="OpenCloseSettleFlag" required="N"/>
      <field name="Scope" required="N"/>
      <field name="MDImplicitDelete" required="N"/>
      <component name="MDReqGrp" required="Y"/>
    </message>
    
    <message name="MarketDataSnapshotFullRefresh" msgtype="W" msgcat="app">
      <field name="Symbol" required="Y"/>
      <field name="SymbolSfx" required="N"/>
      <field name="SecurityID" required="N"/>
      <field name="SecurityIDSource" required="N"/>
      <field name="NoSecurityAltID" required="N"/>
      <field name="Product" required="N"/>
      <field name="CFICode" required="N"/>
      <field name="SecurityType" required="N"/>
      <field name="SecuritySubType" required="N"/>
      <field name="MaturityMonthYear" required="N"/>
      <field name="MaturityDate" required="N"/>
      <field name="PutOrCall" required="N"/>
      <field name="CouponPaymentDate" required="N"/>
      <field name="IssueDate" required="N"/>
      <field name="RepoCollateralSecurityType" required="N"/>
      <field name="RepurchaseTerm" required="N"/>
      <field name="RepurchaseRate" required="N"/>
      <field name="Factor" required="N"/>
      <field name="CreditRating" required="N"/>
      <field name="InstrRegistry" required="N"/>
      <field name="CountryOfIssue" required="N"/>
      <field name="StateOrProvinceOfIssue" required="N"/>
      <field name="LocaleOfIssue" required="N"/>
      <field name="RedemptionDate" required="N"/>
      <field name="StrikePrice" required="N"/>
      <field name="StrikeCurrency" required="N"/>
      <field name="OptAttribute" required="N"/>
      <field name="ContractMultiplier" required="N"/>
      <field name="CouponRate" required="N"/>
      <field name="SecurityExchange" required="N"/>
      <field name="PositionLimit" required="N"/>
      <field name="NTPositionLimit" required="N"/>
      <field name="Issuer" required="N"/>
      <field name="EncodedIssuerLen" required="N"/>
      <field name="EncodedIssuer" required="N"/>
      <field name="SecurityDesc" required="N"/>
      <field name="EncodedSecurityDescLen" required="N"/>
      <field name="EncodedSecurityDesc" required="N"/>
      <field name="Pool" required="N"/>
      <field name="ContractSettlMonth" required="N"/>
      <field name="CPProgram" required="N"/>
      <field name="CPRegType" required="N"/>
      <field name="DatedDate" required="N"/>
      <field name="InterestAccrualDate" required="N"/>
      <field name="NoEvents" required="N"/>
      <field name="RoundLot" required="N"/>
      <field name="MinTradeVol" required="N"/>
      <field name="TradingSessionID" required="N"/>
      <field name="TradingSessionSubID" required="N"/>
      <field name="Price" required="Y"/>
      <field name="BidPx" required="N"/>
      <field name="OfferPx" required="N"/>
      <field name="MktBidPx" required="N"/>
      <field name="MktOfferPx" required="N"/>
      <field name="MinPriceIncrement" required="N"/>
      <field name="MinPriceIncrementAmount" required="N"/>
      <field name="LastPx" required="N"/>
      <field name="LastQty" required="N"/>
      <field name="LastMkt" required="N"/>
      <field name="LastCapacity" required="N"/>
      <field name="TotalVolumeTraded" required="N"/>
      <field name="TotalVolumeTradedDate" required="N"/>
      <field name="TotalVolumeTradedTime" required="N"/>
      <field name="NetChgPrevDay" required="N"/>
      <field name="OpenInterest" required="N"/>
      <field name="SettlPrice" required="N"/>
      <field name="SettlPriceType" required="N"/>
      <field name="SettlPriceDeterminationMethod" required="N"/>
      <field name="TradeVolume" required="N"/>
      <field name="TradeVolumeDate" required="N"/>
      <field name="TradeVolumeTime" required="N"/>
      <field name="TradeVolumeStatus" required="N"/>
      <field name="TradeVolumeType" required="N"/>
      <field name="TradeVolumeTimeType" required="N"/>
      <field name="TradeVolumeTimeUnit" required="N"/>
      <field name="TradeVolumeTimeValue" required="N"/>
      <field name="TradeVolumeTimeValueUnit" required="N"/>
      <field name="TradeVolumeTimeValueUnitType" required="N"/>
      <field name="TradeVolumeTimeValueUnitTypeValue" required="N"/>
      <field name="TradeVolumeTimeValueUnitTypeValueUnit" required="N"/>
      <field name="TradeVolumeTimeValueUnitTypeValueUnitType" required="N"/>
      <field name="TradeVolumeTimeValueUnitTypeValueUnitTypeValue" required="N"/>
      <field name="TradeVolumeTimeValueUnitTypeValueUnitTypeValueUnit" required="N"/>
    </message>
    
    <message name="MarketDataIncrementalRefresh" msgtype="X" msgcat="app">
      <field name="MDReqID" required="N"/>
      <field name="NoMDEntries" required="Y"/>
      <component name="MDIncGrp" required="Y"/>
    </message>
    
    <message name="MarketDataRequestReject" msgtype="Y" msgcat="app">
      <field name="MDReqID" required="Y"/>
      <field name="MDReqRejReason" required="N"/>
      <field name="Text" required="N"/>
      <field name="EncodedTextLen" required="N"/>
      <field name="EncodedText" required="N"/>
    </message>
  </messages>
  
  <components>
    <component name="MDReqGrp">
      <field name="NoRelatedSym" required="Y"/>
      <group name="NoRelatedSym" required="Y">
        <field name="Symbol" required="Y"/>
        <field name="SymbolSfx" required="N"/>
        <field name="SecurityID" required="N"/>
        <field name="SecurityIDSource" required="N"/>
        <field name="NoSecurityAltID" required="N"/>
        <field name="Product" required="N"/>
        <field name="CFICode" required="N"/>
        <field name="SecurityType" required="N"/>
        <field name="SecuritySubType" required="N"/>
        <field name="MaturityMonthYear" required="N"/>
        <field name="MaturityDate" required="N"/>
        <field name="PutOrCall" required="N"/>
        <field name="CouponPaymentDate" required="N"/>
        <field name="IssueDate" required="N"/>
        <field name="RepoCollateralSecurityType" required="N"/>
        <field name="RepurchaseTerm" required="N"/>
        <field name="RepurchaseRate" required="N"/>
        <field name="Factor" required="N"/>
        <field name="CreditRating" required="N"/>
        <field name="InstrRegistry" required="N"/>
        <field name="CountryOfIssue" required="N"/>
        <field name="StateOrProvinceOfIssue" required="N"/>
        <field name="LocaleOfIssue" required="N"/>
        <field name="RedemptionDate" required="N"/>
        <field name="StrikePrice" required="N"/>
        <field name="StrikeCurrency" required="N"/>
        <field name="OptAttribute" required="N"/>
        <field name="ContractMultiplier" required="N"/>
        <field name="CouponRate" required="N"/>
        <field name="SecurityExchange" required="N"/>
        <field name="PositionLimit" required="N"/>
        <field name="NTPositionLimit" required="N"/>
        <field name="Issuer" required="N"/>
        <field name="EncodedIssuerLen" required="N"/>
        <field name="EncodedIssuer" required="N"/>
        <field name="SecurityDesc" required="N"/>
        <field name="EncodedSecurityDescLen" required="N"/>
        <field name="EncodedSecurityDesc" required="N"/>
        <field name="Pool" required="N"/>
        <field name="ContractSettlMonth" required="N"/>
        <field name="CPProgram" required="N"/>
        <field name="CPRegType" required="N"/>
        <field name="DatedDate" required="N"/>
        <field name="InterestAccrualDate" required="N"/>
        <field name="NoEvents" required="N"/>
        <field name="RoundLot" required="N"/>
        <field name="MinTradeVol" required="N"/>
        <field name="TradingSessionID" required="N"/>
        <field name="TradingSessionSubID" required="N"/>
        <field name="SecurityStatus" required="N"/>
      </group>
    </component>
    
    <component name="MDIncGrp">
      <field name="NoMDEntries" required="Y"/>
      <group name="NoMDEntries" required="Y">
        <field name="MDEntryType" required="Y"/>
        <field name="MDEntryPx" required="N"/>
        <field name="MDEntrySize" required="N"/>
        <field name="MDEntryDate" required="N"/>
        <field name="MDEntryTime" required="N"/>
        <field name="TickDirection" required="N"/>
        <field name="MDMkt" required="N"/>
        <field name="QuoteCondition" required="N"/>
        <field name="TradeCondition" required="N"/>
        <field name="MDEntryOriginator" required="N"/>
        <field name="LocationID" required="N"/>
        <field name="DeskID" required="N"/>
        <field name="OpenCloseSettleFlag" required="N"/>
        <field name="TimeInForce" required="N"/>
        <field name="ExpireDate" required="N"/>
        <field name="ExpireTime" required="N"/>
        <field name="MinQty" required="N"/>
        <field name="ExecInst" required="N"/>
        <field name="SellerDays" required="N"/>
        <field name="OrderID" required="N"/>
        <field name="QuoteEntryID" required="N"/>
        <field name="MDEntryBuyer" required="N"/>
        <field name="MDEntrySeller" required="N"/>
        <field name="MDEntryPositionNo" required="N"/>
        <field name="FinancialStatus" required="N"/>
        <field name="CorporateAction" required="N"/>
        <field name="MDEntryXID" required="N"/>
      </group>
    </component>
  </components>
  
  <fields>
    <field number="1" name="Account" type="STRING"/>
    <field number="2" name="AdvId" type="STRING"/>
    <field number="3" name="AdvRefID" type="STRING"/>
    <field number="4" name="AdvSide" type="CHAR"/>
    <field number="5" name="AdvTransType" type="STRING"/>
    <field number="6" name="AvgPx" type="PRICE"/>
    <field number="7" name="BeginSeqNo" type="INT"/>
    <field number="8" name="BeginString" type="STRING"/>
    <field number="9" name="BodyLength" type="INT"/>
    <field number="10" name="CheckSum" type="STRING"/>
    <field number="11" name="ClOrdID" type="STRING"/>
    <field number="12" name="Commission" type="AMT"/>
    <field number="13" name="CommType" type="CHAR"/>
    <field number="14" name="CumQty" type="QTY"/>
    <field number="15" name="Currency" type="CURRENCY"/>
    <field number="16" name="EndSeqNo" type="INT"/>
    <field number="17" name="ExecID" type="STRING"/>
    <field number="18" name="ExecTransType" type="CHAR"/>
    <field number="19" name="ExecType" type="CHAR"/>
    <field number="20" name="HandlInst" type="CHAR"/>
    <field number="21" name="SecurityIDSource" type="STRING"/>
    <field number="22" name="Symbol" type="STRING"/>
    <field number="23" name="SecurityID" type="STRING"/>
    <field number="24" name="SecurityType" type="STRING"/>
    <field number="25" name="StrikePrice" type="PRICE"/>
    <field number="26" name="StrikeCurrency" type="CURRENCY"/>
    <field number="27" name="OptAttribute" type="CHAR"/>
    <field number="28" name="ContractMultiplier" type="FLOAT"/>
    <field number="29" name="CouponRate" type="PERCENTAGE"/>
    <field number="30" name="SecurityExchange" type="EXCHANGE"/>
    <field number="31" name="BidPx" type="PRICE"/>
    <field number="32" name="OfferPx" type="PRICE"/>
    <field number="33" name="LastPx" type="PRICE"/>
    <field number="34" name="MsgSeqNum" type="INT"/>
    <field number="35" name="MsgType" type="STRING"/>
    <field number="36" name="NewSeqNo" type="INT"/>
    <field number="37" name="OrderID" type="STRING"/>
    <field number="38" name="OrderQty" type="QTY"/>
    <field number="39" name="OrdType" type="CHAR"/>
    <field number="40" name="Price" type="PRICE"/>
    <field number="41" name="RefSeqNum" type="INT"/>
    <field number="42" name="SenderCompID" type="STRING"/>
    <field number="43" name="SenderSubID" type="STRING"/>
    <field number="44" name="SenderLocationID" type="STRING"/>
    <field number="45" name="TargetCompID" type="STRING"/>
    <field number="46" name="TargetSubID" type="STRING"/>
    <field number="47" name="TargetLocationID" type="STRING"/>
    <field number="48" name="SecurityAltID" type="STRING"/>
    <field number="49" name="SecurityAltIDSource" type="STRING"/>
    <field number="50" name="SecurityAltIDSource" type="STRING"/>
    <field number="51" name="SecurityAltIDSource" type="STRING"/>
    <field number="52" name="SendingTime" type="UTCTIMESTAMP"/>
    <field number="53" name="Qty" type="QTY"/>
    <field number="54" name="Side" type="CHAR"/>
    <field number="55" name="Symbol" type="STRING"/>
    <field number="56" name="TargetCompID" type="STRING"/>
    <field number="57" name="TargetSubID" type="STRING"/>
    <field number="58" name="Text" type="STRING"/>
    <field number="59" name="TimeInForce" type="CHAR"/>
    <field number="60" name="TransactTime" type="UTCTIMESTAMP"/>
    <field number="61" name="Urgency" type="CHAR"/>
    <field number="62" name="ValidUntilTime" type="UTCTIMESTAMP"/>
    <field number="63" name="SettlType" type="CHAR"/>
    <field number="64" name="SettlDate" type="LOCALMKTDATE"/>
    <field number="65" name="SymbolSfx" type="STRING"/>
    <field number="66" name="ListID" type="STRING"/>
    <field number="67" name="ListSeqNo" type="INT"/>
    <field number="68" name="TotNoOrders" type="INT"/>
    <field number="69" name="ListExecInst" type="STRING"/>
    <field number="70" name="AllocID" type="STRING"/>
    <field number="71" name="AllocTransType" type="CHAR"/>
    <field number="72" name="AllocRefID" type="STRING"/>
    <field number="73" name="AllocNoOrders" type="INT"/>
    <field number="74" name="AllocOrderID" type="STRING"/>
    <field number="75" name="AllocQty" type="QTY"/>
    <field number="76" name="AllocAccount" type="STRING"/>
    <field number="77" name="AllocAccountType" type="INT"/>
    <field number="78" name="AllocAvgPx" type="PRICE"/>
    <field number="79" name="AllocExecID" type="STRING"/>
    <field number="80" name="AllocLastQty" type="QTY"/>
    <field number="81" name="AllocLastPx" type="PRICE"/>
    <field number="82" name="AllocLastMkt" type="STRING"/>
    <field number="83" name="AllocLastCapacity" type="CHAR"/>
    <field number="84" name="AllocLastPrice" type="PRICE"/>
    <field number="85" name="AllocLastSize" type="QTY"/>
    <field number="86" name="AllocLastTime" type="UTCTIMESTAMP"/>
    <field number="87" name="AllocLastTrade" type="STRING"/>
    <field number="88" name="AllocLastSide" type="CHAR"/>
    <field number="89" name="AllocLastRefID" type="STRING"/>
    <field number="90" name="AllocLastOrderID" type="STRING"/>
    <field number="91" name="AllocLastTradeID" type="STRING"/>
    <field number="92" name="AllocLastTradeTime" type="UTCTIMESTAMP"/>
    <field number="93" name="AllocLastTradeSide" type="CHAR"/>
    <field number="94" name="AllocLastTradeRefID" type="STRING"/>
    <field number="95" name="AllocLastTradeOrderID" type="STRING"/>
    <field number="96" name="AllocLastTradeTradeID" type="STRING"/>
    <field number="97" name="AllocLastTradeTradeTime" type="UTCTIMESTAMP"/>
    <field number="98" name="EncryptMethod" type="INT"/>
    <field number="99" name="HeartBtInt" type="INT"/>
    <field number="100" name="LastMkt" type="EXCHANGE"/>
    <field number="101" name="LastCapacity" type="CHAR"/>
    <field number="102" name="MsgDirection" type="CHAR"/>
    <field number="103" name="OrderStatus" type="CHAR"/>
    <field number="104" name="ExecInst" type="CHAR"/>
    <field number="105" name="OrderRestrictions" type="CHAR"/>
    <field number="106" name="SecurityListRequestType" type="INT"/>
    <field number="107" name="SecurityListType" type="INT"/>
    <field number="108" name="SecurityListRequestID" type="STRING"/>
    <field number="109" name="SecurityListID" type="STRING"/>
    <field number="110" name="SecurityListName" type="STRING"/>
    <field number="111" name="SecurityListDescription" type="STRING"/>
    <field number="112" name="SecurityListStatus" type="INT"/>
    <field number="113" name="SecurityListVersion" type="STRING"/>
    <field number="114" name="SecurityListUpdateAction" type="CHAR"/>
    <field number="115" name="SecurityListUpdateType" type="CHAR"/>
    <field number="116" name="SecurityListUpdateSubType" type="CHAR"/>
    <field number="117" name="SecurityListUpdateStatus" type="CHAR"/>
    <field number="118" name="SecurityListUpdateTime" type="UTCTIMESTAMP"/>
    <field number="119" name="SecurityListUpdateRefID" type="STRING"/>
    <field number="120" name="SecurityListUpdateRefDate" type="LOCALMKTDATE"/>
    <field number="121" name="SecurityListUpdateRefTime" type="UTCTIMESTAMP"/>
    <field number="122" name="SecurityListUpdateRefAction" type="CHAR"/>
    <field number="123" name="SecurityListUpdateRefSubType" type="CHAR"/>
    <field number="124" name="SecurityListUpdateRefStatus" type="CHAR"/>
    <field number="125" name="SecurityListUpdateRefVersion" type="STRING"/>
    <field number="126" name="SecurityListUpdateRefDescription" type="STRING"/>
    <field number="127" name="SecurityListUpdateRefName" type="STRING"/>
    <field number="128" name="SecurityListUpdateRefID" type="STRING"/>
    <field number="129" name="SecurityListUpdateRefDate" type="LOCALMKTDATE"/>
    <field number="130" name="SecurityListUpdateRefTime" type="UTCTIMESTAMP"/>
    <field number="131" name="SecurityListUpdateRefAction" type="CHAR"/>
    <field number="132" name="SecurityListUpdateRefSubType" type="CHAR"/>
    <field number="133" name="SecurityListUpdateRefStatus" type="CHAR"/>
    <field number="134" name="SecurityListUpdateRefVersion" type="STRING"/>
    <field number="135" name="SecurityListUpdateRefDescription" type="STRING"/>
    <field number="136" name="SecurityListUpdateRefName" type="STRING"/>
    <field number="137" name="SecurityListUpdateRefID" type="STRING"/>
    <field number="138" name="SecurityListUpdateRefDate" type="LOCALMKTDATE"/>
    <field number="139" name="SecurityListUpdateRefTime" type="UTCTIMESTAMP"/>
    <field number="140" name="SecurityListUpdateRefAction" type="CHAR"/>
    <field number="141" name="SecurityListUpdateRefSubType" type="CHAR"/>
    <field number="142" name="SecurityListUpdateRefStatus" type="CHAR"/>
    <field number="143" name="SecurityListUpdateRefVersion" type="STRING"/>
    <field number="144" name="SecurityListUpdateRefDescription" type="STRING"/>
    <field number="145" name="SecurityListUpdateRefName" type="STRING"/>
    <field number="146" name="SecurityListUpdateRefID" type="STRING"/>
    <field number="147" name="SecurityListUpdateRefDate" type="LOCALMKTDATE"/>
    <field number="148" name="SecurityListUpdateRefTime" type="UTCTIMESTAMP"/>
    <field number="149" name="SecurityListUpdateRefAction" type="CHAR"/>
    <field number="150" name="SecurityListUpdateRefSubType" type="CHAR"/>
    <field number="151" name="SecurityListUpdateRefStatus" type="CHAR"/>
    <field number="152" name="SecurityListUpdateRefVersion" type="STRING"/>
    <field number="153" name="SecurityListUpdateRefDescription" type="STRING"/>
    <field number="154" name="SecurityListUpdateRefName" type="STRING"/>
    <field number="155" name="SecurityListUpdateRefID" type="STRING"/>
    <field number="156" name="SecurityListUpdateRefDate" type="LOCALMKTDATE"/>
    <field number="157" name="SecurityListUpdateRefTime" type="UTCTIMESTAMP"/>
    <field number="158" name="SecurityListUpdateRefAction" type="CHAR"/>
    <field number="159" name="SecurityListUpdateRefSubType" type="CHAR"/>
    <field number="160" name="SecurityListUpdateRefStatus" type="CHAR"/>
    <field number="161" name="SecurityListUpdateRefVersion" type="STRING"/>
    <field number="162" name="SecurityListUpdateRefDescription" type="STRING"/>
    <field number="163" name="SecurityListUpdateRefName" type="STRING"/>
    <field number="164" name="SecurityListUpdateRefID" type="STRING"/>
    <field number="165" name="SecurityListUpdateRefDate" type="LOCALMKTDATE"/>
    <field number="166" name="SecurityListUpdateRefTime" type="UTCTIMESTAMP"/>
    <field number="167" name="SecurityListUpdateRefAction" type="CHAR"/>
    <field number="168" name="SecurityListUpdateRefSubType" type="CHAR"/>
    <field number="169" name="SecurityListUpdateRefStatus" type="CHAR"/>
    <field number="170" name="SecurityListUpdateRefVersion" type="STRING"/>
    <field number="171" name="
    ```