//
// Created by rob22 on 04.05.2024.
//

#include "../include/table.h"
#include "../../common/include/storage_interface.h"
#include <fstream>
#include <functional>
#include <sstream>

std::function<int(const std::string &, const std::string &)> table::_default_string_comparer = [](const std::string &a, const std::string &b) -> int { return a.compare(b); };

table::table() : _data(std::make_unique<b_tree<std::string, user_data>>(4, _default_string_comparer, nullptr, nullptr))
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

    _absolute_directory_name = "C:\\Users\\rob22\\CLionProjects\\cw_os\\cw\\filesystem\\tables\\";

    set_strategy(storage_strategy);

    if (this->_storaged_strategy == storage_interface<std::string, user_data>::storage_strategy::in_memory)
    {
	//trace_with_guard("data was deserialized");
	table::deserialize();
    }
}

table::~table()
{

    if (this->_storaged_strategy == storage_interface<std::string, user_data>::storage_strategy::in_memory)
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
	    insert_to_filesystem(key, std::move(value));
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

    auto filename = table::_absolute_directory_name + get_instance_name() + _file_format;
    auto index_filename = table::_absolute_directory_name + "index_" + get_instance_name() + _file_format;

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

    std::string temp_filename = _absolute_directory_name + std::string{"temp"} + _file_format;

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

    std::string backup_filename = _absolute_directory_name + "backup_" + get_instance_name() + _file_format;

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
	throw_if_not_open(final_data_file);

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
	throw_if_not_open(src_file);
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
 */
void table::copy_file(const std::string &source_path, const std::string &dest_path)
{
    std::ifstream source_file(source_path, std::ios::binary);
    if (!source_file.is_open())
    {
	throw std::runtime_error("Cannot open source file: " + source_path);
    }

    std::ofstream dest_file(dest_path, std::ios::binary);
    if (!dest_file.is_open())
    {
	source_file.close();
	throw std::runtime_error("Cannot create destination file: " + dest_path);
    }

    dest_file << source_file.rdbuf();

    if (source_file.bad())
    {
	throw std::runtime_error("Error occurred while reading the source file.");
    }

    if (dest_file.bad())
    {
	throw std::runtime_error("Error occurred while writing to the destination file.");
    }

    source_file.close();
    dest_file.close();
}

void table::update_in_filesystem(std::string const &key, user_data &&value)
{
    if (get_strategy() == storage_strategy::in_memory)
    {
	throw std::logic_error("access denied, invalid strategy");
    }

    std::string out_str = key + "#" + std::to_string(value.get_id()) + "#" + value.get_name() + "#" + value.get_surname() + "|";
    length_alignment(out_str);

    auto filename = table::_absolute_directory_name + get_instance_name() + _file_format;
    auto index_filename = table::_absolute_directory_name + "index_" + get_instance_name() + _file_format;

    std::ifstream index_file(index_filename);
    throw_if_not_open(index_file);

    std::vector<std::streamoff> index_array = load_index(index_filename);

    std::fstream target_file(filename, std::ios::in | std::ios::out);
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
	std::getline(target_file, file_key, '#');

	if (key == file_key)
	{
	    target_file.seekg(index_array[mid]);
	    target_file << out_str;
	    target_file.close();
	    return;
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
	    right = mid - 1;
	}
    }


    target_file.close();
    throw std::logic_error("key not found");
}

void table::update_in_filesystem(std::string const &key, user_data const &value)
{
    update_in_filesystem(key,user_data(value.get_id(), value.get_name(), value.get_surname()));
}

user_data table::create_user_data(const std::string &ud_line) {
    if (ud_line.empty())
    {
	throw std::logic_error("attempt to map empty line");
    }

    std::istringstream iss(ud_line);
    std::string key, id_str, name, surname;

    std::getline(iss, key, '#');
    std::getline(iss, id_str, '#');
    std::getline(iss, name, '#');
    std::getline(iss, surname, '|');

    size_t id = std::stoul(id_str);

    user_data ud(id, name, surname);

    return ud;
}

int table::get_index_by_bin_search(std::ifstream &src, std::vector<std::streamoff> const &index_array, std::string const &key)
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
	    right = mid - 1;
	}
    }

    return -1;
}

std::map<std::string , user_data> table::obtain_between_in_filesystem(
	const std::string &lower_bound,
	const std::string &upper_bound,
	bool lower_bound_inclusive,
	bool upper_bound_inclusive)
{
    if (get_strategy() == storage_strategy::in_memory)
    {
	throw std::logic_error("access denied, invalid strategy");
    }

    if (upper_bound < lower_bound)
    {
	throw std::logic_error("upper bound less than lower?? puc puc..");
    }

    auto filename = table::_absolute_directory_name + get_instance_name() + _file_format;
    auto index_filename = table::_absolute_directory_name + "index_" + get_instance_name() + _file_format;

    std::ifstream index_file(index_filename);
    throw_if_not_open(index_file);

    std::vector<std::streamoff> index_array = load_index(index_filename);

    std::ifstream data_file(filename);
    throw_if_not_open(data_file);

    size_t left = 0;
    size_t right = index_array.size() - 1;
    std::string file_key;
    bool is_lower_found = false;
    while (left <= right)
    {
	size_t mid = left + (right - left) / 2;
	data_file.seekg(index_array[mid]);
	std::getline(data_file, file_key, '#');

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
	    right = mid - 1;
	}
    }

    if (!is_lower_found && left >= index_array.size() - 1)
    {
	data_file.close();
	throw std::logic_error("left bound is not exist");
    }

    size_t start_index = is_lower_found && (lower_bound_inclusive) ? left : left + 1 ;
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
	    std::string current_key = readln.substr(0, pos);

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

void table::dispose_from_filesystem(std::string const &key)
{
    if (get_strategy() != storage_strategy::filesystem)
    {
	throw std::logic_error("incorrect strategy for this method :((");
    }

    auto filename = table::_absolute_directory_name + get_instance_name() + _file_format;
    auto index_filename = table::_absolute_directory_name + "index_" + get_instance_name() + _file_format;

    std::vector<std::streamoff> index_array = load_index(index_filename);
    if (index_array.empty())
    {
	throw std::logic_error("Attemp to dispose from empty file");
    }

    std::ifstream src(filename);
    throw_if_not_open(src);

    size_t left = 0;
    size_t right = index_array.size() - 1;
    bool is_found = false;
    std::string file_key;
    while (left <= right)
    {
	size_t mid = left + (right - left) / 2;

	src.seekg(index_array[mid]);


	std::getline(src, file_key, '#');

	if (key == file_key)
	{
	    is_found = true;
	    break;
	}

	if (right == left)
	{
	    src.close();
	    throw std::logic_error("key not found");
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

    if (is_found && index_array.size() == 1)
    {
	src.close();
	std::ofstream clear_file(filename, std::ios::trunc);
	throw_if_not_open(clear_file);
	clear_file.close();
	decrease_index(index_array);
	save_index(index_array, index_filename);
	return;
    }

    src.seekg(0);
    std::string temp_filename = _absolute_directory_name + std::string{"temp"} + _file_format;

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
		continue;
	    }
	}

	tmp_file << src_line << std::endl;
    }

    index_array.pop_back();
    save_index(index_array, index_filename);
    src.close();
    tmp_file.close();

    //TODO: copy temp to src;

    std::string backup_filename = _absolute_directory_name + "backup_" + get_instance_name() + _file_format;

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
	throw_if_not_open(final_data_file);

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
	throw_if_not_open(src_file);
	src_file << backup_file.rdbuf();

	if (!backup_file.good() || !src_file.good())
	{
	    throw std::logic_error("smth went wrong puc puc..; back up went wrong");
	}

	backup_file.close();
	src_file.close();
    }
}

void table::insert_to_filesystem(const std::string &key, user_data &&value)
{
    if (get_strategy() == storage_strategy::in_memory)
    {
	throw std::logic_error("incorrect strategy");
    }
    std::string out_str = key + "#" + std::to_string(value.get_id()) + "#" + value.get_name() + "#" + value.get_surname() + "|";
    length_alignment(out_str);

    auto filename = table::_absolute_directory_name + get_instance_name() + _file_format;
    auto index_filename = table::_absolute_directory_name + "index_" + get_instance_name() + _file_format;

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

    std::string temp_filename = _absolute_directory_name + std::string{"temp"} + _file_format;

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
    std::string backup_filename = _absolute_directory_name + "backup_" + get_instance_name() + _file_format;

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
	throw_if_not_open(final_data_file);

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
	throw_if_not_open(src_file);
	src_file << backup_file.rdbuf();

	if (!backup_file.good() || !src_file.good())
	{
	    throw std::logic_error("smth went wrong puc puc..; back up went wrong");
	}

	backup_file.close();
	src_file.close();
    }
}

user_data &table::obtain(const std::string &key)
{
    return _data->obtain(key);
}

//std::streamoff table::binary_search_in_file(std::fstream &file, std::string const &key)

user_data table::obtain_in_filesystem(const std::string &key)
{
    if (get_strategy() == storage_strategy::in_memory)
    {
	throw std::logic_error("access denied, invalid strategy");
    }

    auto filename = table::_absolute_directory_name + get_instance_name() + _file_format;
    auto index_filename = table::_absolute_directory_name + "index_" + get_instance_name() + _file_format;

    std::ifstream index_file(index_filename);
    throw_if_not_open(index_file);

    std::vector<std::streamoff> index_array = load_index(index_filename);

    std::ifstream data_file(filename);
    throw_if_not_open(data_file);

    size_t left = 0;
    size_t right = index_array.size() - 1;

    while (left <= right)
    {
	size_t mid = left + (right - left) / 2;

	data_file.seekg(index_array[mid]);

	std::string file_key;
	std::getline(data_file, file_key, '#');
	std::cout << file_key << std::endl;
	if (key == file_key)
	{
	    std::string user_info;
	    std::getline(data_file, user_info, '|');
	    std::istringstream iss(user_info);
	    std::string id_str, name, surname;

	    std::getline(iss, id_str, '#');
	    std::getline(iss, name, '#');
	    std::getline(iss, surname, '#');

	    size_t id = std::stoul(id_str);

	    data_file.close();

	    return user_data(id, name, surname);
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
	    right = mid - 1;
	}
    }


    data_file.close();
    throw std::logic_error("key not found");
}

std::map<std::string, user_data> table::obtain_between(
	const std::string &lower_bound,
	const std::string &upper_bound,
	bool lower_bound_inclusive,
	bool upper_bound_inclusive)
{

    switch (this->get_strategy())
    {
	case storage_strategy::in_memory: {
	    auto collection = _data->obtain_between(lower_bound, upper_bound, lower_bound_inclusive, upper_bound_inclusive);
	    std::map<std::string, user_data> result_map;

	    size_t length = collection.size();

	    for (auto &i: collection)
	    {
		auto key = i.key;
		auto value = i.value;
		result_map.emplace(key, value);
	    }
	    break;
	}
	case storage_strategy::filesystem: {
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

void table::serialize()
{
    std::ofstream output_file(table::_absolute_directory_name + get_instance_name() + _file_format);
    if (!output_file.is_open())
    {
	error_with_guard("file for serializing did not open! file_name: [ " + _storage_filename + " ]");
	throw std::runtime_error("file did not open");
    }

    std::ofstream index_file(table::_absolute_directory_name + "index_" + get_instance_name() + _file_format);
    if (!index_file.is_open())
    {
	output_file.close();
	throw std::runtime_error("file did not open");
    }

    auto it = _data->begin_infix();
    auto it_end = _data->end_infix();

    size_t offset = 0;
    size_t counter = 0;
    while (it != it_end)
    {
	auto string_key = std::get<2>(*it);
	auto user_data_value = std::get<3>(*it);

	auto serialized_line = string_key + "#" + std::to_string(user_data_value.get_id()) + "#" + user_data_value.get_name() + "#" + user_data_value.get_surname() + "|";
	length_alignment(serialized_line);
	auto length = serialized_line.length();
	output_file << serialized_line << std::endl;
	++it;
	++counter;
	/*
	std::string line_for_index = std::to_string(offset) + "#";
	index_file << line_for_index;
	offset += length + SIZEOF_NEXT_LINE_SYMB;
	*/
    }
    index_file << counter << "#" << std::endl;
    index_file.close();
    output_file.close();
}

void table::deserialize()
{
    std::ifstream input_file(_absolute_directory_name + get_instance_name() + _file_format);
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
    std::string filename_copy = filename.length() == 0
					? get_instance_name()
					: filename;

    auto index_filename = _absolute_directory_name + "index_" + filename_copy + _file_format;

    std::ofstream index_file(index_filename);
    throw_if_not_open(index_file);

    std::string src_filename = _absolute_directory_name + filename_copy + _file_format;
    std::ofstream output_file(src_filename);
    if (!output_file.is_open())
    {
	index_file.close();
	error_with_guard("file for serializing did not open! file_name: [ " + filename_copy + " ]");
	throw std::runtime_error("file for serializing did not open!");
    }

    auto it = _data->begin_infix();
    auto it_end = _data->end_infix();

    size_t offset = 0;
    size_t counter = 0;
    while (it != it_end)
    {
	auto string_key = std::get<2>(*it);
	auto user_data_value = std::get<3>(*it);

	auto serialized_line = string_key + "#" + std::to_string(user_data_value.get_id()) + "#" + user_data_value.get_name() + "#" + user_data_value.get_surname() + "|";
	length_alignment(serialized_line);
	auto length = serialized_line.length();
	output_file << serialized_line << std::endl;
	++it;
	++counter;
	/*
	std::string line_for_index = std::to_string(offset) + "#";
	index_file << line_for_index;
	offset += length + SIZEOF_NEXT_LINE_SYMB;
	*/
    }
    index_file << counter << "#" << std::endl;
    index_file.close();
    output_file.close();
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

    std::vector<std::streamoff> index_array;
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