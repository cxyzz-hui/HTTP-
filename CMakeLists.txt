# 修正后的CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(Server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加所有必需的源文件
set(SOURCE_FILES
    src/main.cpp
    src/Server.cpp
    src/Connection.cpp
    src/EventLoop.cpp
    src/Channel.cpp
    src/Epoll.cpp
    src/Socket.cpp
    src/InetAddr.cpp
    src/Acceptor.cpp
    src/Buffer.cpp
    src/util.cpp          # 新增：工具函数
    src/ThreadPool.cpp
    src/EventLoopThread.cpp
    src/EventLoopThreadPool.cpp
    src/EventLoop.cpp
)

# 创建可执行文件
add_executable(Server ${SOURCE_FILES})

target_include_directories(Server PRIVATE .)
target_link_libraries(Server pthread)