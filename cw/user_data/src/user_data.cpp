//
// Created by rob22 on 04.05.2024.
//
#include "../include/user_data.h"
#include <sstream>
#include <stdexcept>

user_data::user_data(const std::string &id, const std::string &name,const std::string &surname)
{
    _current_state._id = std::stol(id);
    _current_state._name_ind = string_pool::add_string(string_pool::get_string(std::stol(name)));
    _current_state._surname_ind = string_pool::add_string(string_pool::get_string(std::stol(surname)));
}

user_data::user_data(size_t id, const std::string &name, const std::string &surname)
{
    _current_state._id = id;
    _current_state._name_ind = string_pool::add_string(name);
    _current_state._surname_ind = string_pool::add_string(surname);
}

user_data::user_data(size_t id, std::string &&name, std::string &&surname)
{
    _current_state._id = id;
    _current_state._name_ind = string_pool::add_string(name);
    _current_state._surname_ind = string_pool::add_string(surname);
}

user_data::user_data(const user_data &other)
{
    _current_state._id = other.get_id();
    _current_state._name_ind = other.get_name_ind();
    _current_state._surname_ind = other.get_surname_ind();
}

user_data::user_data(user_data &&other) noexcept
{
    _current_state._id = other.get_id();
    _current_state._name_ind = other.get_name_ind();
    _current_state._surname_ind = other.get_surname_ind();
}

user_data &user_data::operator=(const user_data &other)
{
    if (this != &other)
    {
	_current_state._id = other.get_id();
	_current_state._name_ind = other.get_name_ind();
	_current_state._surname_ind = other.get_surname_ind();
    }
    return *this;
}

user_data &user_data::operator=(user_data &&other) noexcept
{
    if (this != &other)
    {
	_current_state._id = other.get_id();
	_current_state._name_ind = other.get_name_ind();
	_current_state._surname_ind = other.get_surname_ind();
    }
    return *this;
}

user_data::~user_data() = default;

size_t user_data::get_id() const
{
    return _current_state._id;
}

std::string user_data::to_string() const noexcept
{
    std::string out = std::to_string(get_id()) + " " + get_name_value() + " " + get_surname_value();

    return out;
}

void user_data::create_ud_from_string_pool_line(const std::string &ud_line, char delim)
{
    if (ud_line.empty())
    {
	throw std::logic_error("attempt to map empty line");
    }

    std::istringstream iss(ud_line);
    std::string id_str, name, surname;

    std::getline(iss, id_str, delim);
    std::getline(iss, name, delim);
    std::getline(iss, surname);

    size_t id = std::stoul(id_str);

    set_id(id);
    set_name(name);
    set_surname(surname);
}

void user_data::create_user_data_from_str(const std::string &ud_line, char delim)
{
    if (ud_line.empty())
    {
	throw std::logic_error("attempt to map empty line");
    }

    std::istringstream iss(ud_line);
    std::string id_str, name, surname;

    std::getline(iss, id_str, delim);
    std::getline(iss, name, delim);
    std::getline(iss, surname);

    size_t id = std::stoul(id_str);

    set_id(id);
    set_name(name);
    set_surname(surname);
}
void user_data::set_id(size_t id)
{
    _current_state._id = id;
}
