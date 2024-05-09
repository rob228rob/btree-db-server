#include <gtest/gtest.h>
#include "client_logger.h"
#include "client_logger_builder.h"
#include "logger.h"


int main(
    int argc,
    char *argv[])
{
    logger* lg = (new client_logger_builder("%s %m"))->add_console_stream(logger::severity::debug)->add_file_stream("C:\\Users\\rob22\\CLionProjects\\mp_os\\logger\\client_logger\\tests\\1.txt", logger::severity::debug)->add_file_stream("new.txt", logger::severity::error)->build();

    lg->log("FIsrekfsdp log", logger::severity::debug);
    lg->log("logger sev must be errr", logger::severity::error);
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}