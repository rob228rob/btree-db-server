//
// Created by rob22 on 04.05.2024.
//
#include "../include/user_data.h"

user_data::user_data(size_t id, const std::string &name, const std::string &surname)
    : _id(id), _name(name), _surname(surname)
{}

user_data::user_data(size_t id, std::string &&name, std::string &&surname)
    : _id(id), _name(std::move(name)), _surname(std::move(surname))
{}

user_data::user_data(const user_data &other)
    : _id(other._id), _name(other._name), _surname(other._surname)
{}

user_data::user_data(user_data &&other) noexcept
    : _id(other._id), _name(std::move(other._name)), _surname(std::move(other._surname))
{}

user_data &user_data::operator=(const user_data &other)
{
    if (this != &other)
    {
	_id = other._id;
	_name = other._name;
	_surname = other._surname;
    }
    return *this;
}

user_data &user_data::operator=(user_data &&other) noexcept
{
    if (this != &other)
    {
	_id = other._id;
	_name = std::move(other._name);
	_surname = std::move(other._surname);
    }
    return *this;
}

user_data::~user_data()
{

}

size_t user_data::get_id() const
{
    return _id;
}

std::string user_data::get_name() const
{
    return _name;
}

std::string user_data::get_surname() const
{
    return _surname;
}

void user_data::set_id(size_t id)
{
    _id = id;
}

void user_data::set_name(std::string &name)
{
    _name = name;
}

void user_data::set_name(std::string &&name)
{
    _name = std::move(name);
}

void user_data::set_surname(std::string &surname)
{
    _surname = surname;
}
void user_data::set_surname(std::string &&surname)
{
    _surname = std::move(surname);
}
