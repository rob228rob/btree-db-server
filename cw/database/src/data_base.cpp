//
// Created by rob22 on 06.05.2024.
//

#include "../include/data_base.h"
#include "../../../allocator/allocator_boundary_tags/include/allocator_boundary_tags.h"
#include <filesystem>
#include <fstream>
#include <iostream>

//std::function<int(const std::string &, const std::string &)> data_base::_default_string_comparer = [](const std::string &a, const std::string &b) -> int { return a.compare(b); };

struct custom_compare {
    int operator()(int const &a, int const &b) const
    {
	if (a < b)
	{
	    return -1;
	}
	else if (a > b)
	{
	    return 1;
	}

	return 0;
    }
};

struct string_compare {
    int operator()(std::string const &a, std::string const &b) const
    {
	if (a < b)
	{
	    return -1;
	}
	else if (a > b)
	{
	    return 1;
	}

	return 0;
    }
};


std::function<int(const int &, const int &)> data_base::_int_comparer = custom_compare();

std::function<int(const std::string &, const std::string &)> data_base::_default_string_comparer = string_compare();


data_base::data_base(const std::string &instance_name, storage_strategy strategy, allocator *allocator)
    : _data(std::make_unique<b_tree<std::string, schemas_pool>>(8, _default_string_comparer, allocator, nullptr))
{
    this->_allocator = allocator;
    this->_logger = nullptr;
    this->set_strategy(strategy);

    try
    {
	_instance_path = std::filesystem::absolute(instance_name);
	if (std::filesystem::exists(_instance_path))
	{
	    return;
	}
	std::filesystem::create_directories(_instance_path);
    }
    catch (...)
    {
	throw;
    }
}

data_base::~data_base()
{
}

data_base::data_base(data_base &&other) noexcept
    : _data(std::move(other._data))
{
}

data_base &data_base::operator=(data_base &&other) noexcept
{
    if (this != &other)
    {
	_data = std::move(other._data);
    }

    return *this;
}

void data_base::insert(const std::string &key, const schemas_pool &value)
{
    _data->insert(key, value);
}

void data_base::insert(const std::string &key, schemas_pool &&value)
{
    _data->insert(key, std::move(value));
}

void data_base::update(const std::string &key, const schemas_pool &value)
{
    _data->update(key, value);
}
void data_base::update(const std::string &key, schemas_pool &&value)
{
    _data->update(key, std::move(value));
}

schemas_pool &data_base::obtain(const std::string &key)
{
    return _data->obtain(key);
}

std::map<std::string, schemas_pool> data_base::obtain_between(
	const std::string &lower_bound,
	const std::string &upper_bound,
	bool lower_bound_inclusive,
	bool upper_bound_inclusive)
{
    auto vec = _data->obtain_between(lower_bound, upper_bound, lower_bound_inclusive, upper_bound_inclusive);
    std::map<std::string, schemas_pool> result_map;
    for (auto &elem: vec)
    {
	result_map.emplace(elem.key, elem.value);
    }

    return result_map;
}

void data_base::dispose(const std::string &key)
{
    try
    {
	_data->dispose(key);
    }
    catch (std::exception const &e)
    {

	throw;
    }
}

void data_base::serialize()
{
}

void data_base::insert_schemas_pool(const std::string &schemas_pool_name, const schemas_pool &value)
{
    throw_if_key_invalid(schemas_pool_name);

    std::lock_guard<std::mutex> lock(_mtx);
    switch (this->get_strategy())
    {
	case storage_strategy::in_memory:
	    insert(schemas_pool_name, value);
	    break;
	case storage_strategy::filesystem:
	    add_pool_to_filesystem(schemas_pool_name);
	    break;
    }
}

void data_base::insert_schemas_pool(const std::string &schemas_pool_name, schemas_pool &&value)
{
    throw_if_key_invalid(schemas_pool_name);

    std::lock_guard<std::mutex> lock(_mtx);
    switch (this->get_strategy())
    {
	case storage_strategy::in_memory:
	    insert(schemas_pool_name, std::move(value));
	    break;
	case storage_strategy::filesystem:
	    add_pool_to_filesystem(schemas_pool_name);
	    break;
    }
}

void data_base::insert_schema(const std::string &schemas_pool_name, const std::string &schema_name, const schema &value)
{
    throw_if_key_invalid(schemas_pool_name);
    throw_if_key_invalid(schema_name);

    std::lock_guard<std::mutex> lock(_mtx);
    switch (this->get_strategy())
    {
	case storage_strategy::in_memory: {
	    auto &pool = _data->obtain(schemas_pool_name);
	    pool.insert(schema_name, value);
	}
	break;
	case storage_strategy::filesystem:
	    add_schema_to_filesystem(schemas_pool_name, schema_name);
	    break;
    }
}

void data_base::insert_schema(const std::string &schemas_pool_name, const std::string &schema_name, schema &&value)
{
    throw_if_key_invalid(schemas_pool_name);
    throw_if_key_invalid(schema_name);

    std::lock_guard<std::mutex> lock(_mtx);
    switch (this->get_strategy())
    {
	case storage_strategy::in_memory: {
	    auto &pool = _data->obtain(schemas_pool_name);
	    pool.insert(schema_name, std::move(value));
	}
	break;
	case storage_strategy::filesystem:
	    add_schema_to_filesystem(schemas_pool_name, schema_name);
	    break;
    }
}

void data_base::insert_table(
	const std::string &schemas_pool_name,
	const std::string &schema_name,
	const std::string &table_name,
	const table &value)
{
    throw_if_key_invalid(schemas_pool_name);
    throw_if_key_invalid(schema_name);
    throw_if_key_invalid(table_name);

    std::lock_guard<std::mutex> lock(_mtx);
    switch (this->get_strategy())
    {
	case storage_strategy::in_memory: {
	    auto &pool = _data->obtain(schemas_pool_name);
	    auto &schm = pool.obtain(schema_name);
	    schm.insert(table_name, value);
	}
	break;
	case storage_strategy::filesystem:
	    add_table_to_filesystem(schemas_pool_name, schema_name, table_name);
	    break;
    }
}

void data_base::insert_table(
	const std::string &schemas_pool_name,
	const std::string &schema_name,
	const std::string &table_name,
	table &&value)
{
    throw_if_key_invalid(schemas_pool_name);
    throw_if_key_invalid(schema_name);
    throw_if_key_invalid(table_name);

    std::lock_guard<std::mutex> lock(_mtx);
    switch (this->get_strategy())
    {
	case storage_strategy::in_memory: {
	    auto &pool = _data->obtain(schemas_pool_name);
	    auto &schm = pool.obtain(schema_name);
	    schm.insert(table_name, value);
	}
	break;
	case storage_strategy::filesystem:
	    add_table_to_filesystem(schemas_pool_name, schema_name, table_name);
	    break;
    }
}

void data_base::insert_data(
	const std::string &schemas_pool_name,
	const std::string &schema_name,
	const std::string &table_name,
	const std::string &user_data_key,
	user_data &&value)
{
    throw_if_key_invalid(schemas_pool_name);
    throw_if_key_invalid(schema_name);
    throw_if_key_invalid(table_name);
    throw_if_key_invalid(user_data_key);

    std::lock_guard<std::mutex> lock(_mtx);
    switch (this->get_strategy())
    {
	case storage_strategy::in_memory: {
	    auto &pool = _data->obtain(schemas_pool_name);
	    auto &schm = pool.obtain(schema_name);
	    auto &tbl = schm.obtain(table_name);
	    tbl.insert(user_data_key, value);
	}
	break;
	case storage_strategy::filesystem:
	    insert_data_to_filesystem(schemas_pool_name, schema_name, table_name, user_data_key, value);
	    break;
    }
}

void data_base::insert_data(
	const std::string &schemas_pool_name,
	const std::string &schema_name,
	const std::string &table_name,
	const std::string &user_data_key,
	const user_data &value)
{
    throw_if_key_invalid(schemas_pool_name);
    throw_if_key_invalid(schema_name);
    throw_if_key_invalid(table_name);
    throw_if_key_invalid(user_data_key);

    std::lock_guard<std::mutex> lock(_mtx);
    switch (this->get_strategy())
    {
	case storage_strategy::in_memory: {
	    auto &pool = _data->obtain(schemas_pool_name);
	    auto &schm = pool.obtain(schema_name);
	    auto &tbl = schm.obtain(table_name);
	    tbl.insert(user_data_key, value);
	}
	break;
	case storage_strategy::filesystem:
	    insert_data_to_filesystem(schemas_pool_name, schema_name, table_name, user_data_key, value);
	    break;
    }
}

user_data data_base::obtain_data(const std::string &schemas_pool_name, const std::string &schema_name, const std::string &table_name, const std::string &user_data_key)
{
    throw_if_key_invalid(schemas_pool_name);
    throw_if_key_invalid(schema_name);
    throw_if_key_invalid(table_name);
    throw_if_key_invalid(user_data_key);

    std::lock_guard<std::mutex> lock(_mtx);
    switch (this->get_strategy())
    {
	case storage_strategy::in_memory: {
	    auto &pool = _data->obtain(schemas_pool_name);
	    auto &schm = pool.obtain(schema_name);
	    auto &tbl = schm.obtain(table_name);
	    const auto &data = tbl.obtain(user_data_key);
	    return data;
	}
	case storage_strategy::filesystem: {
	    auto ud = obtain_data_in_filesystem(schemas_pool_name, schema_name, table_name, user_data_key);
	    return ud;
	}
    }
}

void data_base::update_data(const std::string &schemas_pool_name, const std::string &schema_name, const std::string &table_name, const std::string &user_data_key, const user_data &value)
{
    throw_if_key_invalid(schemas_pool_name);
    throw_if_key_invalid(schema_name);
    throw_if_key_invalid(table_name);
    throw_if_key_invalid(user_data_key);

    std::lock_guard<std::mutex> lock(_mtx);
    switch (this->get_strategy())
    {
	case storage_strategy::in_memory: {
	    auto &pool = _data->obtain(schemas_pool_name);
	    auto &schm = pool.obtain(schema_name);
	    auto &tbl = schm.obtain(table_name);
	    tbl.update(user_data_key, value);
	}
	break;
	case storage_strategy::filesystem:
	    update_ud_in_filesystem(schemas_pool_name, schema_name, table_name, user_data_key, value);
	    break;
    }
}

void data_base::update_data(const std::string &schemas_pool_name, const std::string &schema_name, const std::string &table_name, const std::string &user_data_key, user_data &&value)
{
    throw_if_key_invalid(schemas_pool_name);
    throw_if_key_invalid(schema_name);
    throw_if_key_invalid(table_name);
    throw_if_key_invalid(user_data_key);

    std::lock_guard<std::mutex> lock(_mtx);
    switch (this->get_strategy())
    {
	case storage_strategy::in_memory: {
	    auto &pool = _data->obtain(schemas_pool_name);
	    auto &schm = pool.obtain(schema_name);
	    auto &tbl = schm.obtain(table_name);
	    tbl.update(user_data_key, value);
	}
	break;
	case storage_strategy::filesystem:
	    update_ud_in_filesystem(schemas_pool_name, schema_name, table_name, user_data_key, std::move(value));
	    break;
    }
}

void data_base::dispose_schemas_pool(const std::string &schemas_pool_name)
{
    throw_if_key_invalid(schemas_pool_name);

    std::lock_guard<std::mutex> lock(_mtx);
    switch (this->get_strategy())
    {
	case storage_strategy::in_memory:
	    _data->dispose(schemas_pool_name);
	    break;
	case storage_strategy::filesystem:
	    dispose_pool_in_filesystem(schemas_pool_name);
	    break;
    }
}

void data_base::dispose_schema(const std::string &schemas_pool_name, const std::string &schema_name)
{
    throw_if_key_invalid(schemas_pool_name);
    throw_if_key_invalid(schema_name);

    std::lock_guard<std::mutex> lock(_mtx);
    switch (this->get_strategy())
    {
	case storage_strategy::in_memory: {
	    auto &target_pool = _data->obtain(schemas_pool_name);
	    target_pool.dispose(schema_name);
	}
	break;
	case storage_strategy::filesystem:
	    dispose_schema_in_filesystem(schemas_pool_name, schema_name);
	    break;
    }
}

void data_base::dispose_table(const std::string &schemas_pool_name, const std::string &schema_name, const std::string &table_name)
{
    throw_if_key_invalid(schemas_pool_name);
    throw_if_key_invalid(schema_name);
    throw_if_key_invalid(table_name);

    std::lock_guard<std::mutex> lock(_mtx);
    switch (this->get_strategy())
    {
	case storage_strategy::in_memory: {
	    auto &target_pool = _data->obtain(schemas_pool_name);
	    auto &schema = target_pool.obtain(schema_name);
	    schema.dispose(table_name);
	}
	break;
	case storage_strategy::filesystem:
	    dispose_table_in_filesystem(schemas_pool_name, schema_name, table_name);
	    break;
    }
}

void data_base::dispose_user_data(
	const std::string &schemas_pool_name,
	const std::string &schema_name,
	const std::string &table_name,
	const std::string &user_data_key)
{
    throw_if_key_invalid(schemas_pool_name);
    throw_if_key_invalid(schema_name);
    throw_if_key_invalid(table_name);
    throw_if_key_invalid(user_data_key);

    std::lock_guard<std::mutex> lock(_mtx);
    switch (this->get_strategy())
    {
	case storage_strategy::in_memory: {
	    auto &target_pool = _data->obtain(schemas_pool_name);
	    auto &schema = target_pool.obtain(schema_name);
	    auto &tbl = schema.obtain(table_name);
	    tbl.dispose(user_data_key);
	}
	break;
	case storage_strategy::filesystem:
	    dispose_ud_in_filesystem(schemas_pool_name, schema_name, table_name, user_data_key);
	    break;
    }
}

void data_base::load_data_base_state()
{
    if (get_strategy() != storage_strategy::in_memory)
    {
	throw std::logic_error("Invalid strategy for this operation");
    }

    std::filesystem::path db_storage_path = _instance_path / _additional_storage;

    if (!std::filesystem::exists(db_storage_path))
    {
	throw std::runtime_error("Database directory does not exist: " + db_storage_path.string());
    }
    std::lock_guard<std::mutex> lock(_mtx);

    auto temp_file_path = _instance_path / ("meta.txt");
    if (std::filesystem::exists(temp_file_path))
    {
	std::ifstream temp_file(temp_file_path);
	std::string allocator_name, fit_mode;
	std::getline(temp_file, allocator_name);
	std::getline(temp_file, fit_mode);
	temp_file.close();
	allocator *alc;
	allocator_with_fit_mode::fit_mode mode = allocator_with_fit_mode::fit_mode::first_fit;

	if (fit_mode == "the_best_fit")
	{
	    mode = allocator_with_fit_mode::fit_mode::the_best_fit;
	}
	else if (fit_mode == "the_worst_fit")
	{
	    mode = allocator_with_fit_mode::fit_mode::the_worst_fit;
	}

	if (allocator_name == "allocator_boundary_tags")
	{
	    alc = new allocator_boundary_tags(100'000, nullptr, nullptr, mode);
	}
	else
	{
	    alc = nullptr;
	}

	this->set_allocator(alc);
    }

    for (auto &pool_entry: std::filesystem::directory_iterator(db_storage_path))
    {
	if (!pool_entry.is_directory())
	{
	    continue;
	}

	const auto &pool_path = pool_entry.path();
	std::cout << "Loading pool: " << pool_path << std::endl;
	schemas_pool pool;
	for (auto &schema_entry: std::filesystem::directory_iterator(pool_entry))
	{
	    if (!schema_entry.is_directory())
	    {
		continue;
	    }

	    auto schema_path = schema_entry.path();
	    std::cout << "Loading schema: " << schema_path << std::endl;
	    schema schm;
	    for (auto &table_entry: std::filesystem::directory_iterator(schema_entry))
	    {
		if (!table_entry.is_regular_file())
		{
		    continue;
		}

		auto table_path = table_entry.path();
		std::cout << "Found table: " << table_path << std::endl;

		std::cout << "Path to table file: " << table_entry.path() << std::endl;

		table tbl = table::load_data_from_filesystem(table_path.string());
		auto tbl_name = table_path.filename().string();
		for (int i = 0; i < _file_format.length(); ++i)
		{
		    tbl_name.pop_back();
		}

		schm.insert(tbl_name, std::move(tbl));
	    }

	    pool.insert(schema_path.filename().string(), schm);
	}
	_data->insert(pool_path.filename().string(), pool);
    }
}


void data_base::save_data_base_state()
{
    if (get_strategy() != storage_strategy::in_memory)
    {
	throw std::logic_error("Invalid strategy for this operation");
    }
    std::lock_guard<std::mutex> lock(_mtx);

    if (_allocator)
    {
	std::ofstream temp_file(_instance_path / ("meta.txt"));
	throw_if_not_open(temp_file);
	temp_file << _allocator->get_typename() << std::endl;
	temp_file << _allocator->get_fit_mode_str() << std::endl;
	temp_file.close();
    }

    auto it = _data->begin_infix();
    auto it_end = _data->end_infix();
    while (it != it_end)
    {
	auto string_key = std::get<2>(*it);
	auto target_schemas_pool = std::get<3>(*it);

	insert_pool_to_filesystem(string_key, std::move(target_schemas_pool));
	++it;
    }
}

void data_base::deserialize()
{
}

void data_base::throw_if_key_invalid(const std::string &key)
{
    if (key.empty())
    {
	throw std::logic_error("key must not be an empty");
    }

    size_t length = key.length();

    if (length > 20)
    {
	throw std::logic_error("key length too big");
    }

    for (size_t i = 0; i < length; ++i)
    {
	if (key[i] == '#' || key[i] == '|')
	{
	    throw std::logic_error("invalid symbol in key: " + std::string{key[i]});
	}
    }
}

std::map<std::string, user_data> data_base::obtain_between_data(const std::string &pool_name, const std::string &schema_name, const std::string &table_name, const std::string &lower_bound, const std::string &upper_bound, bool lower_bound_inclusive, bool upper_bound_inclusive)
{
    throw_if_key_invalid(pool_name);
    throw_if_key_invalid(schema_name);
    throw_if_key_invalid(table_name);
    throw_if_key_invalid(lower_bound);
    throw_if_key_invalid(upper_bound);

    std::lock_guard<std::mutex> lock(_mtx);

    switch (this->get_strategy())
    {
	case storage_strategy::in_memory: {
	    auto &pool = _data->obtain(pool_name);
	    auto &schm = pool.obtain(schema_name);
	    auto &tbl = schm.obtain(table_name);
	    return tbl.obtain_between(lower_bound, upper_bound, lower_bound_inclusive, upper_bound_inclusive);
	}
	case storage_strategy::filesystem:
	    return obtain_between_ud_in_filesystem(pool_name, schema_name, table_name, lower_bound, upper_bound, lower_bound_inclusive, upper_bound_inclusive);
    }
}

void data_base::insert_pool_to_filesystem(const std::string &pool_name, const schemas_pool &value)
{
    std::filesystem::path inmem_storage_path = _instance_path / (_additional_storage);

    if (!std::filesystem::exists(inmem_storage_path))
    {

	if (!std::filesystem::create_directory(inmem_storage_path))
	{
	    throw std::runtime_error("Directory already exist: " + pool_name);
	}
    }

    std::filesystem::path pool_path = inmem_storage_path / pool_name;

    if (!std::filesystem::exists(pool_path))
    {
	if (!std::filesystem::create_directory(pool_path))
	{
	    throw std::runtime_error("Directory already exist: " + pool_name);
	}
    }

    auto it = value._data->begin_infix();
    auto it_end = value._data->end_infix();
    while (it != it_end)
    {
	auto schema_name = std::get<2>(*it);
	auto target_schema = std::get<3>(*it);

	std::filesystem::path schema_path = pool_path / schema_name;

	if (!std::filesystem::exists(schema_path))
	{
	    if (!std::filesystem::create_directory(schema_path))
	    {
		throw std::runtime_error("Could not create dir" + schema_name);
	    }
	}

	target_schema.insert_schema_to_filesystem(schema_path);
	++it;
    }
}

void data_base::insert_pool_to_filesystem(const std::string &pool_name, schemas_pool &&value)
{
    std::filesystem::path inmem_storage_path = _instance_path / (_additional_storage);

    if (!std::filesystem::exists(inmem_storage_path))
    {

	if (!std::filesystem::create_directory(inmem_storage_path))
	{
	    throw std::runtime_error("Directory already exist: " + pool_name);
	}
    }

    std::filesystem::path pool_path = inmem_storage_path / pool_name;

    if (!std::filesystem::exists(pool_path))
    {
	if (!std::filesystem::create_directory(pool_path))
	{
	    throw std::runtime_error("Directory already exist: " + pool_name);
	}
    }

    auto it = value._data->begin_infix();
    auto it_end = value._data->end_infix();
    while (it != it_end)
    {
	auto schema_name = std::get<2>(*it);
	auto target_schema = std::get<3>(*it);

	std::filesystem::path schema_path = pool_path / schema_name;

	if (!std::filesystem::exists(schema_path))
	{
	    if (!std::filesystem::create_directory(schema_path))
	    {
		throw std::runtime_error("Could not create dir" + schema_name);
	    }
	}

	target_schema.insert_schema_to_filesystem(schema_path);
	++it;
    }
}

void data_base::insert_schema_to_filesystem(const std::string &pool_name, const std::string &schema_name, schema &&value)
{

    std::filesystem::path pool_path = _instance_path / pool_name;

    if (!std::filesystem::exists(pool_path))
    {
	if (!std::filesystem::create_directory(pool_path))
	{
	    throw std::runtime_error("Could not create pool directory: " + pool_name);
	}
    }

    std::filesystem::path schema_path = pool_path / schema_name;

    if (!std::filesystem::exists(schema_path))
    {
	if (!std::filesystem::create_directory(schema_path))
	{
	    throw std::runtime_error("Could not create schema directory: " + schema_name);
	}
    }

    value.insert_schema_to_filesystem(schema_path);
}

void data_base::insert_table_to_filesystem(const std::string &pool_name, const std::string &schema_name, const std::string &table_name, table &&value)
{
    std::filesystem::path pool_path = _instance_path / pool_name;

    if (!std::filesystem::exists(pool_path) && !std::filesystem::create_directory(pool_path))
    {
	throw std::runtime_error("Could not create pool directory: " + pool_name);
    }

    std::filesystem::path schema_path = pool_path / schema_name;

    if (!std::filesystem::exists(schema_path) && !std::filesystem::create_directory(schema_path))
    {
	throw std::runtime_error("Could not create schema directory: " + schema_name);
    }

    std::filesystem::path table_file_path = schema_path / (table_name + table::_file_format);

    try
    {
	value.insert_table_to_filesystem(table_file_path);
    }
    catch (...)
    {
	throw;
    }
}

void data_base::insert_data_to_filesystem(const std::string &pool_name, const std::string &schema_name, const std::string &table_name, const std::string &user_data_key, user_data &&value)
{
    std::filesystem::path pool_path = _instance_path / pool_name;

    if (!std::filesystem::exists(pool_path) && !std::filesystem::create_directory(pool_path))
    {
	throw std::runtime_error("Could not create pool directory: " + pool_name);
    }

    std::filesystem::path schema_path = pool_path / schema_name;

    if (!std::filesystem::exists(schema_path) && !std::filesystem::create_directory(schema_path))
    {
	throw std::runtime_error("Could not create schema directory: " + schema_name);
    }

    auto table_path = schema_path / (table_name + _file_format);
    auto index_table_path = schema_path / ("index_" + table_name + _file_format);

    table::insert_ud_to_filesystem(table_path, index_table_path, user_data_key, std::move(value));
}

void data_base::insert_schema_to_filesystem(const std::string &pool_name, const std::string &schema_name, const schema &value)
{
}

void data_base::insert_data_to_filesystem(const std::string &pool_name, const std::string &schema_name, const std::string &table_name, const std::string &user_data_key, const user_data &value)
{
    std::filesystem::path pool_path = _instance_path / pool_name;

    if (!std::filesystem::exists(pool_path) && !std::filesystem::create_directory(pool_path))
    {
	throw std::runtime_error("Could not create pool directory: " + pool_name);
    }

    std::filesystem::path schema_path = pool_path / schema_name;

    if (!std::filesystem::exists(schema_path) && !std::filesystem::create_directory(schema_path))
    {
	throw std::runtime_error("Could not create schema directory: " + schema_name);
    }

    auto table_path = schema_path / (table_name + _file_format);
    auto index_table_path = schema_path / ("index_" + table_name + _file_format);

    if (!std::filesystem::exists(table_path) || !std::filesystem::exists(index_table_path))
    {
	throw std::runtime_error("Could not insert to undefined Table" + table_path.string());
    }


    table::insert_ud_to_filesystem(table_path, index_table_path, user_data_key, value);
}

void data_base::insert_table_to_filesystem(const std::string &pool_name, const std::string &schema_name, const std::string &table_name, const table &value)
{
    std::filesystem::path pool_path = _instance_path / pool_name;

    if (!std::filesystem::exists(pool_path) || !std::filesystem::is_directory(pool_path))
    {
	throw std::runtime_error("Pool directory does not exist: " + pool_name);
    }

    std::filesystem::path schema_path = pool_path / schema_name;

    if (!std::filesystem::exists(schema_path) || !std::filesystem::is_directory(schema_path))
    {
	throw std::runtime_error("Schema directory does not exist: " + schema_path.string());
    }

    auto table_path = schema_path / (table_name + _file_format);
    auto index_table_path = schema_path / ("index_" + table_name + _file_format);
    if (!std::filesystem::exists(table_path))
    {
	table::check_and_create_empty(table_path, index_table_path);
    }
}

void data_base::add_table_to_filesystem(const std::string &pool_name, const std::string &schema_name, const std::string &table_name)
{
    std::filesystem::path pool_path = _instance_path / pool_name;

    if (!std::filesystem::exists(pool_path) || !std::filesystem::is_directory(pool_path))
    {
	throw std::runtime_error("Pool directory does not exist: " + pool_name);
    }

    std::filesystem::path schema_path = pool_path / schema_name;

    if (!std::filesystem::exists(schema_path) || !std::filesystem::is_directory(schema_path))
    {
	throw std::runtime_error("Schema directory does not exist: " + schema_path.string());
    }

    auto table_path = schema_path / (table_name + _file_format);
    auto index_table_path = schema_path / ("index_" + table_name + _file_format);
    if (!std::filesystem::exists(table_path))
    {
	table::check_and_create_empty(table_path, index_table_path);
    }
}

void data_base::add_schema_to_filesystem(const std::string &pool_name, const std::string &schema_name)
{
    std::filesystem::path pool_path = _instance_path / pool_name;

    if (!std::filesystem::exists(pool_path) || !std::filesystem::is_directory(pool_path))
    {
	throw std::runtime_error("Pool directory does not exist: " + pool_name);
    }

    std::filesystem::path schema_path = pool_path / schema_name;

    if (!std::filesystem::exists(schema_path))
    {
	if (!std::filesystem::create_directory(schema_path))
	{
	    throw std::runtime_error("Could not create schema directory: " + schema_name);
	}
    }
}

void data_base::add_pool_to_filesystem(const std::string &pool_name)
{
    std::filesystem::path pool_path = _instance_path / pool_name;

    if (!std::filesystem::exists(pool_path) && !std::filesystem::is_directory(pool_path))
    {
	if (!std::filesystem::create_directory(pool_path))
	{
	    throw std::runtime_error("Could not create schema directory: " + pool_path.string());
	}
    }
}

user_data data_base::obtain_data_in_filesystem(const std::string &pool_name, const std::string &schema_name, const std::string &table_name, const std::string &ud_key)
{
    std::filesystem::path pool_path = _instance_path / pool_name;

    if (!std::filesystem::exists(pool_path) || !std::filesystem::is_directory(pool_path))
    {
	throw std::runtime_error("Could not find pool: " + pool_name);
    }

    std::filesystem::path schema_path = pool_path / schema_name;

    if (!std::filesystem::exists(schema_path) || !std::filesystem::is_directory(schema_path))
    {
	throw std::runtime_error("Could not find schema: " + schema_name);
    }

    auto table_path = schema_path / (table_name + _file_format);
    auto index_table_path = schema_path / ("index_" + table_name + _file_format);

    if (!std::filesystem::exists(table_path) || !std::filesystem::exists(index_table_path))
    {
	throw std::runtime_error("Could not obtain in undefined Table" + table_path.string());
    }

    auto ud = storage_interface::obtain_in_filesystem(table_path, index_table_path, ud_key);

    return ud;
}

void data_base::update_ud_in_filesystem(const std::string &pool_name, const std::string &schema_name, const std::string &table_name, const std::string &user_data_key, user_data &&value)
{
    std::filesystem::path pool_path = _instance_path / pool_name;

    if (!std::filesystem::exists(pool_path) || !std::filesystem::is_directory(pool_path))
    {
	throw std::runtime_error("Could not find pool directory: " + pool_name);
    }

    std::filesystem::path schema_path = pool_path / schema_name;

    if (!std::filesystem::exists(schema_path) || !std::filesystem::is_directory(schema_path))
    {
	throw std::runtime_error("Could not find schema directory: " + schema_name);
    }

    auto table_path = schema_path / (table_name + _file_format);
    auto index_table_path = schema_path / ("index_" + table_name + _file_format);

    if (!std::filesystem::exists(table_path) || !std::filesystem::exists(index_table_path))
    {
	throw std::runtime_error("Could not obtain in undefined Table" + table_path.string());
    }

    table::update_ud_in_filesystem(table_path, index_table_path, user_data_key, std::move(value));
}

void data_base::update_ud_in_filesystem(const std::string &schemas_pool_name, const std::string &schema_name, const std::string &table_name, const std::string &user_data_key, const user_data &value)
{
}

void data_base::dispose_pool_in_filesystem(const std::string &pool_name)
{
    std::filesystem::path pool_path = _instance_path / pool_name;

    if (std::filesystem::exists(pool_path) && std::filesystem::is_directory(pool_path))
    {
	try
	{

	    std::filesystem::remove_all(pool_path);
	}
	catch (const std::filesystem::filesystem_error &e)
	{

	    throw std::runtime_error("Could not dispose pool directory: " + pool_path.string() + " Error: " + e.what());
	}
    }
}

void data_base::dispose_schema_in_filesystem(const std::string &pool_name, const std::string &schema_name)
{
    std::filesystem::path pool_path = _instance_path / pool_name;

    if (!std::filesystem::exists(pool_path) || !std::filesystem::is_directory(pool_path))
    {
	throw std::runtime_error("Pool directory does not exist: " + pool_name);
    }

    std::filesystem::path schema_path = pool_path / schema_name;

    if (std::filesystem::exists(schema_path) && std::filesystem::is_directory(schema_path))
    {
	try
	{
	    std::filesystem::remove_all(schema_path);
	}
	catch (...)
	{
	    throw std::runtime_error("Could not dispose schema directory: " + schema_name);
	}
    }
}

void data_base::dispose_table_in_filesystem(const std::string &schemas_pool_name, const std::string &schema_name, const std::string &table_name)
{
    std::filesystem::path pool_path = _instance_path / schemas_pool_name;

    if (!std::filesystem::exists(pool_path) || !std::filesystem::is_directory(pool_path))
    {
	throw std::runtime_error("Could not find pool directory: " + schemas_pool_name);
    }

    std::filesystem::path schema_path = pool_path / schema_name;

    if (!std::filesystem::exists(schema_path) || !std::filesystem::is_directory(schema_path))
    {
	throw std::runtime_error("Could not find schema directory: " + schema_name);
    }

    auto table_path = schema_path / (table_name + _file_format);
    auto index_table_path = schema_path / ("index_" + table_name + _file_format);

    if (!std::filesystem::exists(table_path))
    {
	throw std::runtime_error("Table file does not exist: " + table_path.string());
    }

    if (!std::filesystem::exists(index_table_path))
    {
	throw std::runtime_error("Index file does not exist: " + index_table_path.string());
    }

    create_backup(table_path);
    create_backup(index_table_path);
    try
    {
	std::filesystem::remove(table_path);
	std::filesystem::remove(index_table_path);
	delete_backup(table_path);
	delete_backup(index_table_path);
    }
    catch (...)
    {
	storage_interface::load_backup(table_path);
	storage_interface::load_backup(index_table_path);
	throw;
    }
}

std::map<std::string, user_data> data_base::obtain_between_ud_in_filesystem(
	std::string const &pool_name,
	std::string const &schema_name,
	std::string const &table_name,
	std::string const &lower_bound,
	std::string const &upper_bound,
	bool lower_bound_inclusive,
	bool upper_bound_inclusive)
{
    std::filesystem::path pool_path = _instance_path / pool_name;

    if (!std::filesystem::exists(pool_path) || !std::filesystem::is_directory(pool_path))
    {
	throw std::runtime_error("Could not find pool directory: " + pool_name);
    }

    std::filesystem::path schema_path = pool_path / schema_name;

    if (!std::filesystem::exists(schema_path) || !std::filesystem::is_directory(schema_path))
    {
	throw std::runtime_error("Could not find schema directory: " + schema_name);
    }

    auto table_path = schema_path / (table_name + _file_format);
    auto index_table_path = schema_path / ("index_" + table_name + _file_format);

    if (!std::filesystem::exists(table_path))
    {
	throw std::runtime_error("Table file does not exist: " + table_path.string());
    }

    if (!std::filesystem::exists(index_table_path))
    {
	throw std::runtime_error("Index file does not exist: " + index_table_path.string());
    }

    return table::obtain_between_ud_in_filesystem(table_path, index_table_path, lower_bound, upper_bound, lower_bound_inclusive, upper_bound_inclusive);
}

void data_base::dispose_ud_in_filesystem(const std::string &pool_name, const std::string &schema_name, const std::string &table_name, const std::string &user_data_key)
{
    std::filesystem::path pool_path = _instance_path / pool_name;

    if (!std::filesystem::exists(pool_path) || !std::filesystem::is_directory(pool_path))
    {
	throw std::runtime_error("Could not find pool directory: " + pool_name);
    }

    std::filesystem::path schema_path = pool_path / schema_name;

    if (!std::filesystem::exists(schema_path) || !std::filesystem::is_directory(schema_path))
    {
	throw std::runtime_error("Could not find schema directory: " + schema_name);
    }

    auto table_path = schema_path / (table_name + _file_format);
    auto index_table_path = schema_path / ("index_" + table_name + _file_format);

    if (!std::filesystem::exists(table_path) || !std::filesystem::exists(index_table_path))
    {
	throw std::runtime_error("Could not obtain in undefined Table" + table_path.string());
    }

    table::dispose_ud_from_filesystem(table_path, index_table_path, user_data_key);
}

void data_base::execute_command_from_file(const std::string &filename)
{
    std::ifstream in_file(filename);
    if (!in_file.is_open())
    {
	throw std::runtime_error("Cannot open file: " + filename);
    }

    std::string command;
    while (std::getline(in_file, command))
    {
	std::istringstream iss(command);
	std::string action, pool_name, schema_name, table_name, user_data_key, data;
	std::string lower_bound, upper_bound;
	bool lower_inclusive = false, upper_inclusive = false;

	getline(iss, action, ':');
	if (action == "insert_pool")
	{
	    getline(iss, pool_name, ':');
	    schemas_pool pool;
	    insert_schemas_pool(pool_name, std::move(pool));
	}
	else if (action == "insert_schema")
	{
	    getline(iss, pool_name, ':');
	    getline(iss, schema_name, ':');
	    schema schm;
	    insert_schema(pool_name, schema_name, std::move(schm));
	}
	else if (action == "insert_table")
	{
	    getline(iss, pool_name, ':');
	    getline(iss, schema_name, ':');
	    getline(iss, table_name, ':');
	    table tbl;
	    insert_table(pool_name, schema_name, table_name, std::move(tbl));
	}
	else if (action == "insert_data")
	{
	    getline(iss, pool_name, ':');
	    getline(iss, schema_name, ':');
	    getline(iss, table_name, ':');
	    getline(iss, user_data_key, ':');
	    std::getline(iss, data);
	    user_data ud;
	    ud.create_user_data_from_str(data, ',');
	    //TODO: !!!!!!!!!!!!create ud
	    insert_data(pool_name, schema_name, table_name, user_data_key, ud);
	}
	else if (action == "obtain_data")
	{
	    getline(iss, pool_name, ':');
	    getline(iss, schema_name, ':');
	    getline(iss, table_name, ':');
	    getline(iss, user_data_key, ':');
	    user_data ud = obtain_data(pool_name, schema_name, table_name, user_data_key);
	}
	else if (action == "obtain_between_data")
	{
	    getline(iss, pool_name, ':');
	    getline(iss, schema_name, ':');
	    getline(iss, table_name, ':');
	    getline(iss, lower_bound, ':');
	    getline(iss, upper_bound, ':');
	    iss >> lower_inclusive;
	    iss.ignore(1);
	    iss >> upper_inclusive;

	    auto data_range = obtain_between_data(pool_name, schema_name, table_name, lower_bound, upper_bound, lower_inclusive, upper_inclusive);
	    //TODO: output data;
	}
	else if (action == "update_data")
	{
	    getline(iss, pool_name, ':');
	    getline(iss, schema_name, ':');
	    getline(iss, table_name, ':');
	    getline(iss, user_data_key, ':');
	    std::getline(iss, data);
	    user_data updated_ud;
	    updated_ud.create_user_data_from_str(data, ',');
	    update_data(pool_name, schema_name, table_name, user_data_key, std::move(updated_ud));
	}
	else if (action == "dispose_pool")
	{
	    getline(iss, pool_name, ':');
	    dispose_schemas_pool(pool_name);
	}
	else if (action == "dispose_schema")
	{
	    getline(iss, pool_name, ':');
	    getline(iss, schema_name, ':');
	    dispose_schema(pool_name, schema_name);
	}
	else if (action == "dispose_table")
	{
	    getline(iss, pool_name, ':');
	    getline(iss, schema_name, ':');
	    getline(iss, table_name, ':');
	    dispose_table(pool_name, schema_name, table_name);
	}
	else if (action == "dispose_data")
	{
	    getline(iss, pool_name, ':');
	    getline(iss, schema_name, ':');
	    getline(iss, table_name, ':');
	    getline(iss, user_data_key, ':');
	    dispose_user_data(pool_name, schema_name, table_name, user_data_key);
	}
	else
	{
	    std::cerr << "Unknown command: " << action << std::endl;
	}
    }

    in_file.close();
}

void data_base::start_console_dialog()
{
    std::string command;
    int uncknown_command_counter = 0;
    std::cout << "Session started" << std::endl;
    while (true)
    {
	std::cout << "Input operation from list: \n[1] insert_pool; [2] insert_schema; [3] insert_table \n[4] dispose_pool; "
		  << "[5] dispose_schema; [6] dispose_table \n[7] insert_data; [8] dispose_data; [9] obtain_data\n[10] obtain_between_data "
		  << "[11] update_data [12] exit\n[13] execute_file_command\n[14] save_db_state\n[15] load_db_state\n[16] obtain_all\n(Please, input only text command)\n"
		  << std::endl;

	std::cin >> command;
	if (command.empty())
	{
	    std::cout << "Request could not be an empty !" << std::endl;
	    continue;
	}

	if (uncknown_command_counter > 4)
	{
	    std::cerr << "Suspicious activity. Too many wrong commands." << std::endl;
	    break;
	}

	if (command == "exit")
	{
	    break;
	}

	std::istringstream iss(command);
	std::string action;

	getline(iss, action, ':');
	if (action == "insert_pool")
	{
	    std::string pool_name;
	    std::cout << "Enter the name of the schema pool: ";
	    std::cin >> pool_name;
	    try
	    {
		schemas_pool pool;
		insert_schemas_pool(pool_name, std::move(pool));
		std::cout << "\nSchema pool added successfully.\n"
			  << std::endl;
	    }
	    catch (const std::exception &e)
	    {
		std::cerr << "Error inserting schema pool: " << e.what() << std::endl;
	    }
	}
	else if (action == "insert_schema")
	{
	    std::string pool_name, schema_name;
	    std::cout << "Enter the name of the schema pool: ";
	    std::cin >> pool_name;
	    std::cout << "Enter the schema name: ";
	    std::cin >> schema_name;
	    try
	    {
		schema schm;
		insert_schema(pool_name, schema_name, std::move(schm));
		std::cout << "\nSchema added successfully.\n"
			  << std::endl;
	    }
	    catch (const std::exception &e)
	    {
		std::cerr << "Error inserting schema: " << e.what() << std::endl;
	    }
	}
	else if (action == "insert_table")
	{
	    std::string pool_name, schema_name, table_name;
	    read_path_to_table(pool_name, schema_name, table_name);
	    try
	    {
		table tbl;
		insert_table(pool_name, schema_name, table_name, std::move(tbl));
		std::cout << "\nTable added successfully.\n"
			  << std::endl;
	    }
	    catch (const std::exception &e)
	    {
		std::cerr << "Error inserting table: " << e.what() << std::endl;
	    }
	}
	else if (action == "dispose_pool")
	{
	    std::string pool;
	    std::cout << "Enter the name of the schema pool to delete: ";
	    std::cin >> pool;
	    try
	    {
		dispose_schemas_pool(pool);
		std::cout << "\nSchema pool removed successfully.\n"
			  << std::endl;
	    }
	    catch (const std::exception &e)
	    {
		std::cerr << "Error removing schema pool: " << e.what() << std::endl;
	    }
	}
	else if (action == "insert_data")
	{
	    std::string pool_name, schema_name, table_name, user_data_key, data;
	    read_path_to_table_and_key(pool_name, schema_name, table_name, user_data_key);
	    std::string str_id, str_name, str_surname;
	    read_user_data(str_id, str_name, str_surname);

	    try
	    {
		user_data ud(std::stol(str_id), str_name, str_surname);
		insert_data(pool_name, schema_name, table_name, user_data_key, std::move(ud));
		std::cout << "\nData has been successfully added.\n"
			  << std::endl;
	    }
	    catch (const std::exception &e)
	    {
		std::cerr << "Error inserting data: " << e.what() << std::endl;
	    }
	}
	else if (action == "obtain_all")
	{
	    if (this->get_strategy() != storage_strategy::filesystem)
	    {
		std::cerr << "Incorrect strategy for this operation" << std::endl;
		continue;
	    }
	    std::string pool_name, schema_name, table_name;
	    read_path_to_table(pool_name, schema_name, table_name);
	    try
	    {
		std::filesystem::path pool_path = _instance_path / pool_name;

		if (!std::filesystem::exists(pool_path) && !std::filesystem::create_directory(pool_path))
		{
		    throw std::runtime_error("Could not create pool directory: " + pool_name);
		}

		std::filesystem::path schema_path = pool_path / schema_name;

		if (!std::filesystem::exists(schema_path) && !std::filesystem::create_directory(schema_path))
		{
		    throw std::runtime_error("Could not create schema directory: " + schema_name);
		}

		auto table_path = schema_path / (table_name + _file_format);
		auto index_table_path = schema_path / ("index_" + table_name + _file_format);

		auto res = table::obtain_all_ud_in_filesystem(table_path, index_table_path);
		std::cout << '\n';
		for (auto const &item: res)
		{
		    std::cout << item.first << " value: " << item.second.to_string() << std::endl;
		}
		std::cout << '\n';
		std::cout << "All data was obtaining successfully" << std::endl;
	    }
	    catch (const std::exception &e)
	    {
		std::cerr << "Error obtaining all data: " << e.what() << std::endl;
	    }
	}
	else if (action == "obtain_data")
	{
	    std::string pool_name, schema_name, table_name, user_data_key;
	    read_path_to_table_and_key(pool_name, schema_name, table_name, user_data_key);
	    try
	    {
		user_data ud = obtain_data(pool_name, schema_name, table_name, user_data_key);
		std::cout << "\nData obtained: " << ud.to_string() << "\n";
		std::cout << std::endl;
	    }
	    catch (const std::exception &e)
	    {
		std::cerr << "Error obtaining data: " << e.what() << std::endl;
	    }
	}
	else if (action == "obtain_between_data")
	{
	    std::string pool_name, schema_name, table_name, lower_bound, upper_bound;
	    std::string lower_inclusive, upper_inclusive;
	    read_path_to_table(pool_name, schema_name, table_name);
	    std::cout << "Enter the lower bound: ";
	    std::cin >> lower_bound;
	    std::cout << "Inclusive? (1 - yes, 0 - no): ";
	    std::cin >> lower_inclusive;
	    std::cin.ignore();
	    throw_if_str_not_bool(lower_inclusive);
	    std::cout << "Enter the upper bound: ";
	    std::cin >> upper_bound;
	    std::cout << "Inclusive? (1 - yes, 0 - no): ";
	    std::cin >> upper_inclusive;
	    std::cin.ignore();
	    throw_if_str_not_bool(upper_inclusive);

	    try
	    {
		auto data_range = obtain_between_data(pool_name, schema_name, table_name, lower_bound, upper_bound,
						      lower_inclusive == "1", upper_inclusive == "1");
		std::cout << "Range data obtained:\n";
		for (const auto &ud: data_range)
		{
		    std::cout << "key: " << ud.first << "; value: " << ud.second.to_string() << std::endl;
		}
	    }
	    catch (const std::exception &e)
	    {
		std::cerr << "Error obtaining data in range: " << e.what() << std::endl;
	    }
	}
	else if (action == "update_data")
	{
	    std::string pool_name, schema_name, table_name, user_data_key, data;
	    read_path_to_table_and_key(pool_name, schema_name, table_name, user_data_key);
	    std::string str_id, str_name, str_surname;
	    read_user_data(str_id, str_name, str_surname);

	    try
	    {
		user_data ud(std::stol(str_id), str_name, str_surname);
		update_data(pool_name, schema_name, table_name, user_data_key, std::move(ud));
		std::cout << "Data updated successfully." << std::endl;
	    }
	    catch (const std::exception &e)
	    {
		std::cerr << "Error updating data: " << e.what() << std::endl;
	    }
	}
	else if (action == "dispose_schema")
	{
	    std::string pool_name, schema_name;
	    std::cout << "Enter the name of the schema pool: ";
	    std::cin >> pool_name;
	    std::cout << "Enter the name of the schema to delete: ";
	    std::cin >> schema_name;
	    if (pool_name.empty() || schema_name.empty())
	    {
		std::cerr << "The schema pool name and schema name cannot be empty." << std::endl;
		continue;
	    }
	    try
	    {
		dispose_schema(pool_name, schema_name);
		std::cout << "Schema removed successfully." << std::endl;
	    }
	    catch (const std::exception &e)
	    {
		std::cerr << "Error removing schema: " << e.what() << std::endl;
	    }
	}
	else if (action == "dispose_table")
	{
	    std::string pool_name, schema_name, table_name;
	    read_path_to_table(pool_name, schema_name, table_name);
	    if (pool_name.empty() || schema_name.empty() || table_name.empty())
	    {
		std::cerr << "The names of the schema pool, schema, and table cannot be empty." << std::endl;
		continue;
	    }
	    try
	    {
		dispose_table(pool_name, schema_name, table_name);
		std::cout << "Table removed successfully." << std::endl;
	    }
	    catch (const std::exception &e)
	    {
		std::cerr << "Error removing table: " << e.what() << std::endl;
	    }
	}
	else if (action == "dispose_data")
	{
	    std::string pool_name, schema_name, table_name, user_data_key;
	    read_path_to_table_and_key(pool_name, schema_name, table_name, user_data_key);
	    try
	    {
		dispose_user_data(pool_name, schema_name, table_name, user_data_key);
		std::cout << "Data removed successfully." << std::endl;
	    }
	    catch (const std::exception &e)
	    {
		std::cerr << "Error removing data: " << e.what() << std::endl;
	    }
	}
	else if (action == "save_db_state")
	{
	    try
	    {
		save_data_base_state();
		std::cout << "\nData base state saved successfully.\n"
			  << std::endl;
	    }
	    catch (std::exception const &e)
	    {
		std::cerr << "Error saving state: " << e.what() << std::endl;
	    }
	}
	else if (action == "load_db_state")
	{
	    try
	    {
		load_data_base_state();
		std::cout << "\nData base state load successfully.\n"
			  << std::endl;
	    }
	    catch (std::exception const &e)
	    {
		std::cerr << "Error saving state: " << e.what() << std::endl;
	    }
	}
	else if (action == "execute_file_command")
	{
	    std::string filename;
	    std::cout << "Enter the path to command filename: " << std::endl;
	    std::cin >> filename;
	    try
	    {
		this->execute_command_from_file(filename);
		std::cout << "\nOperation finished successfully\n"
			  << std::endl;
	    }
	    catch (std::exception const &e)
	    {
		std::cerr << "Execution command file faied: " << e.what() << std::endl;
	    }
	}
	else
	{
	    ++uncknown_command_counter;
	    std::cerr << "Unknown command: " << action << std::endl;
	}
    }
    std::cout << "Session finished." << std::endl;
}
void data_base::read_path_to_table(std::string &pool_name, std::string &schema_name, std::string &table_name)
{
    std::cout << "Enter the schema pool name: ";
    std::cin >> pool_name;
    std::cout << "Enter the schema name: ";
    std::cin >> schema_name;
    std::cout << "Enter the table name: ";
    std::cin >> table_name;
}

void data_base::read_path_to_table_and_key(std::string &pool_name, std::string &schema_name, std::string &table_name, std::string &ud_key)
{
    read_path_to_table(pool_name, schema_name, table_name);
    std::cout << "Enter the user data key: ";
    std::cin >> ud_key;
}

void data_base::read_user_data(std::string &str_id, std::string &str_name, std::string &str_surname)
{
    std::cout << "Enter the id: ";
    std::cin >> str_id;
    std::cout << "Enter the name: ";
    std::cin >> str_name;
    std::cout << "Enter the surname: ";
    std::cin >> str_surname;
}

void data_base::throw_if_str_not_bool(std::string const &str)
{
    if (str.length() != 1 || (str[0] != '0' && str[0] != '1'))
    {
	throw std::logic_error("Invalid string for cast to bool value! There must be only 1 or 0. ");
    }
}