//
// Created by rob22 on 04.05.2024.
//

#ifndef CW_OS_TABLE_H
#define CW_OS_TABLE_H

#include "../../../b-tree/b_tree.h"
#include "../../common/include/storage_interface.h"
#include "../../user_data/include/user_data.h"
#include <memory>
#include <string>
#include <vector>

class table :
    public storage_interface<std::string, user_data>
//TODO: inheritance the logger_guardant and allocator_guardant
{
private:

    inline static size_t _table_uid = 0;

    static std::function<int(const std::string& , const std::string&)> _default_string_comparer;

    //TODO: move to builder (may be)
    std::string _storage_filename;

    std::unique_ptr<b_tree<std::string, user_data>> _data;

    inline static std::string _absolute_directory_name = "C:\\Users\\rob22\\CLionProjects\\cw_os\\cw\\filesystem\\tables\\";

public:

    explicit table(std::size_t t,
		   allocator* allocator = nullptr,
		   logger* logger = nullptr,
		   const std::function<int(const std::string&, const std::string&)>& keys_comparer = _default_string_comparer,
		   data_storage_strategy storage_strategy = data_storage_strategy::filesystem_storaged,
		   std::string instance_name = "table_name");

public:

    table();

    ~table() override;

    table(const table& other);

    table(table&& other) noexcept;

    table& operator=(const table& other);

    table& operator=(table&& other) noexcept;

public:

    void insert(const std::string &key, const user_data &value) override;

    void insert(const std::string &key, user_data &&value) override;

    const user_data &obtain(const std::string &key) override;

    std::vector<typename associative_container<std::string, user_data>::key_value_pair> obtain_between(std::string const &lower_bound, std::string const &upper_bound, bool lower_bound_inclusive, bool upper_bound_inclusive) override;

    void dispose(const std::string &key) override;

private:
    //TODO: Call serialize in destructor and deserialize in constructor, and Private access
    void serialize() override;

    void deserialize() override;

public:

    void set_storage_filename(std::string &filename);

    void set_storage_filename(std::string &&filename);

    void print_table();

    table load_data_from_filesystem(std::string const &filename = "");

    void save_data_to_filesystem(std::string const &filename = "");
};

#endif//CW_OS_TABLE_H