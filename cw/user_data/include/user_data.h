//
// Created by rob22 on 04.05.2024.
//

#ifndef CW_OS_USER_DATA_H
#define CW_OS_USER_DATA_H

#include <string>

class user_data
{
private:

    size_t _id;

    std::string _name;

    std::string _surname;

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

    void set_id(size_t id);

    void set_name(std::string &name);

    void set_surname(std::string &surname);

    void set_name(std::string &&name);

    void set_surname(std::string &&surname);

    size_t get_id() const;

    std::string get_name() const;

    std::string get_surname() const;
};

#endif//CW_OS_USER_DATA_H