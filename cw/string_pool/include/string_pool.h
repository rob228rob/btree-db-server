//
// Created by rob22 on 22.05.2024.
//

#ifndef CW_OS_STRING_POOL_H
#define CW_OS_STRING_POOL_H

#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>

class string_pool final
{
private:
    static std::set<std::string> _pool;

    static string_pool* _instance;

    static std::string _storage_filename;

    static std::fstream _file;

    static std::mutex _mutex;

    void load_from_file()
    {
	std::lock_guard<std::mutex> lock(_mutex);

	if (!_file.is_open())
	{
	    throw std::runtime_error("Cannot open file for reading: " + _storage_filename);
	}

	std::string line;
	while (std::getline(_file, line))
	{
	    size_t pos = line.find('#');
	    std::string substring = line.substr(0, pos);
	    _pool.emplace(substring);
	}
    }

    void save_to_file()
    {
	std::lock_guard<std::mutex> lock(_mutex);

	_file.seekg(0);

	if (!_file.is_open())
	{
	    throw std::runtime_error("Cannot open file for writing: " + _storage_filename);
	}

	for (const auto &pair: _pool)
	{
	    _file << (pair) << '\n';
	}
    }

    void obtain_in_file()
    {

    }

    string_pool()
    {
	auto path = std::filesystem::absolute(_storage_filename);

	if (!std::filesystem::exists(path))
	{
	    std::ofstream ofs(path);
	    if (!ofs)
	    {
		std::string error_message = "File could not be created: ";
		error_message += std::strerror(errno);
		throw std::runtime_error(error_message);
	    }

	    ofs.close();
	}

	_file.open(path, std::ios::out | std::ios::in);
	if (!_file.is_open())
	{
	    std::string error_message = "File did not open: ";
	    error_message += std::strerror(errno);
	    throw std::runtime_error(error_message);
	}

	load_from_file();
    }

    ~string_pool()
    {
	std::lock_guard<std::mutex> lock(_mutex);
	if (_file.is_open())
	{
	    _file.close();
	}

	delete _instance;
	//save_to_file();
    }

    //TODO:: CREATE LOGIC TO WRITE READ AND DELETE IN RUNTIME!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

public:
    static string_pool &get_instance()
    {

	if (!_instance)
	{
	    _instance = new string_pool;
	}

	return *_instance;
    }

    string_pool(const string_pool &) = delete;

    string_pool(string_pool &&) = delete;

    string_pool &operator=(const string_pool &) = delete;

    string_pool &operator=(string_pool &&) = delete;

public:

    static const std::string &get_string(const std::string &str)
    {
	auto& pool = get_instance();

	auto it = string_pool::_pool.find(str);

	if (it == string_pool::_pool.end())
	{
	    string_pool::_pool.emplace(str);
	    add_string_to_file(str);

	    const auto &result = *(_pool.find(str));
	    return result;
	}

	return *(it);
    }

private:

    static std::string find_in_file(std::string const &target_string)
    {
	_file.seekg(0);

	std::string line;
	bool is_found = false;
	while (std::getline(_file, line))
	{
	    size_t pos = line.find('#');
	    std::string substring = line.substr(0, pos);

	    if (substring == target_string)
	    {
		is_found = true;
	    }
	}

	return is_found ? line : "" ;
    }

    static int add_string_to_file(const std::string &str)
    {
	if (str.empty())
	{
	    throw std::logic_error("String pool can't storage empty string :(");
	}

	_file << str  + "#" << '\n';
	_pool.emplace(str);

	return 0;
    }
};



#endif//CW_OS_STRING_POOL_H
