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

class data_base : public storage_interface<std::string, schemas_pool>
{
public:

    static std::function<int(const std::string &, const std::string &)> _default_string_comparer;

    static inline std::string _absolute_directory_name = "C:\\Users\\rob22\\CLionProjects\\cw_os\\cw\\filesystem\\db_instances";

private:

    friend class table;

    friend class schema;

    friend class schema_pool;

private:

    static std::set<std::string> _instance_names;

    std::unique_ptr<b_tree<std::string, schemas_pool>> _data;

public:

    explicit data_base(std::size_t t,
		       std::string &instance_name,
		       allocator* allocator = nullptr,
		       logger* logger = nullptr,
		       const std::function<int(const std::string&, const std::string&)>& keys_comparer = _default_string_comparer,
		       storage_strategy storaged_strategy = storage_strategy::in_memory);

    data_base();

    ~data_base() override;

    data_base(const data_base &other) = delete;

    data_base(data_base &&other) noexcept;

    data_base &operator=(const data_base &other) = delete;

    data_base &operator=(data_base &&other) noexcept;

private:

    static void throw_if_invalid(std::string const &key);

public:

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

    const user_data& obtain_data(
	    std::string const &pool_name,
	    std::string const &schema_name,
	    std::string const &table_name,
	    std::string const &user_data_key);

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

    schema &get_schema_by_key(
	    std::string const &pool_name,
	    std::string const &schema_name);

    table &get_table_by_key(
	    std::string const &pool_name,
	    std::string const &schema_name,
	    std::string const &table_name);

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

    void save_db_to_filesystem();

    void load_db_from_filesystem(const std::string &filename = "");

    std::string find_key_in_file(const std::string &key);

    void insert_pool_to_filesystem(const std::string &pool_name, schemas_pool &&value);

    void insert_pool_to_filesystem(const std::string &pool_name, const schemas_pool &value);

};

#endif//CW_OS_DATA_BASE_H
