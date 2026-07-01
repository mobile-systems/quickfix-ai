# вывод интерактивного графика R при обновлении БД

Разберу задачу пошагово — как организовать вывод интерактивного графика в R при обновлении базы данных.

## Общий алгоритм

1. Подключиться к БД.
2. Получить актуальные данные.
3. Построить график.
4. Сделать его интерактивным.
5. Настроить обновление при изменении данных.

## Пошаговая реализация

### Шаг 1. Подключение к БД и загрузка данных

Для подключения к БД используйте соответствующие пакеты:
* `DBI` + драйвер (например, `RSQLite`, `RMySQL`, `odbc`) — для реляционных БД;
* `mongolite` — для MongoDB;
* другие пакеты для специфических БД.

**Пример подключения к SQLite:**

```r
library(DBI)
library(RSQLite)

# Подключение к БД
con <- dbConnect(SQLite(), "my_database.db")

# Функция для получения данных
get_data <- function() {
  query <- "SELECT * FROM my_table"
  data <- dbGetQuery(con, query)
  return(data)
}
```

### Шаг 2. Создание интерактивного графика

Используйте пакеты для интерактивной визуализации:

**Вариант 1: `plotly`**

```r
library(plotly)

create_interactive_plot <- function(data) {
  p <- plot_ly(
    data,
    x = ~x_column,
    y = ~y_column,
    type = 'scatter',
    mode = 'lines+markers'
  ) %>%
  layout(
    title = "Интерактивный график",
    xaxis = list(title = "Ось X"),
    yaxis = list(title = "Ось Y")
  )
  return(p)
}
```

**Вариант 2: `ggplot2` + `ggplotly`**

```r
library(ggplot2)
library(plotly)

create_ggplot_interactive <- function(data) {
  p <- ggplot(data, aes(x = x_column, y = y_column)) +
    geom_line() +
    geom_point() +
    labs(title = "Интерактивный график с ggplot2")
  ggplotly(p)
}
```

**Вариант 3: `highcharter`**

```r
library(highcharter)

create_highchart_plot <- function(data) {
  hchart(data, "line", hcaes(x = x_column, y = y_column)) %>%
  hc_title(text = "Интерактивный график Highcharter")
}
```

### Шаг 3. Автоматическое обновление графика

#### Способ 1. Периодическая проверка (polling)

Используйте `shiny` для веб‑интерфейса с автоматическим обновлением.

**Файл `app.R`:**

```r
library(shiny)
library(plotly)
library(DBI)
library(RSQLite)

ui <- fluidPage(
  titlePanel("Автообновляемый интерактивный график"),
  plotlyOutput("interactive_plot"),
  actionButton("refresh", "Обновить вручную")
)

server <- function(input, output, session) {
  # Подключение к БД
  con <- dbConnect(SQLite(), "my_database.db")
  
  # Данные с периодическим обновлением
  data_reactive <- reactive({
    invalidateLater(5000, session)  # Обновление каждые 5 с
    input$refresh  # И при нажатии кнопки
    dbGetQuery(con, "SELECT * FROM my_table")
  })
  
  # Вывод графика
  output$interactive_plot <- renderPlotly({
    data <- data_reactive()
    plot_ly(data, x = ~x_column, y = ~y_column, type = 'scatter', mode = 'lines+markers')
  })
}

shinyApp(ui, server)
```

#### Способ 2. Триггеры в БД

Если БД поддерживает триггеры (например, PostgreSQL), настройте уведомление об изменении данных и перехватывайте его в R.

**Пример для PostgreSQL:**

1. Создайте триггер в БД:
```sql
CREATE OR REPLACE FUNCTION notify_update() RETURNS trigger AS $$
BEGIN
  PERFORM pg_notify('data_update', 'table_updated');
  RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER data_change
  AFTER INSERT OR UPDATE ON my_table
  FOR EACH ROW EXECUTE FUNCTION notify_update();
```
2. В R используйте `RPostgres` для подписки на уведомления и обновления графика при получении сигнала.

#### Способ 3. Файловый мониторинг

Если данные обновляются через файлы, отслеживайте изменения с помощью `fs::file_info()` или `file.mtime()`.

**Пример:**

```r
check_file_update <- function(file_path, last_modified) {
  current_mtime <- file.mtime(file_path)
  if (current_mtime > last_modified) {
    return(list(updated = TRUE, new_time = current_mtime))
  } else {
    return(list(updated = FALSE, new_time = last_modified))
  }
}
```

## Полный рабочий пример

**Сценарий:** график продаж обновляется каждые 10 секунд при изменении данных в SQLite.

```r
# install.packages(c("shiny", "plotly", "DBI", "RSQLite"))

library(shiny)
library(plotly)
library(DBI)
library(RSQLite)

# UI
ui <- fluidPage(
  titlePanel("График продаж (автообновление)"),
  plotlyOutput("sales_plot"),
  textOutput("last_update")
)

# Server
server <- function(input, output, session) {
  # Подключение к БД
  con <- dbConnect(SQLite(), "sales.db")
  
  # Реактивные данные с обновлением каждые 10 с
  sales_data <- reactive({
    invalidateLater(10000, session)
    dbGetQuery(con, "SELECT date, sales FROM sales_data ORDER BY date")
  })
  
  # Вывод графика
  output$sales_plot <- renderPlotly({
    data <- sales_data()
    plot_ly(
      data,
      x = ~date,
      y = ~sales,
      type = 'scatter',
      mode = 'lines+markers',
      name = 'Продажи'
    ) %>%
    layout(
      title = "Продажи по дням",
      xaxis = list(title = "Дата"),
      yaxis = list(title = "Объём продаж")
    )
  })
  
  # Время последнего обновления
  output$last_update <- renderText({
    paste("Последнее обновление:", format(Sys.time(), "%H:%M:%S"))
  })
}

# Запуск приложения
shinyApp(ui, server)
```

## Рекомендации

* **Для веб‑приложений** используйте `shiny` + `plotly`/`highcharter` — это самый гибкий вариант.
* **Для десктопа** подойдёт `plotly` без `shiny`, но обновление придётся запускать вручную.
* **Частота обновления:** выбирайте интервал, исходя из нагрузки на БД и важности данных (5–60 с обычно достаточно).
* **Обработка ошибок:** добавьте `tryCatch()` вокруг запросов к БД, чтобы приложение не падало при проблемах с соединением.
* **Оптимизация:** кэшируйте данные или запрашивайте только новые записи, если БД большая.

---

## Краткий итог

Чтобы вывести интерактивный график R при обновлении БД:
1. Подключитесь к БД через соответствующий пакет.
2. Получите данные функцией, которую можно вызвать повторно.
3. Постройте график с `plotly`, `ggplotly` или `highcharter`.
4. Оберните логику в `shiny`‑приложение с `invalidateLater()` для автообновления.
5. Запустите приложение — график будет обновляться по расписанию или по событию.

Хотите, я уточню какой‑то этап подробнее или помогу адаптировать код под ваш сценарий?

# Можно вывести несколько графиков?

Да, в R можно вывести несколько интерактивных графиков одновременно — разберу варианты реализации.

## Основные способы размещения нескольких графиков

**1. В одном `plotly`-объекте (на одной оси с разными сериями)**

Подходит, когда графики связаны одной темой и осью.

```r
library(plotly)

create_multiple_series <- function(data) {
  plot_ly() %>%
    add_trace(
      data = data,
      x = ~date,
      y = ~sales,
      type = 'scatter',
      mode = 'lines+markers',
      name = 'Продажи'
    ) %>%
    add_trace(
      data = data,
      x = ~date,
      y = ~revenue,
      type = 'scatter',
      mode = 'lines+markers',
      name = 'Выручка',
      yaxis = 'y2'
    ) %>%
    layout(
      title = "Продажи и выручка",
      yaxis = list(title = "Продажи"),
      yaxis2 = list(
        title = "Выручка",
        overlaying = "y",
        side = "right"
      )
    )
}
```

**2. Сетка графиков (`subplot`)**

Размещает несколько независимых графиков рядом.

```r
create_subplots <- function(data) {
  p1 <- plot_ly(data, x = ~date, y = ~sales, type = 'scatter', mode = 'lines', name = 'Продажи')
  p2 <- plot_ly(data, x = ~date, y = ~profit, type = 'scatter', mode = 'lines', name = 'Прибыль')
  p3 <- plot_ly(data, x = ~date, y = ~customers, type = 'scatter', mode = 'lines', name = 'Клиенты')

  subplot(p1, p2, p3, nrows = 3, shareX = TRUE) %>%
    layout(title = "Три графика в сетке")
}
```

**3. `shiny`-приложение с несколькими `plotlyOutput`**

Оптимальный вариант для веб‑интерфейса с автообновлением.

**Файл `app.R`:**

```r
library(shiny)
library(plotly)
library(DBI)
library(RSQLite)

ui <- fluidPage(
  titlePanel("Мониторинг показателей"),
  fluidRow(
    column(6, plotlyOutput("plot_sales")),
    column(6, plotlyOutput("plot_revenue"))
  ),
  fluidRow(
    column(6, plotlyOutput("plot_profit")),
    column(6, plotlyOutput("plot_customers"))
  ),
  actionButton("refresh", "Обновить данные")
)

server <- function(input, output, session) {
  # Подключение к БД
  con <- dbConnect(SQLite(), "my_database.db")

  # Реактивные данные
  data_reactive <- reactive({
    invalidateLater(10000, session)  # Обновление каждые 10 с
    input$refresh
    dbGetQuery(con, "SELECT * FROM metrics")
  })

  # График 1: Продажи
  output$plot_sales <- renderPlotly({
    data <- data_reactive()
    plot_ly(
      data,
      x = ~date,
      y = ~sales,
      type = 'scatter',
      mode = 'lines+markers'
    ) %>% layout(title = "Продажи по дням")
  })

  # График 2: Выручка
  output$plot_revenue <- renderPlotly({
    data <- data_reactive()
    plot_ly(
      data,
      x = ~date,
      y = ~revenue,
      type = 'scatter',
      mode = 'lines+markers'
    ) %>% layout(title = "Выручка по дням")
  })

  # График 3: Прибыль
  output$plot_profit <- renderPlotly({
    data <- data_reactive()
    plot_ly(
      data,
      x = ~date,
      y = ~profit,
      type = 'scatter',
      mode = 'lines+markers'
    ) %>% layout(title = "Прибыль по дням")
  })

  # График 4: Клиенты
  output$plot_customers <- renderPlotly({
    data <- data_reactive()
    plot_ly(
      data,
      x = ~date,
      y = ~customers,
      type = 'scatter',
      mode = 'lines+markers'
    ) %>% layout(title = "Количество клиентов")
  })
}

shinyApp(ui, server)
```

**4. `ggplot2` + `patchwork`**

Для статических графиков с возможностью экспорта.

```r
library(ggplot2)
library(patchwork)

create_ggplots <- function(data) {
  p1 <- ggplot(data, aes(x = date, y = sales)) +
    geom_line() +
    labs(title = "Продажи")

  p2 <- ggplot(data, aes(x = date, y = revenue)) +
    geom_line(color = "red") +
    labs(title = "Выручка")

  p3 <- ggplot(data, aes(x = date, y = profit)) +
    geom_line(color = "green") +
    labs(title = "Прибыль")

  # Компоновка в сетку 2×2
  (p1 + p2) / (p3 + plot_spacer())
}
```

## Расширенный пример: дашборд с фильтрами

**Файл `app.R` (расширенная версия):**

```r
library(shiny)
library(plotly)
library(dplyr)

ui <- fluidPage(
  titlePanel("Бизнес‑дашборд"),
  sidebarLayout(
    sidebarPanel(
      dateRangeInput("date_range", "Период:", start = "2023-01-01"),
      selectInput("metric", "Показатель:",
                 choices = c("Продажи", "Выручка", "Прибыль", "Клиенты"))
    ),
    mainPanel(
      fluidRow(
        column(6, plotlyOutput("main_plot")),
        column(6, plotlyOutput("comparison_plot"))
      ),
      fluidRow(
        column(4, plotlyOutput("pie_sales")),
        column(4, plotlyOutput("bar_revenue")),
        column(4, plotlyOutput("line_trend"))
      )
    )
  )
)

server <- function(input, output, session) {
  # Имитация данных (замените на запрос к БД)
  simulate_data <- reactive({
    dates <- seq(as.Date("2023-01-01"), as.Date("2023-12-31"), by = "day")
    data.frame(
      date = dates,
      sales = runif(length(dates), 100, 500),
      revenue = runif(length(dates), 5000, 20000),
      profit = runif(length(dates), 500, 3000),
      customers = sample(50:200, length(dates), replace = TRUE)
    )
  })

  # Фильтрованные данные
  filtered_data <- reactive({
    data <- simulate_data()
    data %>%
      filter(date >= input$date_range[1], date <= input$date_range[2])
  })

  # Основной график (выбранный показатель)
  output$main_plot <- renderPlotly({
    data <- filtered_data()
    y_col <- switch(input$metric,
                   "Продажи" = ~sales,
                   "Выручка" = ~revenue,
                   "Прибыль" = ~profit,
                   "Клиенты" = ~customers)

    plot_ly(data, x = ~date, y = y_col, type = 'scatter', mode = 'lines') %>%
      layout(title = paste("Динамика", input$metric))
  })

  # Сравнительный график
  output$comparison_plot <- renderPlotly({
    data <- filtered_data()
    plot_ly(data, x = ~date) %>%
      add_lines(y = ~sales, name = "Продажи", line = list(color = "blue")) %>%
      add_lines(y = ~profit, name = "Прибыль", line = list(color = "green"), yaxis = "y2") %>%
      layout(
        title = "Сравнение продаж и прибыли",
        yaxis = list(title = "Продажи"),
        yaxis2
```        


# В R используйте RPostgres для подписки на уведомления и обновления графика при получении сигнала.

Разберу задачу пошагово — покажу, как в R с помощью пакета `RPostgres` подписаться на уведомления PostgreSQL и обновлять график при получении сигнала.

## Шаг 1. Установка и подключение

Убедитесь, что установлены пакеты, и подключитесь к БД:

```r
# Установка пакетов (если не установлены)
# install.packages("DBI")
# install.packages("RPostgres")
# install.packages("callr")

library(DBI)
library(RPostgres)
library(callr)

# Подключение к PostgreSQL
conn <- dbConnect(RPostgres::Postgres(),
                  dbname = "your_db",
                  host = "localhost",
                  port = 5432,
                  user = "your_user",
                  password = "your_password")
```

## Шаг 2. Подписка на уведомления

Подписываемся на канал уведомлений с помощью команды `LISTEN`:

```r
# Подписываемся на канал "graph_updates"
dbExecute(conn, "LISTEN graph_updates")
```

## Шаг 3. Функция для обновления графика

Создадим функцию, которая будет строить или обновлять график на основе данных из БД:

```r
update_plot <- function() {
  # Получаем актуальные данные для графика
  data <- dbGetQuery(conn, "SELECT x, y FROM graph_data ORDER BY x")
  
  # Очищаем предыдущее изображение и строим новый график
  plot(data$x, data$y, type = "l",
       main = "Обновляемый график",
       xlab = "X", ylab = "Y")
}
```

## Шаг 4. Основной цикл прослушивания

Реализуем цикл, который будет ждать уведомлений и обновлять график:

```r
cat("Ожидание уведомлений на канале 'graph_updates'...\n")

while (TRUE) {
  # Ждём уведомления (таймаут 1 секунда)
  notification <- RPostgres::postgresWaitForNotify(conn, timeout = 1)
  
  if (!is.null(notification)) {
    cat(paste("Получено уведомление:", notification$payload, "\n"))
    
    # Обновляем график
    update_plot()
    
    # Дополнительная логика на основе полезной нагрузки
    if (notification$payload == "refresh_full") {
      cat("Выполняется полное обновление данных...\n")
      # Здесь можно добавить код для перезагрузки данных
    }
  }
  
  # Небольшая пауза между проверками (чтобы не нагружать CPU)
  Sys.sleep(0.1)
}
```

## Шаг 5. Тестирование системы

Для тестирования запустим отдельный процесс, который будет отправлять уведомления:

```r
# Запускаем фоновый процесс для отправки тестовых уведомлений
test_process <- callr::r_bg(function() {
  library(DBI)
  library(RPostgres)
  
  # Подключаемся к той же БД
  test_conn <- dbConnect(RPostgres::Postgres(),
                       dbname = "your_db",
                       host = "localhost",
                       port = 5432,
                       user = "your_user",
                       password = "your_password")
  
  # Отправляем тестовые уведомления с задержкой
  for (i in 1:5) {
    Sys.sleep(3)  # Ждём 3 секунды
    dbExecute(test_conn,
              "NOTIFY graph_updates, 'Обновление графика №{}'",
              params = list(i))
  }
  
  dbDisconnect(test_conn)
})
```

## Шаг 6. Подготовка базы данных

Перед запуском создайте таблицу и настройте триггеры (если нужно):

```sql
-- Создаём таблицу для данных графика
CREATE TABLE graph_data (
  x SERIAL PRIMARY KEY,
  y NUMERIC
);

-- Добавляем тестовые данные
INSERT INTO graph_data (y) VALUES
  (1), (4), (9), (16), (25);

-- Опционально: создаём триггер для автоматических уведомлений
-- при изменении данных
CREATE OR REPLACE FUNCTION notify_graph_update()
RETURNS TRIGGER AS $$
BEGIN
  PERFORM pg_notify('graph_updates', 'data_changed');
  RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER graph_update_trigger
  AFTER INSERT OR UPDATE ON graph_data
  FOR EACH ROW EXECUTE FUNCTION notify_graph_update();
```

## Полный рабочий пример

Объединим всё в один скрипт:

```r
library(DBI)
library(RPostgres)
library(callr)

# 1. Подключение
conn <- dbConnect(RPostgres::Postgres(),
                  dbname = "test_db", host = "localhost")

# 2. Подписка
dbExecute(conn, "LISTEN graph_updates")

# 3. Функция обновления графика
update_plot <- function() {
  data <- dbGetQuery(conn, "SELECT x, y FROM graph_data ORDER BY x")
  plot(data$x, data$y, type = "b",
       main = paste("График обновлён:", Sys.time()),
       xlab = "X", ylab = "Y")
}

# 4. Основной цикл
cat("Система запущена. Ожидание уведомлений...\n")

tryCatch({
  while (TRUE) {
    notification <- RPostgres::postgresWaitForNotify(conn, timeout = 0.5)
    
    if (!is.null(notification)) {
      cat(paste("[", format(Sys.time(), "%H:%M:%S"), "] ",
            "Уведомление:", notification$payload, "\n", sep = ""))
      update_plot()
    }
    
    Sys.sleep(0.1)  # Небольшая пауза
  }
}, finally = {
  cat("Отключение от БД...\n")
  dbDisconnect(conn)
})
```

---

## Важные замечания

1. **Выделенное соединение**. Для прослушивания уведомлений нужно отдельное соединение с БД — не используйте его для других операций.
2. **Таймауты**. Параметр `timeout` в `postgresWaitForNotify()` определяет, сколько ждать уведомления (в секундах). `timeout = 0` — ждать бесконечно.
3. **Обработка ошибок**. Добавьте обработку ошибок в реальном приложении (например, переподключение при разрыве соединения).
4. **Ресурсы**. Долгое соединение занимает ресурс на сервере PostgreSQL.
5. **Каналы**. Имена каналов (`graph_updates` в примере) должны совпадать между `LISTEN` и `NOTIFY`.
6. **Графика**. Для интерактивных графиков в веб‑приложениях рассмотрите `shiny` вместо базового `plot()`.

Хотите, я уточню какой‑то из шагов или помогу адаптировать код под ваш конкретный случай?

