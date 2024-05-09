//
// Created by rob22 on 06.05.2024.
//

#include "../include/data_base.h"
#include <fstream>

std::function<int(const std::string &, const std::string &)> table::_default_string_comparer = [](const std::string &a, const std::string &b) -> int { return a.compare(b); };

data_base::data_base()
    : _data(std::make_unique<b_tree<std::string, schemas_pool>>(4, _default_string_comparer, nullptr, nullptr))
{
    set_instance_name("data_base_name");
}

data_base::data_base(
	std::size_t t,
	std::string &instance_name,
	allocator *allocator,
	logger *logger, const std::function<int(const std::string &, const std::string &)> &keys_comparer,
	storage_interface<std::string, schemas_pool>::data_storage_strategy storaged_strategy) : _data(std::make_unique<b_tree<std::string, schemas_pool>>(2, _default_string_comparer, allocator, logger))
{
    set_instance_name(instance_name);
    set_strategy(storaged_strategy);
}

data_base::~data_base()
{

}

data_base::data_base(const data_base &other)
    : _data(other._data ? std::make_unique<b_tree<std::string, schemas_pool>>(*other._data) : nullptr)
{

}

data_base::data_base(data_base &&other) noexcept
    : _data(std::move(other._data))
{
}

data_base &data_base::operator=(const data_base &other)
{
    if (this != &other)
    {
	_data = other._data ? std::make_unique<b_tree<std::string, schemas_pool>>(*other._data) : nullptr;
    }
    return *this;
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
    try
    {
	_data->insert(key, (value));
	++_u_id;
    }
    catch (std::exception const &e)
    {
	throw std::logic_error("can't insert an existed key");
    }
}

void data_base::insert(const std::string &key, schemas_pool &&value)
{
    try
    {
	_data->insert(key, std::move(value));
    	++_u_id;
    }
    catch (std::exception const &e)
    {
	throw std::logic_error("can't insert an existed key");
    }
}

const schemas_pool &data_base::obtain(const std::string &key)
{
    try
    {
	return _data->obtain(key);
    }
    catch (std::exception const &e)
    {
	throw std::logic_error("key not found");
    }
}

std::vector<typename associative_container<std::string, schemas_pool>::key_value_pair> data_base::obtain_between(
	const std::string &lower_bound,
	const std::string &upper_bound,
	bool lower_bound_inclusive,
	bool upper_bound_inclusive)
{
    return _data->obtain_between(lower_bound, upper_bound, lower_bound_inclusive, upper_bound_inclusive);
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

table data_base::get_table_by_key(
	const std::string &schemas_pool_name,
	const std::string &schema_name,
	const std::string &table_name)
{
    schema schema = get_schema_by_key(schemas_pool_name, schema_name);

    table table = schema.obtain(table_name);

    return table;
}

schema data_base::get_schema_by_key(
	const std::string &schemas_pool_name,
	const std::string &schema_name)
{
    schemas_pool pool;
    try
    {
	pool = _data->obtain(schemas_pool_name);
    }
    catch (std::exception const &e)
    {
	error_with_guard("schemas_pool not found; key: [ " + schemas_pool_name + " ]");
	throw;
    }

    schema schema;
    try
    {
	schema = pool.obtain(schema_name);
    }
    catch (std::exception const &e)
    {
	error_with_guard("schemas_pool not found; key: [ " + schemas_pool_name + " ]");
	throw;
    }

    return schema;
}


void data_base::insert_schemas_pool(const std::string &schemas_pool_name, const schemas_pool &value)
{
    _data->insert(schemas_pool_name, value);
}

void data_base::insert_schemas_pool(const std::string &schemas_pool_name, const schemas_pool &&value)
{
    _data->insert(schemas_pool_name, value);
}

void data_base::insert_schema(const std::string &schemas_pool_name, const std::string &schema_name, const schema &value)
{
    schemas_pool pool;
    try
    {
	pool = _data->obtain(schemas_pool_name);
    }
    catch (std::exception const &e)
    {
	error_with_guard("schemas_pool not found; key: [ " + schemas_pool_name + " ]");
	throw;
    }

    pool.insert(schema_name, value);
}

void data_base::insert_schema(const std::string &schemas_pool_name, const std::string &schema_name, const schema &&value)
{
    schemas_pool pool;
    try
    {
	pool = _data->obtain(schemas_pool_name);
    }
    catch (std::exception const &e)
    {
	error_with_guard("schemas_pool not found; key: [ " + schemas_pool_name + " ]");
	throw;
    }

    pool.insert(schema_name, value);
}

void data_base::insert_table(
	const std::string &schemas_pool_name,
	const std::string &schema_name,
	const std::string &table_name,
	const table &value)
{
    schema schema = get_schema_by_key(schemas_pool_name, schema_name);

    schema.insert(table_name, value);
}

void data_base::insert_table(
	const std::string &schemas_pool_name,
	const std::string &schema_name,
	const std::string &table_name,
	const table &&value)
{
    schema schema = get_schema_by_key(schemas_pool_name, schema_name);

    schema.insert(table_name, value);
}

void data_base::insert_data(
	const std::string &schemas_pool_name,
	const std::string &schema_name,
	const std::string &table_name,
	const std::string &user_data_key,
	const user_data &&value)
{
    table target_table = get_table_by_key(schemas_pool_name, schema_name, table_name);

    target_table.insert(user_data_key, value);
}

void data_base::insert_data(
	const std::string &schemas_pool_name,
	const std::string &schema_name,
	const std::string &table_name,
	const std::string &user_data_key,
	const user_data &value)
{
    table target_table = get_table_by_key(schemas_pool_name, schema_name, table_name);

    target_table.insert(user_data_key, value);
}

void data_base::dispose_schemas_pool(const std::string &schemas_pool_name)
{
    _data->dispose(schemas_pool_name);
}

void data_base::dispose_schema(const std::string &schemas_pool_name, const std::string &schema_name)
{
    schemas_pool target_pool = _data->obtain(schemas_pool_name);

    target_pool.dispose(schema_name);
}

void data_base::dispose_table(const std::string &schemas_pool_name, const std::string &schema_name, const std::string &table_name)
{
    schema target_schema;
    try
    {
	target_schema = get_schema_by_key(schemas_pool_name, schema_name);
    }
    catch (std::exception const &e)
    {
	error_with_guard("can not obtain a schema with name: [ " + schema_name + " ]");
	throw;
    }

    target_schema.dispose(table_name);
}

void data_base::dispose_user_data(
	const std::string &schemas_pool_name,
	const std::string &schema_name,
	const std::string &table_name,
	const std::string &user_data_key)
{
    table target_table;
    try
    {
	target_table = get_table_by_key(schemas_pool_name, schema_name, table_name);
    }
    catch (std::exception const &e)
    {
	throw;
    }

    target_table.dispose(user_data_key);
}

void data_base::save_db_to_filesystem()
{
    //TODO: save class-state and add prefix _abs_directory;
    std::string filename = this->_instance_name + std::to_string(_u_id) + data_base::_file_format;

    std::ofstream output_file(this->_instance_name);

    if (!output_file.is_open())
    {
	error_with_guard("file for deserializing did not open! file_name: [ " + filename + " ]");
	return;
    }

    auto it = _data->begin_infix();
    auto it_end = _data->end_infix();
    while (it != it_end)
    {
	auto string_key = std::get<2>(*it);
	auto target_schemas_pool = std::get<3>(*it);

	output_file << string_key << "|" << std::endl;
	auto schemas_pool_filename = filename + data_base::_file_format;
	target_schemas_pool.save_schemas_pool_to_filesystem(string_key);
	++it;
    }

    output_file.close();
}

void data_base::load_db_from_filesystem(const std::string &filename)
{
    std::string filename_copy = filename.length() == 0 ? "data_base.txt" : filename;

    std::ifstream input_file(filename_copy);
    if (!input_file.is_open())
    {
	error_with_guard("file for deserializing did not open! file_name: [ " + filename + " ]");
	return;
    }

    std::string line;
    while (std::getline(input_file, line))
    {
	if (line.empty() || line.back() != '|')
	{
	    continue;
	}

	line.pop_back();
	std::string schemas_pool_filename = line + data_base::_file_format;
	schemas_pool pool = schemas_pool();
	pool.load_schemas_pool_from_filesystem(schemas_pool_filename);

	_data->insert(line, pool);
    }

    input_file.close();
}
