//
// Created by rob22 on 06.05.2024.
//

#ifndef CW_OS_SCHEMAS_POOL_H
#define CW_OS_SCHEMAS_POOL_H

#include "../../../b-tree/b_tree.h"
#include "../../common/include/storage_interface.h"
#include "../../schema/include/schema.h"

class schemas_pool : public storage_interface<std::string, schema>
{
private:
    static std::function<int(const std::string &, const std::string &)> _default_string_comparer;

    std::unique_ptr<b_tree<std::string, schema>> _data;

    //TODO: implement logic in Constructor
    inline static std::string _absolute_directory_name = "C:\\Users\\rob22\\CLionProjects\\cw_os\\cw\\filesystem\\schemas_pool\\";

public:
    schemas_pool();

    ~schemas_pool() override;

    schemas_pool(const schemas_pool &other);

    schemas_pool(schemas_pool &&other) noexcept;

    schemas_pool &operator=(const schemas_pool &other);

    schemas_pool &operator=(schemas_pool &&other) noexcept;

public:

    void insert(const std::string &key, const schema &value) override;

    void insert(const std::string &key, schema &&value) override;

    const schema &obtain(const std::string &key) override;

    std::vector<typename associative_container<std::string, schema>::key_value_pair> obtain_between(std::string const &lower_bound, std::string const &upper_bound, bool lower_bound_inclusive, bool upper_bound_inclusive) override;

    void dispose(const std::string &key) override;

private:
    //TODO: Call serialize in destructor and deserialize in constructor, and Private access
    void serialize() override;

    void deserialize() override;

public:

    schemas_pool load_schemas_pool_from_filesystem(std::string const &filename);

    void save_schemas_pool_to_filesystem(std::string const &filename = "");

};

#endif//CW_OS_SCHEMAS_POOL_H
