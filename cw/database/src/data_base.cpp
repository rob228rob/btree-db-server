//
// Created by rob22 on 06.05.2024.
//

#include "../include/data_base.h"
#include <fstream>
#include <iostream>
#include <filesystem>

std::function<int(const std::string &, const std::string &)> data_base::_default_string_comparer = [](const std::string &a, const std::string &b) -> int { return a.compare(b); };

data_base::data_base()
    : _data(std::make_unique<b_tree<std::string, schemas_pool>>(4, _default_string_comparer, nullptr, nullptr))
{
    std::string inst_name = "data_base";
    this->set_instance_name(inst_name);

    auto it = _instance_names.find(inst_name);
    if (it != _instance_names.end())
    {
	throw std::logic_error("duplicate instance name");
    }

    _instance_names.emplace(inst_name);

    this->_logger = nullptr;
    this->set_strategy(storage_strategy::in_memory);
}

data_base::data_base(
	std::size_t t,
	std::string &instance_name,
	allocator *allocator,
	logger *logger, const std::function<int(const std::string &, const std::string &)> &keys_comparer,
	storage_interface<std::string, schemas_pool>::storage_strategy storaged_strategy) : _data(std::make_unique<b_tree<std::string, schemas_pool>>(2, _default_string_comparer, allocator, logger))
{
    this->set_instance_name(instance_name);
    this->set_strategy(storaged_strategy);
    this->_logger = logger;

    auto it = _instance_names.find(instance_name);
    if (it != _instance_names.end())
    {
	throw std::logic_error("duplicate instance name");
    }

    _instance_names.emplace(instance_name);

    if (storaged_strategy == storage_strategy::in_memory)
    {
	load_db_from_filesystem();
    }
}

data_base::~data_base()
{
    if (this->get_strategy() == storage_strategy::in_memory)
    {
	save_db_to_filesystem();
    }
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
    _data->obtain(key);
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

table &data_base::get_table_by_key(
	const std::string &schemas_pool_name,
	const std::string &schema_name,
	const std::string &table_name)
{
    schema schema = get_schema_by_key(schemas_pool_name, schema_name);

    table table = schema.obtain(table_name);

    return table;
}

schema &data_base::get_schema_by_key(
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


    try
    {
	const schema &schema = pool.obtain(schema_name);

	return const_cast<class schema &>(schema);
    }
    catch (std::exception const &e)
    {
	error_with_guard("schemas_pool not found; key: [ " + schemas_pool_name + " ]");
	throw;
    }
}


void data_base::insert_schemas_pool(const std::string &schemas_pool_name, const schemas_pool &value)
{
    throw_if_invalid(schemas_pool_name);
    switch (this->get_strategy())
    {
	case storage_strategy::in_memory:
	    insert(schemas_pool_name, value);
	    break;
	case storage_strategy::filesystem:
	    insert_pool_to_filesystem(schemas_pool_name, value);
	    break;
    }
}

void data_base::insert_schemas_pool(const std::string &schemas_pool_name, schemas_pool &&value)
{
    throw_if_invalid(schemas_pool_name);
    switch (this->get_strategy())
    {
	case storage_strategy::in_memory:
	    insert(schemas_pool_name, std::move(value));
	    break;
	case storage_strategy::filesystem:
	    insert_pool_to_filesystem(schemas_pool_name, value);
	    break;
    }
}

void data_base::insert_schema(const std::string &schemas_pool_name, const std::string &schema_name, const schema &value)
{
    throw_if_invalid(schemas_pool_name);
    throw_if_invalid(schema_name);

    switch (this->get_strategy())
    {
	case storage_strategy::in_memory:
	{
	    auto &pool = _data->obtain(schemas_pool_name);
	    pool.insert(schema_name, value);
	}
	    break;
	case storage_strategy::filesystem:
	    break;
    }

}

void data_base::insert_schema(const std::string &schemas_pool_name, const std::string &schema_name, schema &&value)
{
    throw_if_invalid(schemas_pool_name);
    throw_if_invalid(schema_name);
    auto &pool = _data->obtain(schemas_pool_name);
    pool.insert(schema_name, value);
}

void data_base::insert_table(
	const std::string &schemas_pool_name,
	const std::string &schema_name,
	const std::string &table_name,
	const table &value)
{
    throw_if_invalid(schemas_pool_name);
    throw_if_invalid(schema_name);
    throw_if_invalid(table_name);

    switch (this->get_strategy())
    {
	case storage_strategy::in_memory:
	{
	    auto &pool = _data->obtain(schemas_pool_name);
	    auto &schm = pool.obtain(schema_name);
	    schm.insert(table_name, value);
	}
	break;
	case storage_strategy::filesystem:
	    break;
    }

}

void data_base::insert_table(
	const std::string &schemas_pool_name,
	const std::string &schema_name,
	const std::string &table_name,
	table &&value)
{
    throw_if_invalid(schemas_pool_name);
    throw_if_invalid(schema_name);
    throw_if_invalid(table_name);

    switch (this->get_strategy())
    {
	case storage_strategy::in_memory:
	{
	    auto &pool = _data->obtain(schemas_pool_name);
	    auto &schm = pool.obtain(schema_name);
	    schm.insert(table_name, value);
	}
	break;
	case storage_strategy::filesystem:
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
    throw_if_invalid(schemas_pool_name);
    throw_if_invalid(schema_name);
    throw_if_invalid(table_name);
    throw_if_invalid(user_data_key);

    switch (this->get_strategy())
    {
	case storage_strategy::in_memory:
	{
	    auto &pool = _data->obtain(schemas_pool_name);
	    auto &schm = pool.obtain(schema_name);
	    auto &tbl = schm.obtain(table_name);
	    tbl.insert(user_data_key, value);
	}
	break;
	case storage_strategy::filesystem:
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
    throw_if_invalid(schemas_pool_name);
    throw_if_invalid(schema_name);
    throw_if_invalid(table_name);
    throw_if_invalid(user_data_key);

    switch (this->get_strategy())
    {
	case storage_strategy::in_memory:
	{
	    auto &pool = _data->obtain(schemas_pool_name);
	    auto &schm = pool.obtain(schema_name);
	    auto &tbl = schm.obtain(table_name);
	    tbl.insert(user_data_key, value);
	}
	break;
	case storage_strategy::filesystem:
	    break;
    }
}

const user_data &data_base::obtain_data(const std::string &schemas_pool_name, const std::string &schema_name, const std::string &table_name, const std::string &user_data_key)
{
    throw_if_invalid(schemas_pool_name);
    throw_if_invalid(schema_name);
    throw_if_invalid(table_name);
    throw_if_invalid(user_data_key);

    switch (this->get_strategy())
    {
	case storage_strategy::in_memory: {
	    auto &pool = _data->obtain(schemas_pool_name);
	    auto &schm = pool.obtain(schema_name);
	    auto &tbl = schm.obtain(table_name);
	    const auto &data = tbl.obtain(user_data_key);
	    return data;
	}
	case storage_strategy::filesystem:
	    auto pool_key = find_key_in_file(schemas_pool_name);
	    auto sch_key = find_key_in_file(pool_key);
	    break;
    }
}

void data_base::update_data(const std::string &schemas_pool_name, const std::string &schema_name, const std::string &table_name, const std::string &user_data_key, const user_data &value)
{
    throw_if_invalid(schemas_pool_name);
    throw_if_invalid(schema_name);
    throw_if_invalid(table_name);
    throw_if_invalid(user_data_key);
    auto &pool = _data->obtain(schemas_pool_name);
    auto &schm = pool.obtain(schema_name);
    auto &tbl = schm.obtain(table_name);
    tbl.update(user_data_key, value);
}

void data_base::update_data(const std::string &schemas_pool_name, const std::string &schema_name, const std::string &table_name, const std::string &user_data_key, user_data &&value)
{
    throw_if_invalid(schemas_pool_name);
    throw_if_invalid(schema_name);
    throw_if_invalid(table_name);
    throw_if_invalid(user_data_key);
    auto &pool = _data->obtain(schemas_pool_name);
    auto &schm = pool.obtain(schema_name);
    auto &tbl = schm.obtain(table_name);
    tbl.update(user_data_key, value);
}

void data_base::dispose_schemas_pool(const std::string &schemas_pool_name)
{
    throw_if_invalid(schemas_pool_name);
    _data->dispose(schemas_pool_name);
}

void data_base::dispose_schema(const std::string &schemas_pool_name, const std::string &schema_name)
{
    throw_if_invalid(schemas_pool_name);
    throw_if_invalid(schema_name);
    auto &target_pool = _data->obtain(schemas_pool_name);
    target_pool.dispose(schema_name);
}

void data_base::dispose_table(const std::string &schemas_pool_name, const std::string &schema_name, const std::string &table_name)
{
    throw_if_invalid(schemas_pool_name);
    throw_if_invalid(schema_name);
    throw_if_invalid(table_name);
    auto &target_pool = _data->obtain(schemas_pool_name);
    auto &schema = target_pool.obtain(schema_name);
    schema.dispose(table_name);
}

void data_base::dispose_user_data(
	const std::string &schemas_pool_name,
	const std::string &schema_name,
	const std::string &table_name,
	const std::string &user_data_key)
{
    throw_if_invalid(schemas_pool_name);
    throw_if_invalid(schema_name);
    throw_if_invalid(table_name);
    throw_if_invalid(user_data_key);
    auto &target_pool = _data->obtain(schemas_pool_name);
    auto &schema = target_pool.obtain(schema_name);
    auto &tbl = schema.obtain(table_name);
    tbl.dispose(user_data_key);
}

void data_base::save_db_to_filesystem()
{
    //TODO: save class-state and add prefix _abs_directory;

    std::string filename = this->_instance_name + data_base::_file_format;

    std::ofstream output_file(filename);

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
	throw std::runtime_error("file for deserializing did not open! file_name: [ " + filename + " ]");
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
void data_base::deserialize()
{
}

void data_base::throw_if_invalid(const std::string &key)
{
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

/*
 *
 * Need be tested!!!
 *TODO: replace with storage_interface<>::bin_search
 */
std::string data_base::find_key_in_file(const std::string &key)
{
    if (key.length() == 0)
    {
	throw std::logic_error("invalid arguments");
    }

    //TODO: think about std:::Absolute!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    auto filename = std::filesystem::absolute(get_instance_name() + _file_format);
    auto index_filename = std::filesystem::absolute(std::string{"index_"} + get_instance_name() + _file_format);

    std::ifstream index_file(index_filename);
    if (!index_file.is_open())
    {
	throw std::runtime_error("file did not open");
    }

    std::vector<std::streamoff> index_array;
    std::string index_str;
    while (std::getline(index_file, index_str, '#'))
    {
	index_array.push_back(std::stol(index_str));
    }

    index_file.close();

    std::ifstream main_file(filename);
    if (!main_file.is_open())
    {
	throw std::runtime_error(std::string{"file did not open"});
    }

    size_t left = 0;
    size_t right = index_array.size() - 1;

    while (left <= right)
    {
	size_t mid = left + (right - left) / 2;

	main_file.seekg(index_array[mid]);

	std::string file_key;
	std::getline(main_file, file_key, '|');

	if (key == file_key)
	{
	    main_file.close();
	    return file_key;
	}

	if (file_key < key)
	{
	    left = mid + 1;
	}
	else
	{
	    right = mid - 1;
	}
    }

    main_file.close();
    throw std::logic_error("key not found in file");
}

void data_base::insert_pool_to_filesystem(std::string const &pool_name, schemas_pool const &value)
{

}

void data_base::insert_pool_to_filesystem(std::string const &pool_name, schemas_pool &&value)
{
    if (get_strategy() == storage_strategy::in_memory)
    {
	throw std::logic_error("incorrect strategy");
    }

    std::string out_str = pool_name + "#";
    length_alignment(out_str);

    auto filename = data_base::_absolute_directory_name + get_instance_name() + _file_format;
    auto index_filename = data_base::_absolute_directory_name + "index_" + get_instance_name() + _file_format;

    std::ifstream test_exist(filename);
    if (!test_exist)
    {
	std::ofstream new_file(filename);
	std::vector<std::streamoff> new_index_array = {0};
	if (!new_file)
	{
	    throw std::logic_error("Cannot create a new file");
	}

	new_file << out_str << std::endl;
	new_file.close();
	save_index(new_index_array, index_filename);
	return;
    }
    test_exist.close();

    auto index_array = load_index(index_filename);
    if (index_array.empty())
    {
	std::ofstream src(filename);
	throw_if_not_open(src);
	src << out_str << std::endl;
	src.close();
	update_index(index_array);
	save_index(index_array, index_filename);
	return;
    }

    std::ifstream data_file(filename);
    throw_if_not_open(data_file);

    size_t left = 0;
    size_t right = index_array.size() - 1;
    std::string file_key;
    while (left <= right)
    {
	size_t mid = left + (right - left) / 2;

	data_file.seekg(index_array[mid]);

	std::getline(data_file, file_key, '#');

	if (file_key == pool_name)
	{
	    data_file.close();
	    throw std::logic_error("duplicate key");
	}

	if (right == left)
	{
	    break;
	}

	if (file_key < pool_name)
	{
	    left = mid + 1;
	}
	else
	{
	    right = mid;
	}
    }

    bool is_target_greater = file_key < pool_name;

    std::string temp_filename = schemas_pool::_absolute_directory_name + std::string{"temp"} + _file_format;

    data_file.close();

    if (left == index_array.size() - 1 && is_target_greater)
    {
	std::ofstream data_file(filename, std::ios::app);
	table::throw_if_not_open(data_file);
	data_file << out_str << std::endl;
	data_file.close();
	table::update_index(index_array);
	table::save_index(index_array, index_filename);
	return;
    }

    std::ifstream src(filename);
    table::throw_if_not_open(src);
    std::ofstream tmp_file(temp_filename);
    table::throw_if_not_open(tmp_file);

    std::string src_line;
    size_t pos;
    while (std::getline(src, src_line))
    {
	pos = src_line.find('#');
	if (pos != std::string::npos)
	{
	    std::string current_key = src_line.substr(0, pos);
	    if (current_key == file_key)
	    {
		if (is_target_greater)
		{
		    tmp_file << src_line << std::endl;
		    tmp_file << out_str << std::endl;
		}
		else
		{
		    tmp_file << out_str << std::endl;
		    tmp_file << src_line << std::endl;
		}

		continue;
	    }
	}

	tmp_file << src_line << std::endl;
    }

    update_index(index_array);
    save_index(index_array, index_filename);
    data_file.close();
    tmp_file.close();

    //TODO: copy temp to src;
    std::string backup_filename = schemas_pool::_absolute_directory_name + "backup_" + get_instance_name() + _file_format;

    {
	std::ifstream src_orig(filename);
	table::throw_if_not_open(src_orig);
	std::ofstream backup_file(backup_filename, std::ios::trunc);
	table::throw_if_not_open(backup_file);
	backup_file << src_orig.rdbuf();
	src_orig.close();
	backup_file.close();
    }

    bool copy_success = true;

    try
    {
	std::ifstream temp_file(temp_filename, std::ios::binary);
	table::throw_if_not_open(temp_file);

	std::ofstream final_data_file(filename, std::ios::binary | std::ios::trunc);
	table::throw_if_not_open(final_data_file);

	final_data_file << temp_file.rdbuf();

	if (!temp_file.good() || !final_data_file.good())
	{
	    copy_success = false;
	}

	temp_file.close();
	final_data_file.close();
    }
    catch (...)
    {
	copy_success = false;
    }

    if (copy_success)
    {
	//TODO: back-up logic!! Need to implementation
	if (remove(temp_filename.c_str()) != 0 || remove(backup_filename.c_str()) != 0)
	{
	    warning_with_guard("puc puc puc, removing backup or temp file went wrong :(");
	}
    }
    else
    {
	//restoring file from back-up
	std::ifstream backup_file(backup_filename, std::ios::binary);
	table::throw_if_not_open(backup_file);
	std::ofstream src_file(filename, std::ios::binary | std::ios::trunc);
	table::throw_if_not_open(src_file);
	src_file << backup_file.rdbuf();

	if (!backup_file.good() || !src_file.good())
	{
	    throw std::logic_error("smth went wrong puc puc..; back up went wrong");
	}

	backup_file.close();
	src_file.close();
    }
}