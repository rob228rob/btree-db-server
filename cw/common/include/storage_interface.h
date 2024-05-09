//
// Created by rob22 on 04.05.2024.
//

#ifndef CW_OS_STORAGE_INTERFACE_H
#define CW_OS_STORAGE_INTERFACE_H

#include "../../../associative_container/include/associative_container.h"
#include <memory>
#include <vector>

template<
	typename tkey,
	typename tvalue>
class storage_interface : public logger_guardant,
			  public allocator_guardant
{

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


private:
    allocator *_allocator;

    logger *_logger;

public:
    enum class data_storage_strategy
    {
	in_memory_storaged,
	filesystem_storaged
    };

protected:
    data_storage_strategy _storaged_strategy;

    storage_interface<tkey, tvalue>::data_storage_strategy get_strategy() noexcept;

    void set_strategy(data_storage_strategy storaged_strategy);

public:
    virtual void insert(const tkey &key, const tvalue &value) = 0;

    virtual void insert(const tkey &key, tvalue &&value) = 0;

    virtual const tvalue &obtain(const tkey &key) = 0;

    virtual std::vector<typename associative_container<tkey, tvalue>::key_value_pair> obtain_between(
	    tkey const &lower_bound,
	    tkey const &upper_bound,
	    bool lower_bound_inclusive,
	    bool upper_bound_inclusive) = 0;

    virtual void dispose(const tkey &key) = 0;

    virtual ~storage_interface() = default;

protected:
    virtual void serialize() = 0;

    virtual void deserialize() = 0;

    logger *get_logger() const override;

    allocator *get_allocator() const override;
};

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
typename storage_interface<tkey, tvalue>::data_storage_strategy storage_interface<tkey, tvalue>::get_strategy() noexcept
{
    return _storaged_strategy;
}

template<typename tkey, typename tvalue>
void storage_interface<tkey, tvalue>::set_strategy(data_storage_strategy storaged_strategy)
{
    _storaged_strategy = storaged_strategy;
}

#endif//CW_OS_STORAGE_INTERFACE_H
