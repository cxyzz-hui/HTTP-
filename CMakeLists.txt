# 修正后的CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(Server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加所有必需的源文件
set(SOURCE_FILES
    src/Event/Epoll.cpp
    src/Event/Channel.cpp
    src/Event/EventLoop.cpp
    src/Http/HttpServer.cpp
    src/Http/HttpContext.cpp
    src/Http/HttpRequest.cpp
    src/Http/HttpResponse.cpp
    src/Log/Logger.cpp
    src/Log/LogFile.cpp
    src/Log/LogStream.cpp
    src/Log/AppendFile.cpp
    src/Log/FixedBuffer.cpp
    src/Log/AsyncLogger.cpp
    src/Pool/EventPool/EventLoopThread.cpp
    src/Pool/EventPool/EventLoopThreadPool.cpp
    src/Pool/Thread/ThreadPool.cpp
    src/Pool/Thread/CurrentThread.cpp
    src/Socket/Server.cpp
    src/Socket/Socket.cpp
    src/Socket/Acceptor.cpp
    src/Socket/InetAddr.cpp
    src/Socket/Connection.cpp
    src/Time/Timer.cpp
    src/Time/Timestamp.cpp
    src/Time/TimerQueue.cpp
    src/Util/util.cpp
    src/Buffer.cpp
    main.cpp
)

# 创建可执行文件
add_executable(Server ${SOURCE_FILES})

include_directories(src)
target_link_libraries(Server pthread)