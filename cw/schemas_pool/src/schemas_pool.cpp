//
// Created by rob22 on 06.05.2024.
//

#include "../include/schemas_pool.h"
#include <fstream>

std::function<int(const std::string &, const std::string &)> schemas_pool::_default_string_comparer = [](const std::string &a, const std::string &b) -> int { return a.compare(b); };


schemas_pool::schemas_pool() : _data(std::make_unique<b_tree<std::string, schema>>(4, _default_string_comparer, nullptr, nullptr))
{
    this->_logger = nullptr;
}

schemas_pool::~schemas_pool()
{
}

schemas_pool::schemas_pool(const schemas_pool &other)
    : _data(other._data ? std::make_unique<b_tree<std::string, schema>>(*other._data) : nullptr)
{
}

schemas_pool &schemas_pool::operator=(const schemas_pool &other)
{
    if (this != &other)
    {
	_data.reset(other._data ? new b_tree<std::string, schema>(*other._data) : nullptr);
    }
    return *this;
}

schemas_pool::schemas_pool(schemas_pool &&other) noexcept
    : _data(std::move(other._data))
{
}

schemas_pool &schemas_pool::operator=(schemas_pool &&other) noexcept
{
    if (this != &other)
    {
	_data = std::move(other._data);
    }
    return *this;
}

void schemas_pool::insert(const std::string &key, const schema &value)
{
    try
    {
	_data->insert(key, value);
    }
    catch (std::exception const &e)
    {
	//error_with_guard("Exception in insert: " + std::string(e.what()));
	throw;
    }
}

void schemas_pool::insert(const std::string &key, schema &&value)
{
    try
    {
	_data->insert(key, std::move(value));
    }
    catch (std::exception const &e)
    {
	//error_with_guard("Exception in insert: " + std::string(e.what()));
	throw;
    }
}

schema &schemas_pool::obtain(const std::string &key)
{
   return _data->obtain(key);
}

void schemas_pool::update(const std::string &key, const schema &value)
{
    _data->update(key, value);
}

void schemas_pool::update(const std::string &key, schema &&value)
{
    _data->update(key, value);
}


std::map<std::string, schema> schemas_pool::obtain_between(const std::string &lower_bound, const std::string &upper_bound, bool lower_bound_inclusive, bool upper_bound_inclusive)
{
    try
    {
	auto vec = _data->obtain_between(lower_bound, upper_bound, lower_bound_inclusive, upper_bound_inclusive);
    	std::map<std::string, schema> result_map;
	for (auto &item: vec)
	{
	    result_map.emplace(item.key, item.value);
	}

	return result_map;
    }
    catch (std::exception const &e)
    {
	//error_with_guard("Exception in obtain_between: " + std::string(e.what()));
	throw;
    }
}

void schemas_pool::dispose(const std::string &key)
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

schemas_pool schemas_pool::load_schemas_pool_from_filesystem(const std::string &filename)
{
    schemas_pool new_pool;

    std::ifstream input_file(filename);

    if (!input_file.is_open())
    {
	return new_pool;
    }

    std::string line;
    while (std::getline(input_file, line))
    {
	if (line.empty() || line.back() != '|')
	{
	    continue;
	}
	line.pop_back();
	std::string schema_filename = line + this->_file_format;
	schema new_schema = schema();
	new_schema.load_schema_from_filesystem(schema_filename);
	new_pool.insert(line, new_schema);
    }

    input_file.close();
    return new_pool;
}


void schemas_pool::save_schemas_pool_to_filesystem(const std::string &filename)
{
    std::string filename_copy = filename.length() == 0 ? this->_instance_name + _file_format : filename;

    std::ofstream output_file(filename_copy);
    if (!output_file.is_open())
    {
	//error_with_guard("file for serializing did not open! file_name: [ " + filename_copy + " ]");
	return;
    }

    auto it = _data->begin_infix();
    auto it_end = _data->end_infix();
    while (it != it_end)
    {
	auto string_key = std::get<2>(*it);
	auto target_schema = std::get<3>(*it);

	output_file << string_key << "|" << std::endl;

	auto schema_filename = string_key + schemas_pool::_file_format;
	target_schema.save_schema_to_filesystem(schema_filename);
	++it;
    }

    output_file.close();
}

void schemas_pool::serialize()
{
}
void schemas_pool::deserialize()
{
}
