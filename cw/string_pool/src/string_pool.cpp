//
// Created by rob22 on 22.05.2024.
//

#include "../include/string_pool.h"

string_pool* string_pool::_instance;

std::string string_pool::_storage_filename = "string_pool.txt";

std::mutex string_pool::_mutex;

std::set<std::string> string_pool::_pool;

std::fstream string_pool::_file;
