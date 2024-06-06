cmake_minimum_required(VERSION 3.26)
project(cw_os)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Установка предельных значений для использования функций Windows API
add_definitions(-DWIN32_LEAN_AND_MEAN)
add_definitions(-D_WIN32_WINNT=0x0A00)

# Путь к заголовочным файлам json и httplib
include_directories("D:/General Downloads/json-3.11.2/json-3.11.2/include")

include_directories("D:/General Downloads/Newfeature/yhirose-cpp-httplib-a609330")

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)

# For Windows users: prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(
        googletest)


set(MAIN_SOURCE
        cw/database/src/main.cpp
        cw/user_data/src/user_data.cpp
        cw/table/src/table.cpp
        cw/schema/src/schema.cpp
        cw/schemas_pool/src/schemas_pool.cpp
        cw/database/src/data_base.cpp
        logger/client_logger/src/client_logger.cpp
        logger/logger/src/logger.cpp
        logger/logger/src/logger_guardant.cpp
        allocator/allocator/src/allocator_guardant.cpp
        allocator/allocator/src/allocator_test_utils.cpp
        allocator/allocator_boundary_tags/src/allocator_boundary_tags.cpp
        cw/string_pool/src/string_pool.cpp
)

# Список всех исходных файлов для client_requests.cpp
set(CLIENT_REQUESTS_SOURCES
        cw/server/src/сlient_requests.cpp
)

# Определяем исполняемый файл main
add_executable(main cw/database/src/main.cpp
        cw/user_data/src/user_data.cpp
        cw/table/src/table.cpp
        cw/schema/src/schema.cpp
        cw/schemas_pool/src/schemas_pool.cpp
        cw/database/src/data_base.cpp
        logger/client_logger/src/client_logger.cpp
        logger/logger/src/logger.cpp
        logger/logger/src/logger_guardant.cpp
        allocator/allocator/src/allocator_guardant.cpp
        allocator/allocator/src/allocator_test_utils.cpp
        allocator/allocator_boundary_tags/src/allocator_boundary_tags.cpp
        cw/string_pool/src/string_pool.cpp
        allocator/allocator_global_heap/src/allocator_global_heap.cpp)

target_link_libraries(
        main
        PRIVATE
        gtest_main)


add_executable(server_db
        cw/user_data/src/user_data.cpp
        cw/table/src/table.cpp
        cw/schema/src/schema.cpp
        cw/schemas_pool/src/schemas_pool.cpp
        cw/database/src/data_base.cpp
        logger/client_logger/src/client_logger.cpp
        logger/logger/src/logger.cpp
        logger/logger/src/logger_guardant.cpp
        allocator/allocator/src/allocator_guardant.cpp
        allocator/allocator/src/allocator_test_utils.cpp
        allocator/allocator_boundary_tags/src/allocator_boundary_tags.cpp
        cw/string_pool/src/string_pool.cpp
        cw/server/src/db_server.cpp)

# Определяем исполняемый файл client_requests
add_executable(cli_server ${CLIENT_REQUESTS_SOURCES})


# Линковка библиотек с исполняемым файлом сервера
target_link_libraries(server_db PRIVATE ws2_32 iphlpapi)

#target_link_libraries(main PUBLIC gtest gtest_main)