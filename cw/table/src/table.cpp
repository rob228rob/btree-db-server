//
// Created by rob22 on 04.05.2024.
//

#include "../include/table.h"
#include "../../common/include/storage_interface.h"
#include <fstream>
#include <functional>
#include <sstream>

std::function<int(const std::string &, const std::string &)> table::_default_string_comparer
	= [](const std::string &a, const std::string &b) -> int { return a.compare(b); };

table::table() :_data(std::make_unique<b_tree<std::string, user_data>>(4, _default_string_comparer, nullptr, nullptr))
{
    set_instance_name("table_name");

    this->_storaged_strategy = storage_interface<std::string, user_data>::data_storage_strategy::in_memory_storaged;

    _storage_filename = "C:\\Users\\rob22\\CLionProjects\\cw_os\\cw\\table.txt";

    if (this->_storaged_strategy == storage_interface<std::string, user_data>::data_storage_strategy::filesystem_storaged)
    {
	trace_with_guard("data was deserialized");
	deserialize();
    }
}

table::table(
	std::size_t t,
	allocator *allocator,
	logger *logger,
	const std::function<int(const std::string &, const std::string &)>& keys_comparer,
	data_storage_strategy storage_strategy,
	std::string instance_name)
    : _data(std::make_unique<b_tree<std::string, user_data>>(t, keys_comparer, allocator, logger))
{
    set_instance_name(instance_name);
    //TODO: add TableID and generate filename with uid
    _storage_filename = "C:\\Users\\rob22\\CLionProjects\\cw_os\\cw\\table.txt";

    set_strategy(storage_strategy);

    if (this->_storaged_strategy == storage_interface<std::string, user_data>::data_storage_strategy::filesystem_storaged)
    {
	trace_with_guard("data was deserialized");
	table::deserialize();
    }
}

table::~table()
{

    if (this->_storaged_strategy == storage_interface<std::string, user_data>::data_storage_strategy::filesystem_storaged)
    {
	trace_with_guard("data was serialized after desctructor called");
	table::serialize();
    }
}

table::table(const table &other)
{
    if (other._data)
    {
	_data = std::make_unique<b_tree<std::string, user_data>>(*other._data);
    } else
    {
	_data = nullptr;
    }
}

table::table(table &&other) noexcept : _data(std::move(other._data))
{

}

table &table::operator=(const table &other)
{
    if (this != &other)
    {
	if (other._data)
	{
	    _data = std::make_unique<b_tree<std::string, user_data>>(*other._data);
	}
	else
	{
	    _data.reset();
	}
    }
    return *this;
}

table &table::operator=(table &&other) noexcept
{
    if (this != &other)
    {
	_data = std::move(other._data);
    }
    return *this;
}


void table::insert(const std::string &key, const user_data &value)
{
    try
    {
	_data->insert(key, (value));
	++_table_uid;
    }
    catch (std::exception const &e)
    {
	error_with_guard("can't insert an existed key: [ " + key + " ]");
	throw;
    }
}

void table::insert(const std::string &key, user_data &&value)
{
    try
    {
	_data->insert(key, std::move(value));
	++_table_uid;
    }
    catch (std::exception const &e)
    {
	error_with_guard("can't insert an existed key: [ " + key + " ]");
	throw;
    }
}

const user_data &table::obtain(const std::string &key)
{
    try
    {
	return _data->obtain(key);
    }
    catch (std::exception const &e)
    {
	error_with_guard("key not found: [ " + key + " ]");
	throw;
    }
}

std::vector<typename associative_container<std::string, user_data>::key_value_pair> table::obtain_between(
	const std::string &lower_bound,
	const std::string &upper_bound,
	bool lower_bound_inclusive,
	bool upper_bound_inclusive)
{
    return _data->obtain_between(lower_bound, upper_bound, lower_bound_inclusive, upper_bound_inclusive);
}

void table::dispose(const std::string &key)
{
    try
    {
	_data->dispose(key);
    }
    catch (std::exception const &e)
    {
	warning_with_guard("key not found: [ " + key + " ]");
	throw;
    }
}

void table::serialize()
{
    std::ofstream output_file(_storage_filename);
    if (!output_file.is_open())
    {
	error_with_guard("file for serializing did not open! file_name: [ " + _storage_filename + " ]");
	return;
    }
    std::cout << "sigma\n";
    auto it = _data->begin_infix();
    auto it_end = _data->end_infix();
    while (it != it_end)
    {
	auto string_key = std::get<2>(*it);
	auto user_data_value = std::get<3>(*it);

	output_file << string_key << "#" << user_data_value.get_id() << "#" << user_data_value.get_name() << "#" << user_data_value.get_surname() << "|" << std::endl;
	++it;
    }

    output_file.close();
}

void table::deserialize()
{
    std::ifstream input_file(_storage_filename);
    if (!input_file.is_open())
    {
	error_with_guard("file for deserializing did not open! file_name: [ " + _storage_filename + " ]");
	return;
    }

    std::string line;
    while (std::getline(input_file, line))
    {
	if (line.empty() || line.back() != '|')
	    continue;

	line.pop_back();

	std::istringstream line_stream(line);

	std::string segment;

	std::vector<std::string> seg_list;

	while (std::getline(line_stream, segment, '#'))
	{
	    seg_list.push_back(segment);
	}

	if (seg_list.size() != 4)
	{
	    //TODO: handle error
	    continue;
	}

	std::string string_key = seg_list[0];
	size_t id = std::stol(seg_list[1]);
	std::string name = seg_list[2];
	std::string surname = seg_list[3];

	user_data ud(id, name, surname);

	_data->insert(string_key, std::move(ud));
    }

    input_file.close();
}

void table::set_storage_filename(std::string &filename)
{
    this->_storage_filename = filename;
}

void table::set_storage_filename(std::string &&filename)
{
    this->_storage_filename = std::move(filename);
}

void table::print_table()
{
    auto it = _data->begin_infix();
    auto it_end  = _data->end_infix();

    while (it != it_end)
    {
	user_data ud = std::get<3>(*it);
	std::cout << std::get<2>(*it) << " " << ud.get_name() << " " << ud.get_surname()<< " " << ud.get_id() << " " << std::endl;
	++it;
    }
}

table table::load_data_from_filesystem(std::string const &filename)
{
    table src_table = table();

    std::ifstream input_file(filename);
    if (!input_file.is_open())
    {
	error_with_guard("file for deserializing did not open! file_name: [ " + filename + " ]");
	return src_table;
    }

    std::string line;
    while (std::getline(input_file, line))
    {
	if (line.empty() || line.back() != '|')
	    continue;
	line.pop_back();

	std::istringstream line_stream(line);
	std::string segment;
	std::vector<std::string> seg_list;

	while (std::getline(line_stream, segment, '#'))
	{
	    seg_list.push_back(segment);
	}

	if (seg_list.size() != 4)
	{
	    //TODO: handle error
	    continue;
	}

	std::string string_key = seg_list[0];
	size_t id = std::stol(seg_list[1]);
	std::string name = seg_list[2];
	std::string surname = seg_list[3];

	src_table.insert(string_key, user_data(id, name, surname));
    }

    input_file.close();
    return src_table;
}
void table::save_data_to_filesystem(const std::string &filename)
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
	auto user_data_value = std::get<3>(*it);

	output_file << string_key << "#" << user_data_value.get_id() << "#" << user_data_value.get_name() << "#" << user_data_value.get_surname() << "|" << std::endl;
	++it;
    }

    output_file.close();
}
