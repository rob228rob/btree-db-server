//
// Created by rob22 on 04.05.2024.
//

#ifndef CW_OS_USER_DATA_H
#define CW_OS_USER_DATA_H

#include "../../string_pool/include/string_pool.h"
#include <chrono>
#include <map>
#include <memory>
#include <string>

class user_data
{
private:
    std::map<std::chrono::seconds, std::unique_ptr<user_data>> _differs;

    enum class COMMAND
    {
	INSERT_COMMAND,
	DISPOSE_COMMAND,
	UPDATE_COMMAND
    };

    class ud_obj
    {
	friend class user_data;
	size_t _id;
	size_t _name_ind;
	size_t _surname_ind;

	ud_obj() = default;

	ud_obj(size_t id, std::string const &name, std::string const &surname)
	    : _id(id), _name_ind(string_pool::add_string(name, true)), _surname_ind(string_pool::add_string(surname, true))
	{
	}

    };

private:
    ud_obj _current_state;

public:
    user_data(const std::string &id, const std::string &name, const std::string &surname);

    explicit user_data(size_t id, const std::string &name, const std::string &surname);

    explicit user_data(size_t id, std::string &&name, std::string &&surname);

    user_data() = default;

    user_data(const user_data &other);

    user_data(user_data &&other) noexcept;

    user_data &operator=(const user_data &other);

    user_data &operator=(user_data &&other) noexcept;

    ~user_data();

public:
    size_t get_id() const;

    size_t get_name_ind() const
    {
	return _current_state._name_ind;
    }

    size_t get_surname_ind() const
    {
	return _current_state._surname_ind;
    }

    [[nodiscard]] const std::string &get_name_value() const
    {
	return string_pool::get_string(_current_state._name_ind);
    }

    [[nodiscard]] const std::string &get_surname_value() const
    {
	return string_pool::get_string(_current_state._surname_ind);
    }

    std::string get_name() const
    {
	return std::to_string(_current_state._name_ind);
    }

    std::string get_surname() const
    {
	return std::to_string(_current_state._surname_ind);
    }

    void set_id(size_t id);

    void set_name(const std::string &name)
    {
	_current_state._name_ind = string_pool::add_string(name);
    }

    void set_surname(const std::string &surname)
    {
	_current_state._surname_ind = string_pool::add_string(surname);
    }

    void create_user_data_from_str(const std::string &ud_line, char delim = '#');

    [[nodiscard]] std::string to_string() const noexcept;

    void create_ud_from_string_pool_line(const std::string &ud_line, char delim);
};

#endif//CW_OS_USER_DATA_H