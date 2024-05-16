//
// Created by rob22 on 04.05.2024.
//

#ifndef CW_OS_STORAGE_INTERFACE_H
#define CW_OS_STORAGE_INTERFACE_H

#define SIZEOF_NEXT_LINE_SYMB 2

#include "../../../associative_container/include/associative_container.h"
#include <fstream>
#include <memory>
#include <vector>

template<
	typename tkey,
	typename tvalue>
class storage_interface : public logger_guardant,
			  public allocator_guardant
{

protected:
    inline static const size_t max_data_length = 40;

    inline static const size_t index_item_max_length = max_data_length + SIZEOF_NEXT_LINE_SYMB;

    static void length_alignment(std::string &source);

    static void throw_if_exceeds_length_limit(std::string const &str);

protected:
    inline static std::string _file_format = ".txt";

    std::string _instance_name;

protected:
    void set_instance_name(std::string const &name)
    {
	_instance_name = name;
    }

public:
    std::string get_instance_name()
    {
	return _instance_name;
    }


protected:
    allocator *_allocator;

    logger *_logger;

public:
    enum class storage_strategy
    {
	in_memory,
	filesystem
    };

protected:
    storage_strategy _storaged_strategy;

    storage_interface<tkey, tvalue>::storage_strategy get_strategy() noexcept;

    void set_strategy(storage_strategy storaged_strategy);

public:
    virtual void insert(const tkey &key, const tvalue &value) = 0;

    virtual void insert(const tkey &key, tvalue &&value) = 0;

    virtual tvalue &obtain(const tkey &key) = 0;

    virtual std::map<tkey, tvalue> obtain_between(
	    tkey const &lower_bound,
	    tkey const &upper_bound,
	    bool lower_bound_inclusive,
	    bool upper_bound_inclusive) = 0;

    virtual void update(const tkey &key, const tvalue &value) = 0;

    virtual void update(const tkey &key, tvalue &&value) = 0;

    virtual void dispose(const tkey &key) = 0;

    virtual ~storage_interface() = default;

protected:
    virtual void serialize() = 0;

    virtual void deserialize() = 0;

    logger *get_logger() const override;

    allocator *get_allocator() const override;

protected:

    static std::vector<std::streamoff> load_index(const std::string &index_filename);

    static void save_index(const std::vector<std::streamoff> &vec, const std::string &filename);

    static void update_index(std::vector<std::streamoff> &vec);

    static void decrease_index(std::vector<std::streamoff> &vec);

    static void throw_if_not_open(const std::ofstream &file);

    static void throw_if_not_open(const std::ifstream &file);

    static int get_index_by_bin_search(std::ifstream &src, const std::vector<std::streamoff> &index_array, const std::string &key);
};

template<typename tkey, typename tvalue>
void storage_interface<tkey, tvalue>::throw_if_exceeds_length_limit(const std::string &str)
{
    if (str.length() > max_data_length)
    {
	throw std::logic_error("too long , sorry, bruh :((");
    }
}

template<typename tkey, typename tvalue>
void storage_interface<tkey, tvalue>::length_alignment(std::string &source)
{
    throw_if_exceeds_length_limit(source);

    auto difference = max_data_length - source.length();

    std::string dummy;
    char stuff_symb = '_';
    for (int i = 0; i < difference; ++i)
    {
	dummy.push_back(stuff_symb);
    }

    source = source + dummy;
}

template<typename tkey, typename tvalue>
allocator *storage_interface<tkey, tvalue>::get_allocator() const
{
    return _allocator;
}

template<typename tkey, typename tvalue>
logger *storage_interface<tkey, tvalue>::get_logger() const
{
    return _logger;
}

template<typename tkey, typename tvalue>
typename storage_interface<tkey, tvalue>::storage_strategy storage_interface<tkey, tvalue>::get_strategy() noexcept
{
    return _storaged_strategy;
}

template<typename tkey, typename tvalue>
void storage_interface<tkey, tvalue>::set_strategy(storage_strategy storaged_strategy)
{
    _storaged_strategy = storaged_strategy;
}

template<typename tkey, typename tvalue>
void storage_interface<tkey, tvalue>::throw_if_not_open(std::ifstream const &file)
{
    if (!file.is_open())
    {
	throw std::runtime_error("file did not open!!1!!!111!1");
    }
}

template<typename tkey, typename tvalue>
void storage_interface<tkey, tvalue>::throw_if_not_open(std::ofstream const &file)
{
    if (!file.is_open())
    {
	throw std::runtime_error("file did not open!00!!))!1!!!");
    }
}
template<typename tkey, typename tvalue>
void storage_interface<tkey, tvalue>::decrease_index(std::vector<std::streamoff> &vec)
{
    if (vec.empty())
    {
	return;
    }

    vec.pop_back();
}

template<typename tkey, typename tvalue>
void storage_interface<tkey, tvalue>::update_index(std::vector<std::streamoff> &vec)
{
    if (vec.empty())
    {
	vec.push_back(0);
	return;
    }

    vec.push_back(vec.back() + index_item_max_length);
}

template<typename tkey, typename tvalue>
void storage_interface<tkey, tvalue>::save_index(std::vector<std::streamoff> const &vec, std::string const &filename)
{
    std::ofstream file(filename, std::ios::trunc);
    throw_if_not_open(file);

    size_t size = vec.size();
    file << size << "#" << std::endl;

    file.close();
}

template<typename tkey, typename tvalue>
std::vector<std::streamoff> storage_interface<tkey, tvalue>::load_index(std::string const &index_filename)
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

template<typename tkey, typename tvalue>
int storage_interface<tkey, tvalue>::get_index_by_bin_search(std::ifstream &src, std::vector<std::streamoff> const &index_array, std::string const &key)
{
    throw_if_not_open(src);

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

#endif//CW_OS_STORAGE_INTERFACE_H
