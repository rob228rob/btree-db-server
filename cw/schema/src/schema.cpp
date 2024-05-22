//
// Created by rob22 on 05.05.2024.
//

#include "../include/schema.h"
#include "../../table/include/table.h"
#include <fstream>
#include <functional>
#include <sstream>


std::function<int(const std::string &, const std::string &)> schema::_default_string_comparer = [](const std::string &a, const std::string &b) -> int { return a.compare(b); };

schema::schema() : _data(std::make_unique<b_tree<std::string, table>>(4,_default_string_comparer, nullptr, nullptr))
{
    this->_storaged_strategy = storage_interface<std::string, table>::storage_strategy::in_memory;
    this->_logger = nullptr;
    _storage_filename = "C:\\Users\\rob22\\CLionProjects\\cw_os\\cw\\table.txt";

    if (this->_storaged_strategy == storage_interface<std::string, table>::storage_strategy::filesystem)
    {
	deserialize();
    }
}

schema::schema(
	std::size_t t,
	allocator *allocator,
	logger *logger,
	const std::function<int(const std::string &, const std::string &)> &keys_comparer,
	storage_strategy storage_strategy)
    : _data(std::make_unique<b_tree<std::string, table>>(t, keys_comparer, allocator, logger))
{
    //TODO: add TableID and generate filename with uid
    _storage_filename = "C:\\Users\\rob22\\CLionProjects\\cw_os\\cw\\schema.txt";
    this->_logger = nullptr;
    set_strategy(storage_strategy);

    if (this->_storaged_strategy == storage_interface<std::string, table>::storage_strategy::filesystem)
    {
	schema::deserialize();
    }
}

schema::~schema()
{

    if (this->_storaged_strategy == storage_interface<std::string, table>::storage_strategy::filesystem)
    {
	schema::serialize();
    }
}

schema::schema(const schema &other)
{
    if (other._data)
    {
	_data = std::make_unique<b_tree<std::string, table>>(*other._data);
    }
    else
    {
	_data = nullptr;
    }
}


schema::schema(schema &&other) noexcept : _data(std::move(other._data))
{
}

schema &schema::operator=(const schema &other)
{
    if (this != &other)
    {
	if (other._data)
	{
	    _data = std::make_unique<b_tree<std::string, table>>(*other._data);
	}
	else
	{
	    _data.reset();
	}
    }
    return *this;
}

schema &schema::operator=(schema &&other) noexcept
{
    if (this != &other)
    {
	_data = std::move(other._data);
    }
    return *this;
}


void schema::insert(const std::string &key, const table &value)
{
    try
    {
	_data->insert(key, (value));
    }
    catch (std::exception const &e)
    {
	error_with_guard("key duplicate: [ " + key + " ]");
	throw;
    }
}

void schema::insert(const std::string &key, table &&value)
{
    try
    {
	_data->insert(key, std::move(value));
    }
    catch (std::exception const &e)
    {
	error_with_guard("key duplicate: [ " + key + " ]");
	throw;
    }
}

table &schema::obtain(const std::string &key)
{
    try
    {
	return _data->obtain(key);
    }
    catch (std::exception const &e)
    {
	throw;
    }
}

std::map<std::string, table> schema::obtain_between(
	const std::string &lower_bound,
	const std::string &upper_bound,
	bool lower_bound_inclusive,
	bool upper_bound_inclusive)
{
    auto vec = _data->obtain_between(lower_bound, upper_bound, lower_bound_inclusive, upper_bound_inclusive);
    std::map<std::string, table> result_map;

    for (auto &item: vec)
    {
	result_map.emplace(item.key, item.value);
    }

    return result_map;
}

void schema::update(const std::string &key, const table &value)
{
    _data->update(key, value);
}

void schema::update(const std::string &key, table &&value)
{
    _data->update(key, value);
}

void schema::dispose(const std::string &key)
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

void schema::serialize()
{

}

void schema::deserialize()
{

}

void schema::set_storage_filename(std::string &filename)
{
    this->_storage_filename = filename;
}

void schema::set_storage_filename(std::string &&filename)
{
    this->_storage_filename = std::move(filename);
}

void schema::print_table()
{
    //    auto it = _data->begin_infix();
    //    auto it_end  = _data->end_infix();
    //
    //    while (it != it_end)
    //    {
    //	user_data ud = std::get<3>(*it);
    //	std::cout << std::get<2>(*it) << " " << ud.get_name() << " " << ud.get_surname()<< " " << ud.get_id() << " " << std::endl;
    //	++it;
    //    }
}

/*
 * TODO: Create logic to load-save META-data from class besides _data(b-tree)
 */
schema schema::load_schema_from_filesystem(const std::string &filename)
{
    schema result;

    std::ifstream input_file(filename);
    if (!input_file.is_open())
    {
	error_with_guard("file for deserializing did not open! file_name: [ " + filename + " ]");
	throw std::logic_error("file did not open, filename: " + filename);
    }

    std::string line;
    while (std::getline(input_file, line))
    {
	if (line.empty() || line.back() != '|')
	{
	    continue;
	}
	line.pop_back();
	std::string table_filename = line + this->_file_format;
	table new_table = table();
	new_table.load_data_from_filesystem(table_filename);
	result.insert(line, new_table);
    }

    input_file.close();
    return result;
}
/*
 * TODO: Create logic to load-save META-data from class besides _data(b-tree)
 */
void schema::save_schema_to_filesystem(const std::string &filename)
{
    std::string filename_copy = filename.length() == 0 ? _storage_filename : filename;

    std::ofstream output_file(filename_copy);
    if (!output_file.is_open())
    {
	error_with_guard("file for serializing did not open! file_name: [ " + filename_copy + " ]");
	return;
    }

    auto it = _data->begin_infix();
    auto it_end = _data->end_infix();
    while (it != it_end)
    {
	auto string_key = std::get<2>(*it);
	auto target_table = std::get<3>(*it);

	output_file << string_key << "|" << std::endl;

	auto table_filename = string_key + schema::_file_format;
	target_table.save_data_to_filesystem(table_filename);
    	++it;
    }

    output_file.close();
}
void schema::insert_schema_to_filesystem(const std::filesystem::path &path)
{
    auto schm_it = _data->begin_infix();
    auto schm_end = _data->end_infix();
    while (schm_it != schm_end)
    {
	auto table_name = std::get<2>(*schm_it);
	auto target_tbl = std::get<3>(*schm_it);

	std::filesystem::path table_file_path = path / (table_name + _file_format);
//TODO: be cateful, i deleted file open close
	target_tbl.insert_table_to_filesystem(table_file_path);

	++schm_it;
    }
}
