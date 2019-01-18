BCS job contest
==================

Клиент-серверное приложение, обеспечивающее обмен данными, сериализованными по протоколу Protocol Buffers

Особенности решения
------------
* использован только API Windows (без вспомогательных библиотек и фреймфорков)
* на каждое подключение клиента создается отдельный поток
* обработка сетевых данных осуществляется в блокирующем режиме
* в случае занятого сетевого порта другим приложением сервер не запустится (описание ошибки доступно в журнале системных событий)
* возможна ситуация, при которой не будут прочитаны все данные из сокета (это конечно же требует небольшой доработки)
* после обработки одной команды клиент не завершается, а предлагает ввести новую команду
* добавлены команды `quit` для завершения клиента


Установка
------------

Зависимости:
* **Protocol Buffers v3.6.1** 
* **MS redistributable**

#### Windows

`Protocol Buffers` необходимо скомпилировать с поддержкой `msvcr* runtime` (/MD) [MSDN](https://docs.microsoft.com/en-us/previous-versions/visualstudio/visual-studio-2013/2kzt1wy3(v=vs.120)).

Если сборка идет с использованием CMake необходимо выставить флаг:

``` sh
> -Dprotobuf_MSVC_STATIC_RUNTIME=OFF
```

Перед сборкой решения необходимо присвоить переменной окружения `PROTOBUF` путь к SDK c Protocol Buffers. Данная переменная используется в настройках проекта сборки.


Сборка решения:
``` sh
> msbuild bcs.sln
```

После сборки установить один из модулей в качестве windows-сервиса. Для этого необходимо открыть окно командной строки с правами администратора и запустить приложение: 
``` ps
> winSvc.exe install
```

Состав модулей
------------
* libMsg - библиотека сериализации сообщений
* libSrv - многопоточный TCP сервер 
* winSvc - windows-сервис 
* winCln - клиентское приложение
* winSrv - тестовое приложение. Может быть использовано вместо сервиса


Настройка 
------------
Если в каталоге с исполняемыми модулями будет содержаться конфигурационный файл `app.ini`, то ряд настроек будет читаться из него. При его отсутствии используются настройки по умолчанию:
``` app.ini
 [global]
 port=27015
 conn_count=5
 time_limit=3000
```

где `conn_count` - максимальное число одновременно подключенных клиентов, а `time_limit=3000` - максимальное время обработки запроса от подключившегося клиента в миллисекундах.

Запуск и работа 
------------
После установки сервиса `winSvc` он может быть запущен стандартным путем через диспетчер задач или при помощи оснастки "Сервисы". 
Название сервиса `BCS_pbuf_svc`.

Запуск клиента осуществляется командой:
``` ps
> winCln.exe host port
```
где `host` и ` port` - параметры подключения к серверу.
После запуска клиента будет предложено ввести одну из команд:
```
GET VARIABLE
GETT VARIABLE TIMEOUT
SET VARIABLE VALUE
SETRT VARIABLE TIMEOUT
```
где `TIMEOUT` для повтора подключения и отправки команды задается в миллисекундах.

Все события на серверной стороне пишутся в лог-файл `app.txt`, а обрабатываемые данные сохраняются в файл базы данных `app.db`.

Установка пакетов от Microsoft
------------
При запуске приложения могут потребоваться распространяемые пакеты от Microsoft. Об этом могут свидетельствовать сообщения об отсутствии библиотек `msvcr*.dll` и `msvcp*.dll`. Необходимо [скачать](https://support.microsoft.com/ru-ru/help/2977003/the-latest-supported-visual-c-downloads) такие пакеты и установить в зависимости от разрядности сборки приложения (x86/x64).

Дополнительно
------------
* https://github.com/Microsoft/vcpkg
* https://github.com/protocolbuffers/protobuf
* https://www.sqlite.org/index.html
* https://support.microsoft.com/ru-ru/help/2977003/the-latest-supported-visual-c-downloads
