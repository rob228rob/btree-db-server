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
	std::string _name;
	std::string _surname;

	ud_obj() = default;

	ud_obj(size_t id, std::string const &name, std::string const &surname) : _id(id), _name(name), _surname(surname)
	{

	}

    };

private:

    ud_obj _current_state;

public:
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

    const std::string& get_name() const {
	return string_pool::get_string(_current_state._name);
    }

    const std::string& get_surname() const {
	return string_pool::get_string(_current_state._surname);
    }

    void set_id(size_t id);

    void set_name(const std::string& name) {
	_current_state._name = string_pool::get_string(name);
    }

    void set_surname(const std::string& surname) {
	_current_state._surname = string_pool::get_string(surname);
    }

    void create_user_data_from_str(const std::string &ud_line, char delim = '#');

    std::string to_string() const noexcept;
};

#endif//CW_OS_USER_DATA_H