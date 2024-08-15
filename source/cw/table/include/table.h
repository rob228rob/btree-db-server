//
// Created by rob22 on 04.05.2024.
//

#ifndef CW_OS_TABLE_H
#define CW_OS_TABLE_H

#include "../../../b-tree/b_tree.h"
#include "../../common/include/storage_interface.h"
#include "../../user_data/include/user_data.h"
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

class table :
    public storage_interface<std::string, user_data>
{
private:

    friend class data_base;

    friend class schema;

private:

    std::string _storage_filename;

    std::unique_ptr<b_tree<std::string, user_data>> _data;

public:

    explicit table(std::size_t t,
		   allocator* allocator = nullptr,
		   logger* logger = nullptr,
		   const std::function<int(const std::string&, const std::string&)>& keys_comparer = _default_string_comparer,
		   storage_strategy storage_strategy = storage_strategy::filesystem,
		   std::string instance_name = "table_name");

public:

    table();

    ~table() override;

    table(const table& other);

    table(table&& other) noexcept;

    table& operator=(const table& other);

    table& operator=(table&& other) noexcept;

private:

    static void delete_backup(const std::filesystem::path &source_path);

    static void create_backup(const std::filesystem::path &source_path);

    static bool check_and_create_with_insertion(const std::filesystem::path &path, const std::filesystem::path &index_filename, const std::string &out_str);

    static bool check_and_create_empty(const std::filesystem::path &path, const std::filesystem::path &index_filename);

    static void insert_ud_to_filesystem(std::filesystem::path const &path, std::filesystem::path const &index_path, std::string const &key, user_data &&ud);

    static void insert_ud_to_filesystem(std::filesystem::path const &path, std::filesystem::path const &index_path, std::string const &key, user_data const &ud);

public:

    static std::function<int(const std::string& , const std::string&)> _default_string_comparer;

    void insert(const std::string &key, const user_data &value) override;

    void insert(const std::string &key, user_data &&value) override;

    user_data &obtain(const std::string &key) override;

    std::map<std::string, user_data> obtain_between(std::string const &lower_bound, std::string const &upper_bound, bool lower_bound_inclusive, bool upper_bound_inclusive) override;

    void update(const std::string &key, const user_data &value) override;

    void update(const std::string &key, user_data &&value) override;

    void dispose(const std::string &key) override;

    user_data obtain_in_filesystem(const std::string &key);

private:

    void insert_table_to_filesystem(std::filesystem::path const &path);

    void insert_to_filesystem(const std::string &key, user_data &&value);

    void insert_to_filesystem(const std::string &key, const user_data &value);

    static void dispose_ud_from_filesystem(std::filesystem::path const &path, std::filesystem::path const &index_path, std::string const &key);

    static void update_ud_in_filesystem(const std::filesystem::path &table_path, const std::filesystem::path &index_table_path, const std::string &user_data_key, user_data &&value);

    void update_in_filesystem(const std::string &key, user_data &&value);

    void update_in_filesystem(const std::string &key, const user_data &value);

    std::map<std::string, user_data> obtain_between_in_filesystem(const std::string &lower_bound, const std::string &upper_bound, bool lower_bound_inclusive, bool upper_bound_inclusive);

    static void update_index(std::vector<std::streamoff> &vec);

    static void save_index(std::vector<std::streamoff> const &vec, std::string const &filename);

    static void decrease_index(std::vector<std::streamoff> &vec);

    static std::vector<std::streamoff> load_index(const std::string &index_filename);

    static void throw_if_not_open(std::ifstream const &file);

    static void throw_if_not_open(std::ofstream const &file);

private:
    //TODO: Call serialize in destructor and deserialize in constructor, and Private access
    void serialize() override;

    void deserialize() override;

    void copy_to_temp_file(const std::string& source_path, const std::string& temp_path, std::string const &key);

public:

    void set_storage_filename(std::string &filename);

    void set_storage_filename(std::string &&filename);

    void print_table();

    static table load_data_from_filesystem(std::string const &filename = "");

    void save_data_to_filesystem(std::string const &filename = "");

    void copy_file(const std::string &source_path, const std::string &dest_path);

    static user_data create_user_data(const std::string &ud_line);

    [[maybe_unused]] static int get_index_by_bin_search(std::ifstream &src, const std::vector<std::streamoff> &index_array, const std::string &key);

private:

    static void load_backup(const std::filesystem::path &source_path);

    static std::map<std::string, user_data> obtain_all_ud_in_filesystem(const std::filesystem::path &filepath, const std::filesystem::path &index_filepath);

    static std::map<std::string, user_data> obtain_between_ud_in_filesystem(
	    std::filesystem::path const &filepath,
	    std::filesystem::path const &index_filepath,
	    std::string const &lower_bound,
	    std::string const &upper_bound,
	    bool lower_bound_inclusive,
	    bool upper_bound_inclusive);


};

#endif//CW_OS_TABLE_H