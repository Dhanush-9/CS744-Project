cmake_minimum_required(VERSION 3.10)
project(HTTPKeyValueServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MySQL client library (make sure libmysqlclient-dev is installed)
find_package(Threads REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/src)

# Add the executable
add_executable(kv_server
    src/cache.cpp
    src/db.cpp
    src/server.cpp
    src/main.cpp
)

# Link pthread and MySQL client
target_link_libraries(kv_server
    mysqlclient
    Threads::Threads
)

