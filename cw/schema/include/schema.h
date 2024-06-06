//
// Created by rob22 on 05.05.2024.
//

#ifndef CW_OS_SCHEMA_H
#define CW_OS_SCHEMA_H

#include "../../../b-tree/b_tree.h"
#include "../../common/include/storage_interface.h"
#include "../../table/include/table.h"
#include "../../user_data/include/user_data.h"
#include <memory>
#include <string>
#include <vector>


class schema :
    public storage_interface<std::string, table>
{
    friend class data_base;

private:

    static std::function<int(const std::string& , const std::string&)> _default_string_comparer;

    std::string _storage_filename;

    std::unique_ptr<b_tree<std::string, table>> _data;

public:

    explicit schema(std::size_t t,
		   allocator* allocator = nullptr,
		   logger* logger = nullptr,
		   const std::function<int(const std::string&, const std::string&)>& keys_comparer = _default_string_comparer,
		    storage_strategy storaged_strategy = storage_strategy::in_memory);

public:

    void insert(const std::string &key, const table &value) override;

    void insert(const std::string &key, table &&value) override;

    table &obtain(const std::string &key) override;

    void update(const std::string &key, const table &value) override;

    void update(const std::string &key, table &&value) override;

    std::map<std::string, table> obtain_between(std::string const &lower_bound, std::string const &upper_bound, bool lower_bound_inclusive, bool upper_bound_inclusive) override;

    void dispose(const std::string &key) override;

public:

    schema();

    ~schema() override;

    schema(const schema& other);

    schema(schema&& other) noexcept;

    schema& operator=(const schema& other);

    schema& operator=(schema&& other) noexcept;

private:

    void insert_schema_to_filesystem(std::filesystem::path const &path);

    //TODO: Call serialize in destructor and deserialize in constructor, and Private access
    void serialize() override;

    void deserialize() override;

    void set_storage_filename(std::string &filename);

    void set_storage_filename(std::string &&filename);

    void print_table();

public:

    schema load_schema_from_filesystem(std::string const &filename = "");

    void save_schema_to_filesystem(std::string const &filename = "");
};


#endif//CW_OS_SCHEMA_H
