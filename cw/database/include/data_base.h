//
// Created by rob22 on 04.05.2024.
//

#ifndef CW_OS_DATA_BASE_H
#define CW_OS_DATA_BASE_H

#include "../../../b-tree/b_tree.h"
#include "../../common/include/storage_interface.h"
#include "../../schemas_pool/include/schemas_pool.h"
#include "iostream"

class data_base : public storage_interface<std::string, schemas_pool>
{
private:

    static std::function<int(const std::string &, const std::string &)> _default_string_comparer;

    std::unique_ptr<b_tree<std::string, schemas_pool>> _data;

    inline static size_t _u_id = 0;

public:

    explicit data_base(std::size_t t,
		       std::string &instance_name,
		       allocator* allocator = nullptr,
		       logger* logger = nullptr,
		       const std::function<int(const std::string&, const std::string&)>& keys_comparer = _default_string_comparer,
		       data_storage_strategy storaged_strategy = data_storage_strategy::in_memory_storaged);

    data_base();

    ~data_base() override;

    data_base(const data_base &other);

    data_base(data_base &&other) noexcept;

    data_base &operator=(const data_base &other);

    data_base &operator=(data_base &&other) noexcept;

public:

    void insert_schemas_pool(
	    std::string const &schemas_pool_name,
	    schemas_pool const &value);

    void insert_schemas_pool(
	    std::string const &schemas_pool_name,
	    schemas_pool const &&value);

    void insert_schema(
	    std::string const &schemas_pool_name,
	    std::string const &schema_name,
	    schema const &value);

    void insert_schema(
	    std::string const &schemas_pool_name,
	    std::string const &schema_name,
	    schema const &&value);

    void insert_table(
	    std::string const &schemas_pool_name,
	    std::string const &schema_name,
	    std::string const &table_name,
	    table const &value);

    void insert_table(
	    std::string const &schemas_pool_name,
	    std::string const &schema_name,
	    std::string const &table_name,
	    table const &&value);

    void insert_data(
	    std::string const &schemas_pool_name,
	    std::string const &schema_name,
	    std::string const &table_name,
	    std::string const &user_data_key,
	    user_data const &&value);

    void insert_data(
	    std::string const &schemas_pool_name,
	    std::string const &schema_name,
	    std::string const &table_name,
	    std::string const &user_data_key,
	    user_data const &value);

    void dispose_schemas_pool(
	    std::string const &schemas_pool_name);

    void dispose_schema(
	    std::string const &schemas_pool_name,
	    std::string const &schema_name);

    void dispose_table(
	    std::string const &schemas_pool_name,
	    std::string const &schema_name,
	    std::string const &table_name);

    void dispose_user_data(
	    std::string const &schemas_pool_name,
	    std::string const &schema_name,
	    std::string const &table_name,
	    std::string const &user_data_key);

private:

    schema get_schema_by_key(
	    std::string const &schemas_pool_name,
	    std::string const &schema_name);

    table get_table_by_key(
	    std::string const &schemas_pool_name,
	    std::string const &schema_name,
	    std::string const &table_name);

private:

    //TODO: interface API

    void insert(const std::string &key, const schemas_pool &value) override;

    void insert(const std::string &key, schemas_pool &&value) override;

    const schemas_pool &obtain(const std::string &key) override;

    std::vector<typename associative_container<std::string, schemas_pool>::key_value_pair> obtain_between(std::string const &lower_bound, std::string const &upper_bound, bool lower_bound_inclusive, bool upper_bound_inclusive) override;

    void dispose(const std::string &key) override;

private:
    //TODO: Call serialize in destructor and deserialize in constructor, and Private access
    void serialize() override;

    void deserialize() override;

private:

    void save_db_to_filesystem();

    void load_db_from_filesystem(const std::string &filename = "");
};

#endif//CW_OS_DATA_BASE_H
