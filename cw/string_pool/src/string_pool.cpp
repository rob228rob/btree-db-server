//
// Created by rob22 on 22.05.2024.
//

#include "../include/string_pool.h"

int64_t string_pool::_pool_size = 0;

string_pool *string_pool::_instance;

std::mutex string_pool::_mutex;
