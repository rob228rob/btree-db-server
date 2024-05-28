//
// Created by rob22 on 04.05.2024.
//

#ifndef CW_OS_DATA_BASE_H
#define CW_OS_DATA_BASE_H

#include "../../../b-tree/b_tree.h"
#include "../../schemas_pool/include/schemas_pool.h"
#include "../../common/include/storage_interface.h"
#include "iostream"
#include <set>
#include <filesystem>

class data_base : public storage_interface<std::string, schemas_pool>
{
public:

    static std::function<int(const std::string &, const std::string &)> _default_string_comparer;



private:

    friend class string_pool;

    std::filesystem::path _instance_path;

    friend class table;

    friend class schema;

    friend class schema_pool;

    static inline std::string _instances_filename = "../../filesystem/instances/instances_list.txt";

private:

    //TODO: create logic to check duplicate names
    static std::set<std::string> _instance_names;

    std::unique_ptr<b_tree<std::string, schemas_pool>> _data;

private:

    data_base();

public:

    explicit data_base(std::string const &instance_name, storage_strategy _strategy = storage_strategy::in_memory, allocator* allocator = nullptr);

    ~data_base() override;

    data_base(const data_base &other) = delete;

    data_base(data_base &&other) noexcept;

    data_base &operator=(const data_base &other) = delete;

    data_base &operator=(data_base &&other) noexcept;

private:

    static void throw_if_key_invalid(std::string const &key);

public:

    void save_data_base_state();

    void load_data_base_state();

    void start_console_dialog();

    void execute_command_from_file(std::string const &filename);

    void insert_schemas_pool(
	    std::string const &pool_name,
	    schemas_pool const &value);

    void insert_schemas_pool(
	    std::string const &pool_name,
	    schemas_pool &&value);

    void insert_schema(
	    std::string const &pool_name,
	    std::string const &schema_name,
	    schema const &value);

    void insert_schema(
	    std::string const &pool_name,
	    std::string const &schema_name,
	    schema &&value);

    void insert_table(
	    std::string const &pool_name,
	    std::string const &schema_name,
	    std::string const &table_name,
	    table const &value);

    void insert_table(
	    std::string const &pool_name,
	    std::string const &schema_name,
	    std::string const &table_name,
	    table &&value);

    void insert_data(
	    std::string const &pool_name,
	    std::string const &schema_name,
	    std::string const &table_name,
	    std::string const &user_data_key,
	    user_data &&value);

    void insert_data(
	    std::string const &pool_name,
	    std::string const &schema_name,
	    std::string const &table_name,
	    std::string const &user_data_key,
	    user_data const &value);

    user_data obtain_data(
	    std::string const &pool_name,
	    std::string const &schema_name,
	    std::string const &table_name,
	    std::string const &user_data_key);

    std::map<std::string, user_data> obtain_between_data(
	    std::string const &pool_name,
	    std::string const &schema_name,
	    std::string const &table_name,
	    std::string const &lower_bound,
	    std::string const &upper_bound,
	    bool lower_bound_inclusive,
	    bool upper_bound_inclusive);

    void update_data(std::string const &pool_name,
		     std::string const &schema_name,
		     std::string const &table_name,
		     std::string const &user_data_key,
		     user_data const &value);

    void update_data(std::string const &pool_name,
		     std::string const &schema_name,
		     std::string const &table_name,
		     std::string const &user_data_key,
		     user_data &&value);

    void dispose_schemas_pool(
	    std::string const &pool_name);

    void dispose_schema(
	    std::string const &pool_name,
	    std::string const &schema_name);

    void dispose_table(
	    std::string const &pool_name,
	    std::string const &schema_name,
	    std::string const &table_name);

    void dispose_user_data(
	    std::string const &pool_name,
	    std::string const &schema_name,
	    std::string const &table_name,
	    std::string const &user_data_key);

private:

    void update(const std::string &key, const schemas_pool &value) override;

    void update(const std::string &key, schemas_pool &&value) override;

    void insert(const std::string &key, const schemas_pool &value) override;

    void insert(const std::string &key, schemas_pool &&value) override;

    schemas_pool &obtain(const std::string &key) override;

    std::map<std::string, schemas_pool> obtain_between(std::string const &lower_bound, std::string const &upper_bound, bool lower_bound_inclusive, bool upper_bound_inclusive) override;

    void dispose(const std::string &key) override;


private:

    void serialize() override;

    void deserialize() override;

private:

    void add_pool_to_filesystem(
	    const std::string &pool_name);

    void add_schema_to_filesystem(
	    std::string const &pool_name,
	    std::string const &schema_name);

    void add_table_to_filesystem(
	    std::string const &pool_name,
	    std::string const &schema_name,
	    std::string const &table_name);

    void insert_data_to_filesystem(
	    std::string const &pool_name,
	    std::string const &schema_name,
	    std::string const &table_name,
	    std::string const &user_data_key,
	    user_data &&value);

    void insert_data_to_filesystem(
	    std::string const &pool_name,
	    std::string const &schema_name,
	    std::string const &table_name,
	    std::string const &user_data_key,
	    user_data const &value);

    void update_ud_in_filesystem(
	    std::string const &schemas_pool_name,
	    std::string const &schema_name,
	    std::string const &table_name,
	    std::string const &user_data_key,
	    user_data &&value);


    void update_ud_in_filesystem(
	    const std::string &schemas_pool_name,
	    const std::string &schema_name,
	    const std::string &table_name,
	    const std::string &user_data_key,
	    const user_data &value);

    void dispose_pool_in_filesystem(std::string const &pool_name);

    void dispose_schema_in_filesystem(std::string const &schemas_pool_name, std::string const &schema_name);

    void dispose_table_in_filesystem(std::string const &schemas_pool_name, std::string const &schema_name, std::string const &table_name);

    void dispose_ud_in_filesystem(std::string const &schemas_pool_name, std::string const &schema_name, std::string const &table_name, std::string const &user_data_key);

private:

    void insert_pool_to_filesystem(
	    const std::string &pool_name, schemas_pool &&value);

    void insert_pool_to_filesystem(const std::string &pool_name, const schemas_pool &value);

    void insert_schema_to_filesystem(
	    std::string const &pool_name,
	    std::string const &schema_name,
	    schema const &value);

    void insert_schema_to_filesystem(
	    std::string const &pool_name,
	    std::string const &schema_name,
	    schema &&value);

    void insert_table_to_filesystem(
	    std::string const &pool_name,
	    std::string const &schema_name,
	    std::string const &table_name,
	    table const &value);

    void insert_table_to_filesystem(
	    std::string const &pool_name,
	    std::string const &schema_name,
	    std::string const &table_name,
	    table &&value);

    static void throw_if_name_exist(const std::string &instance_name);

    user_data obtain_data_in_filesystem(
	    const std::string &pool_name,
	    const std::string &schema_name,
	    const std::string &table_name,
	    const std::string &ud_key);

    std::map<std::string, user_data>  obtain_between_ud_in_filesystem(
	    const std::string &pool_name,
	    const std::string &schema_name,
	    const std::string &table_name,
	    const std::string &lower_bound,
	    const std::string &upper_bound,
	    bool lower_bound_inclusive,
	    bool upper_bound_inclusive);

    static void read_path_to_table(std::string &pool_name, std::string &schema_name, std::string &table_name);

    static void read_path_to_table_and_key(std::string &pool_name, std::string &schema_name, std::string &table_name, std::string &ud_key);

    static void read_user_data(std::string &str_id, std::string &str_name, std::string &str_surname);

    static void throw_if_str_not_bool(const std::string &str);
};

#endif//CW_OS_DATA_BASE_H
