//
// Created by rob22 on 04.05.2024.
//

#include "../include/table.h"
#include "../../common/include/storage_interface.h"
#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>

std::function<int(const std::string &, const std::string &)> table::_default_string_comparer = [](const std::string &a, const std::string &b) -> int { return a.compare(b); };

table::table() : _data(std::make_unique<b_tree<std::string, user_data>>(2, _default_string_comparer, nullptr, nullptr))
{
    set_instance_name("table_name");
    this->_logger = nullptr;
    this->_storaged_strategy = storage_interface<std::string, user_data>::storage_strategy::in_memory;

    if (this->_storaged_strategy == storage_interface<std::string, user_data>::storage_strategy::in_memory)
    {
	//trace_with_guard("data was deserialized");
	//deserialize();
    }
}

table::table(
	std::size_t t,
	allocator *allocator,
	logger *logger,
	const std::function<int(const std::string &, const std::string &)> &keys_comparer,
	storage_strategy storage_strategy,
	std::string instance_name)
    : _data(std::make_unique<b_tree<std::string, user_data>>(t, keys_comparer, allocator, logger))
{
    set_instance_name(instance_name);
    this->_logger = logger;

    set_strategy(storage_strategy);

    if (this->_storaged_strategy == storage_interface<std::string, user_data>::storage_strategy::in_memory)
    {
	//trace_with_guard("data was deserialized");
	//table::deserialize();
    }
}

table::~table()
{

    if (this->_storaged_strategy == storage_interface<std::string, user_data>::storage_strategy::in_memory)
    {
	trace_with_guard("data was serialized after desctructor called");
	//table::serialize();
    }
}

table::table(const table &other)
{
    if (other._data)
    {
	_data = std::make_unique<b_tree<std::string, user_data>>(*other._data);
    }
    else
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
    std::string out_str = key + "#" + std::to_string(value.get_id()) + "#" + value.get_name() + "#" + value.get_surname() + "|";
    throw_if_exceeds_length_limit(out_str);

    switch (this->get_strategy())
    {
	case storage_strategy::in_memory:
	    _data->insert(key, value);
	    break;
	case storage_strategy::filesystem:
	    insert_to_filesystem(key, value);
	    break;
    }
}

void table::insert(const std::string &key, user_data &&value)
{
    std::string out_str = key + "#" + std::to_string(value.get_id()) + "#" + value.get_name() + "#" + value.get_surname() + "|";
    throw_if_exceeds_length_limit(out_str);

    switch (this->get_strategy())
    {
	case storage_strategy::in_memory:
	    _data->insert(key, std::move(value));
	    break;
	case storage_strategy::filesystem:
	    //insert_to_filesystem(key, std::move(value));
	    break;
    }
}

void table::throw_if_not_open(std::ifstream const &file)
{
    if (!file.is_open())
    {
	throw std::runtime_error("file did not open!!1!!!111!1");
    }
}

void table::throw_if_not_open(std::ofstream const &file)
{
    if (!file.is_open())
    {
	throw std::runtime_error("file did not open!00!!))!1!!!");
    }
}

void table::insert_to_filesystem(std::string const &key, const user_data &value)
{
    if (get_strategy() == storage_strategy::in_memory)
    {
	throw std::logic_error("incorrect strategy");
    }
    std::string out_str = key + "#" + std::to_string(value.get_id()) + "#" + value.get_name() + "#" + value.get_surname() + "|";
    length_alignment(out_str);

    auto filename =  get_instance_name() + _file_format;
    auto index_filename = "index_" + get_instance_name() + _file_format;

    std::ifstream test_exist(filename);
    if (!test_exist)
    {
	std::ofstream new_file(filename);
	std::vector<std::streamoff> new_index_array = {0};
	if (!new_file) throw std::logic_error("Cannot create a new file");
	new_file << out_str << std::endl;
	new_file.close();
	save_index(new_index_array, index_filename);
	return;
    }
    test_exist.close();

    auto index_array = load_index(index_filename);

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

	if (file_key == key)
	{
	    data_file.close();
	    throw std::logic_error("duplicate key");
	}

	if (right == left)
	{
	    break;
	}

	if (file_key < key)
	{
	    left = mid + 1;
	}
	else
	{
	    right = mid;
	}
    }

    bool is_target_greater = file_key < key;

    std::string temp_filename =  std::string{"temp"} + _file_format;

    data_file.close();

    if (left == index_array.size() - 1 && is_target_greater)
    {
	std::ofstream data_file(filename, std::ios::app);
	throw_if_not_open(data_file);
	data_file << out_str << std::endl;
	data_file.close();
	update_index(index_array);
	save_index(index_array, index_filename);
	return;
    }

    std::ifstream src(filename);
    throw_if_not_open(src);
    std::ofstream tmp_file(temp_filename);
    throw_if_not_open(tmp_file);

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

    std::string backup_filename = "backup_" + get_instance_name() + _file_format;

    {
	std::ifstream src_orig(filename);
	throw_if_not_open(src_orig);
	std::ofstream backup_file(backup_filename, std::ios::trunc);
	throw_if_not_open(backup_file);
	backup_file << src_orig.rdbuf();
	src_orig.close();
	backup_file.close();
    }

    bool copy_success = true;

    try
    {
	std::ifstream temp_file(temp_filename, std::ios::binary);
	throw_if_not_open(temp_file);

	std::ofstream final_data_file(filename, std::ios::binary | std::ios::trunc);
	if (!final_data_file.is_open())
	{
	    temp_file.close();
	    throw_if_not_open(final_data_file);
	}

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
	throw_if_not_open(backup_file);
	std::ofstream src_file(filename, std::ios::binary | std::ios::trunc);
	if (!src_file.is_open())
	{
	    backup_file.close();
	    throw_if_not_open(src_file);
	}
	src_file << backup_file.rdbuf();

	if (!backup_file.good() || !src_file.good())
	{
	    throw std::logic_error("smth went wrong puc puc..; back up went wrong");
	}

	backup_file.close();
	src_file.close();
    }
}
/*
 * unused?!?!??
// */
//void table::copy_file(const std::string &source_path, const std::string &dest_path)
//{
//    std::ifstream source_file(source_path, std::ios::binary);
//    if (!source_file.is_open())
//    {
//	throw std::runtime_error("Cannot open source file: " + source_path);
//    }
//
//    std::ofstream dest_file(dest_path, std::ios::binary);
//    if (!dest_file.is_open())
//    {
//	source_file.close();
//	throw std::runtime_error("Cannot create destination file: " + dest_path);
//    }
//
//    dest_file << source_file.rdbuf();
//
//    if (source_file.bad())
//    {
//	throw std::runtime_error("Error occurred while reading the source file.");
//    }
//
//    if (dest_file.bad())
//    {
//	throw std::runtime_error("Error occurred while writing to the destination file.");
//    }
//
//    source_file.close();
//    dest_file.close();
//}

void table::update_ud_in_filesystem(std::filesystem::path const &table_path, std::filesystem::path const &index_table_path, std::string const &user_data_key, user_data &&value)
{
    std::string out_str = std::to_string(string_pool::add_string(user_data_key)) + "#" + std::to_string(value.get_id()) + "#" + value.get_name() + "#" + value.get_surname() + "|";
    length_alignment(out_str);

    std::vector<std::streamoff> index_array = load_index(index_table_path.string());

    std::fstream target_file(table_path, std::ios::in | std::ios::out);
    if (!target_file.is_open())
    {
	throw std::runtime_error("file did not open, sorry ^(");
    }

    size_t left = 0;
    size_t right = index_array.size() - 1;
    std::string file_key;
    while (left <= right)
    {
	size_t mid = left + (right - left) / 2;
	target_file.seekg(index_array[mid]);
	std::string key_ind;
	std::getline(target_file, key_ind, '#');
	file_key = string_pool::get_string(std::stol(key_ind));
	if (user_data_key == file_key)
	{
	    create_backup(table_path);
	    try
	    {
		target_file.seekg(index_array[mid]);
		target_file << out_str;
		target_file.close();
		delete_backup(table_path);
		return;
	    }
	    catch (...)
	    {
		target_file.close();
		load_backup(table_path);
		throw;
	    }
	}

	if (right == left)
	{
	    break;
	}
	if (file_key < user_data_key)
	{
	    left = mid + 1;
	}
	else
	{
	    right = mid;
	}
    }

    target_file.close();
    throw std::logic_error("key not found: " + file_key);
}
//
//void table::update_in_filesystem(std::string const &key, user_data &&value)
//{
//    if (get_strategy() == storage_strategy::in_memory)
//    {
//	throw std::logic_error("access denied, invalid strategy");
//    }
//
//    std::string out_str = std::to_string(string_pool::add_string(key)) + "#" + std::to_string(value.get_id()) + "#" + value.get_name() + "#" + value.get_surname() + "|";
//    length_alignment(out_str);
//
//    auto filename =  get_instance_name() + _file_format;
//    auto index_filename =+ "index_" + get_instance_name() + _file_format;
//
//    std::ifstream index_file(index_filename);
//    throw_if_not_open(index_file);
//
//    std::vector<std::streamoff> index_array = load_index(index_filename);
//
//    std::fstream target_file(filename, std::ios::in | std::ios::out);
//    if (!target_file.is_open())
//    {
//	throw std::runtime_error("file did not open, sorry ^(");
//    }
//
//    size_t left = 0;
//    size_t right = index_array.size() - 1;
//    std::string file_key;
//    while (left <= right)
//    {
//	size_t mid = left + (right - left) / 2;
//	target_file.seekg(index_array[mid]);
//	std::getline(target_file, file_key, '#');
//
//	if (key == file_key)
//	{
//	    target_file.seekg(index_array[mid]);
//	    target_file << out_str;
//	    target_file.close();
//	    return;
//	}
//
//	if (right == left)
//	{
//	    break;
//	}
//	if (file_key < key)
//	{
//	    left = mid + 1;
//	}
//	else
//	{
//	    right = mid;
//	}
//    }
//
//
//    target_file.close();
//    throw std::logic_error("key not found: " + key);
//}
//
//void table::update_in_filesystem(std::string const &key, user_data const &value)
//{
//    update_in_filesystem(key, user_data(value.get_id(), value.get_name(), value.get_surname()));
//}

user_data table::create_user_data(const std::string &ud_line)
{
    if (ud_line.empty())
    {
	throw std::logic_error("attempt to map empty line");
    }

    std::istringstream iss(ud_line);
    std::string key, id_str, name_ind, surname_ind;

    std::getline(iss, key, '#');
    std::getline(iss, id_str, '#');
    std::getline(iss, name_ind, '#');
    std::getline(iss, surname_ind, '|');

    std::string name = string_pool::get_string(std::stol(name_ind));
    std::string surname = string_pool::get_string(std::stol(surname_ind));
    size_t id = std::stoul(id_str);

    user_data ud(id, name, surname);

    return ud;
}

[[maybe_unused]] int table::get_index_by_bin_search(std::ifstream &src, std::vector<std::streamoff> const &index_array, std::string const &key)
{
    if (!src.is_open())
    {
	throw std::logic_error("file is not open :(");
    }

    size_t left = 0;
    size_t right = index_array.size() - 1;
    std::string file_key;

    while (left <= right)
    {
	size_t mid = left + (right - left) / 2;
	src.seekg(index_array[mid]);
	std::getline(src, file_key, '#');

	if (key == file_key)
	{
	    return mid;
	}
	if (right == left)
	{
	    break;
	}
	if (file_key < key)
	{
	    left = mid + 1;
	}
	else
	{
	    right = mid;
	}
    }

    return -1;
}

std::map<std::string, user_data> table::obtain_all_ud_in_filesystem(
	std::filesystem::path const &filepath,
	std::filesystem::path const &index_filepath)
{
    std::vector<std::streamoff> index_array = load_index(index_filepath.string());

    std::ifstream data_file(filepath);
    throw_if_not_open(data_file);

    std::map<std::string, user_data> result;

    std::string readline;
    size_t pos;
    while (std::getline(data_file, readline))
    {
	pos = readline.find('#');
	if (pos != std::string::npos)
	{
	    std::string curr_key_ind = readline.substr(0, pos);
	    std::string current_key = string_pool::get_string(std::stol(curr_key_ind));

	    auto data = create_user_data(readline);
	    result.emplace(current_key, data);
	}
    }

    data_file.close();
    return result;
}

std::map<std::string, user_data> table::obtain_between_ud_in_filesystem(
	std::filesystem::path const &filepath,
	std::filesystem::path const &index_filepath,
	std::string const &lower_bound,
	std::string const &upper_bound,
	bool lower_bound_inclusive,
	bool upper_bound_inclusive)
{
    if (upper_bound < lower_bound)
    {
	throw std::logic_error("upper bound less than lower?? puc puc..");
    }

    std::vector<std::streamoff> index_array = load_index(index_filepath.string());

    std::ifstream data_file(filepath);
    throw_if_not_open(data_file);

    size_t left = 0;
    size_t right = index_array.size() - 1;
    std::string file_key;
    bool is_lower_found = false;
    while (left <= right)
    {
	size_t mid = left + (right - left) / 2;
	std::string temp_str;
	data_file.seekg(index_array[mid]);
	std::getline(data_file, temp_str, '#');
	file_key = string_pool::get_string(std::stol(temp_str));
	if (lower_bound == file_key)
	{
	    left = mid;
	    is_lower_found = true;
	    break;
	}
	if (right == left)
	{
	    break;
	}
	if (file_key < lower_bound)
	{
	    left = mid + 1;
	}
	else
	{
	    right = mid;
	}
    }

    if (!is_lower_found && left >= index_array.size() - 1)
    {
	data_file.close();
	throw std::logic_error("left bound is not exist");
    }

    size_t start_index = is_lower_found && (lower_bound_inclusive) ? left : left + 1;
    if (is_lower_found)
    {
	if (!lower_bound_inclusive)
	{
	    start_index = left + 1;
	}
	else
	{
	    start_index = left;
	}
    }
    else
    {
	if (file_key > lower_bound)
	{
	    start_index = left;
	}
	else
	{
	    start_index = left + 1;
	}
    }

    std::map<std::string, user_data> result;

    std::string readln;
    size_t pos;
    data_file.seekg(index_array[start_index]);
    while (std::getline(data_file, readln))
    {
	pos = readln.find('#');
	if (pos != std::string::npos)
	{
	    std::string curr_key_ind = readln.substr(0, pos);
	    std::string current_key = string_pool::get_string(std::stol(curr_key_ind));

	    if (current_key >= upper_bound)
	    {
		if (current_key == upper_bound && upper_bound_inclusive)
		{
		    auto data = create_user_data(readln);
		    result.emplace(upper_bound, data);
		}

		break;
	    }

	    auto data = create_user_data(readln);
	    result.emplace(current_key, data);
	}
    }

    data_file.close();
    return result;
}
//
//std::map<std::string, user_data> table::obtain_between_in_filesystem(
//	const std::string &lower_bound,
//	const std::string &upper_bound,
//	bool lower_bound_inclusive,
//	bool upper_bound_inclusive)
//{
//    if (get_strategy() == storage_strategy::in_memory)
//    {
//	throw std::logic_error("access denied, invalid strategy");
//    }
//
//    if (upper_bound < lower_bound)
//    {
//	throw std::logic_error("upper bound less than lower?? puc puc..");
//    }
//
//    auto filename =get_instance_name() + _file_format;
//    auto index_filename ="index_" + get_instance_name() + _file_format;
//
//    std::ifstream index_file(index_filename);
//    throw_if_not_open(index_file);
//
//    std::vector<std::streamoff> index_array = load_index(index_filename);
//
//    std::ifstream data_file(filename);
//    throw_if_not_open(data_file);
//
//    size_t left = 0;
//    size_t right = index_array.size() - 1;
//    std::string file_key;
//    bool is_lower_found = false;
//    while (left <= right)
//    {
//	size_t mid = left + (right - left) / 2;
//	data_file.seekg(index_array[mid]);
//	std::getline(data_file, file_key, '#');
//
//	if (lower_bound == file_key)
//	{
//	    left = mid;
//	    is_lower_found = true;
//	    break;
//	}
//	if (right == left)
//	{
//	    break;
//	}
//	if (file_key < lower_bound)
//	{
//	    left = mid + 1;
//	}
//	else
//	{
//	    right = mid - 1;
//	}
//    }
//
//    if (!is_lower_found && left >= index_array.size() - 1)
//    {
//	data_file.close();
//	throw std::logic_error("left bound is not exist");
//    }
//
//    size_t start_index = is_lower_found && (lower_bound_inclusive) ? left : left + 1;
//    if (is_lower_found)
//    {
//	if (!lower_bound_inclusive)
//	{
//	    start_index = left + 1;
//	}
//	else
//	{
//	    start_index = left;
//	}
//    }
//    else
//    {
//	if (file_key > lower_bound)
//	{
//	    start_index = left;
//	}
//	else
//	{
//	    start_index = left + 1;
//	}
//    }
//
//    std::map<std::string, user_data> result;
//
//    std::string readln;
//    size_t pos;
//    data_file.seekg(index_array[start_index]);
//    while (std::getline(data_file, readln))
//    {
//	pos = readln.find('#');
//	if (pos != std::string::npos)
//	{
//	    std::string current_key = readln.substr(0, pos);
//
//	    if (current_key >= upper_bound)
//	    {
//		if (current_key == upper_bound && upper_bound_inclusive)
//		{
//		    auto data = create_user_data(readln);
//		    result.emplace(upper_bound, data);
//		}
//
//		break;
//	    }
//
//	    auto data = create_user_data(readln);
//	    result.emplace(current_key, data);
//	}
//    }
//
//    data_file.close();
//
//    return result;
//}

void table::dispose_ud_from_filesystem(std::filesystem::path const &path, std::filesystem::path const &index_path, std::string const &key)
{
    std::vector<std::streamoff> index_array = load_index(index_path.string());
    if (index_array.empty())
    {
	throw std::logic_error("Attemp to dispose from empty file");
    }

    std::ifstream src(path);
    throw_if_not_open(src);

    size_t left = 0;
    size_t right = index_array.size() - 1;
    bool is_found = false;
    std::string file_key;
    while (left <= right)
    {
	size_t mid = left + (right - left) / 2;
	src.seekg(index_array[mid]);
	std::string key_index;
	std::getline(src, key_index, '#');
	file_key = string_pool::get_string(std::stol(key_index));

	if (key == file_key)
	{
	    is_found = true;
	    break;
	}

	if (right == left)
	{
	    src.close();
	    throw std::logic_error("key not found: " + key);
	}
	if (file_key < key)
	{
	    left = mid + 1;
	}
	else
	{
	    right = mid;
	}
    }

    create_backup(path);

    if (is_found && index_array.size() == 1)
    {
	try
	{
	    src.close();
	    std::ofstream clear_file(path, std::ios::trunc);
	    throw_if_not_open(clear_file);
	    clear_file.close();
	    decrease_index(index_array);
	    save_index(index_array, index_path.string());
	    delete_backup(path);
	    return;
	}
	catch (...)
	{
	    load_backup(path);
	    throw;
	}
    }

    src.seekg(0);

    try
    {
	auto temp_filename = path.string() + (std::string{"temp"} + _file_format);

	std::ofstream tmp_file(temp_filename);
	throw_if_not_open(tmp_file);

	std::string src_line;
	size_t pos;
	while (std::getline(src, src_line))
	{
	    pos = src_line.find('#');
	    if (pos != std::string::npos)
	    {
		std::string key_index = src_line.substr(0, pos);
		std::string current_key = string_pool::get_string(std::stol(key_index));
		if (current_key == file_key)
		{
		    continue;
		}
	    }

	    tmp_file << src_line << std::endl;
	}

	index_array.pop_back();
	save_index(index_array, index_path.string());
	src.close();
	tmp_file.close();

	std::filesystem::remove(path);
	std::filesystem::rename(temp_filename, path);
    }
    catch (...)
    {
	load_backup(path);
	throw;
    }

    delete_backup(path);
}

void table::insert_ud_to_filesystem(std::filesystem::path const &path, std::filesystem::path const &index_path, std::string const &key, user_data const &ud)
{
    std::string out_str = std::to_string(string_pool::add_string(key)) + "#" + std::to_string(ud.get_id()) + "#" + ud.get_name() + "#" + ud.get_surname() + "|";
    length_alignment(out_str);

    if (check_and_create_with_insertion(path, index_path, out_str))
    {
	return;
    }

    auto index_array = load_index(index_path.string());

    if (index_array.empty())
    {
	std::ofstream out_f(path);
	throw_if_not_open(out_f);
	std::ofstream index_f(index_path);
	if (!index_f.is_open())
	{
	    out_f.close();
	    throw_if_not_open(index_f);
	}
	out_f << out_str << std::endl;
	out_f.close();
	storage_interface::update_index(index_array);
	storage_interface::save_index(index_array, index_f);
	index_f.close();
	return;
    }

    std::ifstream src(path);
    throw_if_not_open(src);

    size_t left = 0;
    size_t right = index_array.size() - 1;
    std::string file_key;
    while (left <= right)
    {
	size_t mid = left + (right - left) / 2;
	src.seekg(index_array[mid]);
	std::string key_index;
	std::getline(src, key_index, '#');

	file_key = string_pool::get_string(std::stol(key_index));
	if (file_key == key)
	{
	    src.close();
	    throw std::logic_error("duplicate key");
	}

	if (right == left)
	{
	    src.close();
	    break;
	}

	if (file_key < key)
	{
	    left = mid + 1;
	}
	else
	{
	    right = mid;
	}
    }

    create_backup(path);

    bool is_target_greater = file_key < key;

    if (left == index_array.size() - 1 && is_target_greater)
    {
	try
	{
	    std::ofstream data_file(path, std::ios::app);
	    throw_if_not_open(data_file);
	    data_file << out_str << std::endl;
	    data_file.close();
	    update_index(index_array);
	    save_index(index_array, index_path.string());
	}
	catch (...)
	{
	    load_backup(path);
	    throw;
	}

	delete_backup(path);
	return;
    }

    try
    {
	std::ifstream data_file(path);
	throw_if_not_open(data_file);
	auto tmp_filename = path.string() + "_temp" + _file_format;
	std::ofstream tmp_file(tmp_filename);
	if (!tmp_file.is_open())
	{
	    data_file.close();
	    throw_if_not_open(tmp_file);
	}

	std::string src_line;
	size_t pos;
	while (std::getline(data_file, src_line))
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
	save_index(index_array, index_path.string());
	data_file.close();
	tmp_file.close();

	std::filesystem::remove(path);
	std::filesystem::rename(tmp_filename, path);
    }
    catch (...)
    {
	load_backup(path);
	throw;
    }

    delete_backup(path);
}

void table::insert_ud_to_filesystem(std::filesystem::path const &path, std::filesystem::path const &index_path, std::string const &key, user_data &&ud)
{
    std::string out_str = std::to_string(string_pool::add_string(key)) + "#" + std::to_string(ud.get_id()) + "#" + ud.get_name() + "#" + ud.get_surname() + "|";
    length_alignment(out_str);

    if (check_and_create_with_insertion(path, index_path, out_str))
    {
	return;
    }

    auto index_array = load_index(index_path.string());

    if (index_array.empty())
    {
	std::ofstream out_f(path);
	throw_if_not_open(out_f);
	std::ofstream index_f(index_path);
	if (!index_f.is_open())
	{
	    out_f.close();
	    throw_if_not_open(index_f);
	}
	out_f << out_str << std::endl;
	out_f.close();
	storage_interface::update_index(index_array);
	storage_interface::save_index(index_array, index_f);
	index_f.close();
	return;
    }

    std::ifstream src(path);
    throw_if_not_open(src);

    size_t left = 0;
    size_t right = index_array.size() - 1;
    std::string file_key;
    while (left <= right)
    {
	size_t mid = left + (right - left) / 2;
	src.seekg(index_array[mid]);
	std::string key_index;
	std::getline(src, key_index, '#');

	file_key = string_pool::get_string(std::stol(key_index));
	if (file_key == key)
	{
	    src.close();
	    throw std::logic_error("duplicate key");
	}

	if (right == left)
	{
	    src.close();
	    break;
	}

	if (file_key < key)
	{
	    left = mid + 1;
	}
	else
	{
	    right = mid;
	}
    }

    create_backup(path);

    bool is_target_greater = file_key < key;

    if (left == index_array.size() - 1 && is_target_greater)
    {
	try
	{
	    std::ofstream data_file(path, std::ios::app);
	    throw_if_not_open(data_file);
	    data_file << out_str << std::endl;
	    data_file.close();
	    update_index(index_array);
	    save_index(index_array, index_path.string());
	}
	catch (...)
	{
	    load_backup(path);
	    throw;
	}

	delete_backup(path);
	return;
    }

    try
    {
	std::ifstream data_file(path);
	throw_if_not_open(data_file);
	auto tmp_filename = path.string() + "_temp" + _file_format;
	std::ofstream tmp_file(tmp_filename);
	if (!tmp_file.is_open())
	{
	    data_file.close();
	    throw_if_not_open(tmp_file);
	}

	std::string src_line;
	size_t pos;
	while (std::getline(data_file, src_line))
	{
	    pos = src_line.find('#');
	    if (pos != std::string::npos)
	    {
		std::string key_index = src_line.substr(0, pos);
		std::string current_key = string_pool::get_string(std::stol(key_index));
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
	save_index(index_array, index_path.string());
	data_file.close();
	tmp_file.close();

	std::filesystem::remove(path);
	std::filesystem::rename(tmp_filename, path);
    }
    catch (...)
    {
	load_backup(path);
	throw;
    }

    delete_backup(path);
}

bool table::check_and_create_with_insertion(const std::filesystem::path &path, const std::filesystem::path &index_filename, const std::string &out_str)
{

    if (std::filesystem::exists(path))
    {
	return false;
    }

    std::ofstream new_file(path);
    if (!new_file.is_open())
    {
	throw std::runtime_error("Cannot create a new file: " + path.string());
    }
    std::vector<std::streamoff> new_index_array = {0};
    new_file << out_str << std::endl;
    new_file.close();

    save_index(new_index_array, index_filename.string());

    return true;
}

bool table::check_and_create_empty(const std::filesystem::path &path, const std::filesystem::path &index_filename)
{

    if (std::filesystem::exists(path))
    {
	return false;
    }

    std::ofstream new_file(path, std::ios::out | std::ios::trunc);
    if (!new_file.is_open())
    {
	throw std::runtime_error("Cannot create a new file: " + path.string());
    }
    std::vector<std::streamoff> new_index_array = {};
    new_file.close();

    save_index(new_index_array, index_filename.string());

    return true;
}

void table::load_backup(const std::filesystem::path &source_path)
{

    std::filesystem::path backup_path = source_path;
    backup_path += ".backup";

    if (!std::filesystem::exists(backup_path))
    {
	throw std::runtime_error("Load backup file failed. Backup file does not exist: " + backup_path.string());
    }

    try
    {
	std::filesystem::path temp_backup_path = backup_path;
	temp_backup_path += ".tmp";
	std::filesystem::copy(backup_path, temp_backup_path);

	if (std::filesystem::exists(source_path))
	{
	    std::filesystem::remove(source_path);
	}

	std::filesystem::rename(temp_backup_path, source_path);
    }
    catch (...)
    {

	if (std::filesystem::exists(source_path))
	{
	    std::filesystem::remove(source_path);
	}
	std::filesystem::path temp_backup_path = backup_path;
	temp_backup_path += ".tmp";
	if (std::filesystem::exists(temp_backup_path))
	{
	    std::filesystem::rename(temp_backup_path, source_path);
	}
	throw;
    }

    std::filesystem::path temp_backup_path = backup_path;
    temp_backup_path += ".tmp";
    if (std::filesystem::exists(temp_backup_path))
    {
	std::filesystem::remove(temp_backup_path);
    }
}

void table::delete_backup(const std::filesystem::path &source_path)
{
    std::filesystem::path backup_path = source_path;
    backup_path += ".backup";

    if (std::filesystem::exists(backup_path))
    {
	try
	{
	    std::filesystem::remove(backup_path);
	}
	catch (const std::filesystem::filesystem_error &e)
	{
	    throw std::runtime_error("Failed to delete backup file: " + backup_path.string() + ". Error: " + e.what());
	}
    }
    else
    {
	throw std::runtime_error("Backup file does not exist: " + backup_path.string());
    }
}

void table::create_backup(const std::filesystem::path &source_path)
{
    if (!std::filesystem::exists(source_path))
    {
	throw std::runtime_error("Source file does not exist: " + source_path.string());
    }

    std::filesystem::path backup_path = source_path;
    backup_path += ".backup";

    std::ifstream src_orig(source_path, std::ios::binary);
    throw_if_not_open(src_orig);

    std::ofstream backup_file(backup_path, std::ios::trunc | std::ios::binary);
    if (!backup_file.is_open())
    {
	src_orig.close();
	throw std::runtime_error("Failed to create backup file: " + backup_path.string());
    }

    backup_file << src_orig.rdbuf();

    src_orig.close();
    backup_file.close();
}

user_data &table::obtain(const std::string &key)
{
    return _data->obtain(key);
}

std::map<std::string, user_data> table::obtain_between(
	const std::string &lower_bound,
	const std::string &upper_bound,
	bool lower_bound_inclusive,
	bool upper_bound_inclusive)
{
    std::map<std::string, user_data> result_map;
    switch (this->get_strategy())
    {
	case storage_strategy::in_memory: {
	    auto collection = _data->obtain_between(lower_bound, upper_bound, lower_bound_inclusive, upper_bound_inclusive);

	    size_t length = collection.size();

	    for (auto &i: collection)
	    {
		auto key = i.key;
		auto value = i.value;
		result_map.emplace(key, value);
	    }
	    return result_map;
	}
	case storage_strategy::filesystem: {
	    return result_map;
	    break;
	}
    }
}

void table::dispose(const std::string &key)
{
    switch (this->get_strategy())
    {
	case storage_strategy::in_memory:
	    _data->dispose(key);
	    break;
	case storage_strategy::filesystem:

	    break;
    }
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
    auto it_end = _data->end_infix();

    while (it != it_end)
    {
	user_data ud = std::get<3>(*it);
	std::cout << std::get<2>(*it) << " " << ud.get_name() << " " << ud.get_surname() << " " << ud.get_id() << " " << std::endl;
	++it;
    }
}

table table::load_data_from_filesystem(std::string const &filename)
{
    table src_table;

    std::ifstream input_file(filename);
    if (!input_file.is_open())
    {
	return src_table;
    }

    std::string line;
    while (std::getline(input_file, line))
    {
	if (line.empty())
	    continue;
	//line.pop_back();
	std::string new_line;
	for (int i = 0; i < line.length(); ++i)
	{
	    if (line[i] != '|')
	    {
		new_line += line[i];
	    }
	    else
	    {
		break;
	    }
	}

	std::istringstream line_stream(new_line);
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

	std::string string_key = string_pool::get_string(string_pool::add_string(seg_list[0]));
	size_t id = std::stol(seg_list[1]);
	std::string name = seg_list[2];
	std::string surname = seg_list[3];

	src_table.insert(string_key, user_data(id, name, surname));
    }

    input_file.close();
    return src_table;
}

void table::update(const std::string &key, const user_data &value)
{
    _data->update(key, value);
}

void table::update(const std::string &key, user_data &&value)
{
    _data->update(key, value);
}

void table::decrease_index(std::vector<std::streamoff> &vec)
{
    if (vec.empty())
    {
	return;
    }

    vec.pop_back();
}

void table::update_index(std::vector<std::streamoff> &vec)
{
    if (vec.empty())
    {
	vec.push_back(0);
	return;
    }

    vec.push_back(vec.back() + index_item_max_length);
}

void table::save_index(std::vector<std::streamoff> const &vec, std::string const &filename)
{
    std::ofstream file(filename, std::ios::trunc);
    throw_if_not_open(file);

    size_t size = vec.size();
    file << size << "#" << std::endl;

    file.close();
}

std::vector<std::streamoff> table::load_index(std::string const &index_filename)
{
    std::ifstream index_file(index_filename);
    throw_if_not_open(index_file);

    std::vector<std::streamoff> index_array = {};
    std::string array_size;
    std::getline(index_file, array_size, '#');
    index_file.close();

    size_t arr_size = std::stol(array_size);

    for (long long i = 0; i < arr_size; ++i)
    {
	index_array.push_back(i * index_item_max_length);
    }

    return index_array;
}

void table::insert_table_to_filesystem(const std::filesystem::path &path)
{
    std::ofstream out_file(path, std::ios::trunc | std::ios::out);
    throw_if_not_open(out_file);

    auto tbl_it = _data->begin_infix();
    auto tbl_end = _data->end_infix();
    while (tbl_it != tbl_end)
    {
	auto target_key = std::get<2>(*tbl_it);
	auto target_value = std::get<3>(*tbl_it);

	auto out_str = target_key + "#" + std::to_string(target_value.get_id()) + "#" + target_value.get_name() + "#" + target_value.get_surname() + "|";
	throw_if_exceeds_length_limit(out_str);
	length_alignment(out_str);
	out_file << out_str << std::endl;

	++tbl_it;
    }

    out_file.close();
}
void table::serialize()
{
}
void table::deserialize()
{
}
