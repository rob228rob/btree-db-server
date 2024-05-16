cmake_minimum_required(VERSION 3.26)
project(cw_os)

set(CMAKE_CXX_STANDARD 17)

add_executable(cw_os
        cw/database/src/main.cpp
        C:/Users/rob22/CLionProjects/cw_os/allocator/allocator/src/allocator_guardant.cpp
        b-tree/b_tree.h
        b-tree/search_tree.h
        cw/database/include/data_base.h
        cw/user_data/include/user_data.h
        cw/common/include/storage_interface.h
        cw/table/include/table.h
        cw/user_data/src/user_data.cpp
        cw/table/src/table.cpp
        cw/schema/include/schema.h
        cw/schema/src/schema.cpp
        cw/schemas_pool/include/schemas_pool.h
        cw/schemas_pool/src/schemas_pool.cpp
        cw/database/src/data_base.cpp
        logger/client_logger/src/client_logger.cpp
        logger/logger/src/logger.cpp
        logger/logger/src/logger_guardant.cpp
        cw/test/test.cpp
)


add_executable(test
        cw/user_data/src/user_data.cpp
        C:/Users/rob22/CLionProjects/cw_os/allocator/allocator/src/allocator_guardant.cpp
        cw/table/src/table_test.cpp
        cw/table/src/table.cpp
        logger/client_logger/src/client_logger.cpp
        logger/logger/src/logger.cpp
        logger/logger/src/logger_guardant.cpp
)

add_executable(new_test
        C:/Users/rob22/CLionProjects/cw_os/cw/test/test.cpp
        C:/Users/rob22/CLionProjects/cw_os/cw/table/src/table.cpp
        C:/Users/rob22/CLionProjects/cw_os/cw/user_data/src/user_data.cpp
        C:/Users/rob22/CLionProjects/cw_os/allocator/allocator/src/allocator_guardant.cpp
        logger/client_logger/src/client_logger.cpp
        logger/logger/src/logger.cpp
        logger/logger/src/logger_guardant.cpp
)