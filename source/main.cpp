#include "allocator/allocator_boundary_tags/include/allocator_boundary_tags.h"
#include "allocator/allocator_global_heap/include/allocator_global_heap.h"
#include "b-tree/b_tree.h"
#include "cw/database/include/data_base.h"
#include <iostream>
#include <random>
#include <test/gtest/gtest.h>

std::mt19937 &getRandomGenerator()
{
    static std::mt19937 generator(static_cast<unsigned long>(std::time(nullptr)));
    return generator;
}

std::string generate_random_string(size_t length)
{
    const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			      "abcdefghijklmnopqrstuvwxyz"
			      "0123456789";

    std::mt19937 &generator = getRandomGenerator();
    std::uniform_int_distribution<size_t> distribution(0, chars.size() - 1);

    std::string randomString;
    randomString.reserve(length);

    for (size_t i = 0; i < length; ++i)
    {
	randomString += chars[distribution(generator)];
    }

    return randomString;
}

std::filesystem::path path;

/*
* default api without allocator
*/
TEST(PositiveTest1, test1)
{
    data_base db_1("NewDB");

    db_1.insert_schemas_pool("pl1", schemas_pool());

    db_1.insert_schema("pl1", "hr", schema());

    db_1.insert_table("pl1", "hr", "try", table());

    auto rand_name = generate_random_string(5);
    auto rand_surname = generate_random_string(5);

    db_1.insert_data("pl1", "hr", "try", "key1", user_data(1, rand_name, rand_surname));
    db_1.insert_data("pl1", "hr", "try", "key2", user_data(2, rand_name, rand_surname));
    db_1.insert_data("pl1", "hr", "try", "key3", user_data(3, rand_name, rand_surname));
    db_1.insert_data("pl1", "hr", "try", "key4", user_data(4, rand_name, rand_surname));

    auto result = db_1.obtain_data("pl1", "hr", "try", "key4");

    EXPECT_EQ(result.get_id(), 4);
    EXPECT_EQ(result.get_name_value(), rand_name);
    EXPECT_EQ(result.get_surname_value(), rand_surname);

    /*
     * key not found
    */
    EXPECT_ANY_THROW(db_1.obtain_data("pl1", "hr", "try", "key100"));

    /*
      * duplicate key
      */
    EXPECT_ANY_THROW(db_1.insert_data("pl1", "hr", "try", "key1", user_data()));
}

/*
* default api with allocator
*/
TEST(PositiveTest2FirstFit, test2)
{
    allocator *alc = new allocator_boundary_tags(1'000'000, nullptr, nullptr, allocator_with_fit_mode::fit_mode::first_fit);

    data_base db_1("NewDB", storage_interface<std::string, schemas_pool>::storage_strategy::in_memory, alc);

    db_1.insert_schemas_pool("pl1", schemas_pool());

    db_1.insert_schema("pl1", "hr", schema());

    db_1.insert_table("pl1", "hr", "try", table());

    auto rand_name = generate_random_string(5);
    auto rand_surname = generate_random_string(5);

    for (int i = 0; i < 500; ++i)
    {
	EXPECT_NO_THROW(db_1.insert_data("pl1", "hr", "try", "key" + std::to_string(i + 1), user_data(i + 1, rand_name, rand_surname)));
    }

    auto result = db_1.obtain_data("pl1", "hr", "try", "key333");

    EXPECT_EQ(result.get_id(), 333);
    EXPECT_EQ(result.get_name_value(), rand_name);
    EXPECT_EQ(result.get_surname_value(), rand_surname);

    /*
     * key not found
    */
    EXPECT_ANY_THROW(db_1.obtain_data("pl1", "hr", "try", "key19760"));

    /*
      * duplicate key
      */
    EXPECT_ANY_THROW(db_1.insert_data("pl1", "hr", "try", "key228", user_data()));

    user_data ud(1984, "upd_name", "upd_surname");

    auto prev_data = db_1.obtain_data("pl1", "hr", "try", "key228");

    EXPECT_NO_THROW(db_1.update_data("pl1", "hr", "try", "key228", ud));

    auto new_data = db_1.obtain_data("pl1", "hr", "try", "key228");

    EXPECT_NE(prev_data.get_id(), new_data.get_id());
    EXPECT_NE(prev_data.get_name_value(), new_data.get_name_value());
    EXPECT_NE(prev_data.get_surname_value(), new_data.get_surname_value());

    /*
     * key not found
     */
    EXPECT_ANY_THROW(db_1.dispose_user_data("pl1", "hr", "try", "key2280000"));

    EXPECT_NO_THROW(db_1.dispose_user_data("pl1", "hr", "try", "key499"));
    EXPECT_ANY_THROW(db_1.dispose_user_data("pl1", "hr", "try", "key499"));

    EXPECT_NO_THROW(db_1.dispose_table("pl1", "hr", "try"));
    EXPECT_ANY_THROW(db_1.dispose_table("pl1", "hr", "try"));

    EXPECT_NO_THROW(db_1.dispose_schema("pl1", "hr"));
    EXPECT_ANY_THROW(db_1.dispose_schema("pl1", "hr"));

    EXPECT_NO_THROW(db_1.dispose_schemas_pool("pl1"));
    EXPECT_ANY_THROW(db_1.dispose_schemas_pool("pl1"));

    delete alc;
}

TEST(PositiveTest2BestFit, test3)
{
    allocator *alc = new allocator_boundary_tags(100'000, nullptr, nullptr, allocator_with_fit_mode::fit_mode::the_best_fit);

    data_base db_1("NewDB", storage_interface<std::string, schemas_pool>::storage_strategy::in_memory, alc);

    db_1.insert_schemas_pool("pl1", schemas_pool());

    db_1.insert_schema("pl1", "hr", schema());

    db_1.insert_table("pl1", "hr", "try", table());

    auto rand_name = generate_random_string(5);
    auto rand_surname = generate_random_string(5);

    for (int i = 0; i < 500; ++i)
    {
	EXPECT_NO_THROW(db_1.insert_data("pl1", "hr", "try", "key" + std::to_string(i + 1), user_data(i + 1, rand_name, rand_surname)));
    }

    auto result = db_1.obtain_data("pl1", "hr", "try", "key333");

    EXPECT_EQ(result.get_id(), 333);
    EXPECT_EQ(result.get_name_value(), rand_name);
    EXPECT_EQ(result.get_surname_value(), rand_surname);

    /*
     * key not found
    */
    EXPECT_ANY_THROW(db_1.obtain_data("pl1", "hr", "try", "key19760"));

    /*
      * duplicate key
      */
    EXPECT_ANY_THROW(db_1.insert_data("pl1", "hr", "try", "key228", user_data()));

    user_data ud(1984, "upd_name", "upd_surname");

    auto prev_data = db_1.obtain_data("pl1", "hr", "try", "key228");

    EXPECT_NO_THROW(db_1.update_data("pl1", "hr", "try", "key228", ud));

    auto new_data = db_1.obtain_data("pl1", "hr", "try", "key228");

    EXPECT_NE(prev_data.get_id(), new_data.get_id());
    EXPECT_NE(prev_data.get_name_value(), new_data.get_name_value());
    EXPECT_NE(prev_data.get_surname_value(), new_data.get_surname_value());

    /*
     * key not found
     */
    EXPECT_ANY_THROW(db_1.dispose_user_data("pl1", "hr", "try", "key2280000"));

    EXPECT_NO_THROW(db_1.dispose_user_data("pl1", "hr", "try", "key499"));
    EXPECT_ANY_THROW(db_1.dispose_user_data("pl1", "hr", "try", "key499"));

    EXPECT_NO_THROW(db_1.dispose_table("pl1", "hr", "try"));
    EXPECT_ANY_THROW(db_1.dispose_table("pl1", "hr", "try"));

    EXPECT_NO_THROW(db_1.dispose_schema("pl1", "hr"));
    EXPECT_ANY_THROW(db_1.dispose_schema("pl1", "hr"));

    EXPECT_NO_THROW(db_1.dispose_schemas_pool("pl1"));
    EXPECT_ANY_THROW(db_1.dispose_schemas_pool("pl1"));

    delete alc;
}

TEST(PositiveTest2WorstFit, test4)
{
    allocator *alc = new allocator_boundary_tags(100'000, nullptr, nullptr, allocator_with_fit_mode::fit_mode::the_worst_fit);

    data_base db_1("NewDB", storage_interface<std::string, schemas_pool>::storage_strategy::in_memory, alc);

    db_1.insert_schemas_pool("pl1", schemas_pool());

    db_1.insert_schema("pl1", "hr", schema());

    db_1.insert_table("pl1", "hr", "try", table());

    auto rand_name = generate_random_string(5);
    auto rand_surname = generate_random_string(5);

    for (int i = 0; i < 500; ++i)
    {
	EXPECT_NO_THROW(db_1.insert_data("pl1", "hr", "try", "key" + std::to_string(i + 1), user_data(i + 1, rand_name, rand_surname)));
    }

    auto result = db_1.obtain_data("pl1", "hr", "try", "key333");

    EXPECT_EQ(result.get_id(), 333);
    EXPECT_EQ(result.get_name_value(), rand_name);
    EXPECT_EQ(result.get_surname_value(), rand_surname);

    /*
     * key not found
    */
    EXPECT_ANY_THROW(db_1.obtain_data("pl1", "hr", "try", "key19760"));

    /*
      * duplicate key
      */
    EXPECT_ANY_THROW(db_1.insert_data("pl1", "hr", "try", "key228", user_data()));

    user_data ud(1984, "upd_name", "upd_surname");

    auto prev_data = db_1.obtain_data("pl1", "hr", "try", "key228");

    EXPECT_NO_THROW(db_1.update_data("pl1", "hr", "try", "key228", ud));

    auto new_data = db_1.obtain_data("pl1", "hr", "try", "key228");

    EXPECT_NE(prev_data.get_id(), new_data.get_id());
    EXPECT_NE(prev_data.get_name_value(), new_data.get_name_value());
    EXPECT_NE(prev_data.get_surname_value(), new_data.get_surname_value());

    /*
     * key not found
     */
    EXPECT_ANY_THROW(db_1.dispose_user_data("pl1", "hr", "try", "key2280000"));

    EXPECT_NO_THROW(db_1.dispose_user_data("pl1", "hr", "try", "key499"));
    EXPECT_ANY_THROW(db_1.dispose_user_data("pl1", "hr", "try", "key499"));

    EXPECT_NO_THROW(db_1.dispose_table("pl1", "hr", "try"));
    EXPECT_ANY_THROW(db_1.dispose_table("pl1", "hr", "try"));

    EXPECT_NO_THROW(db_1.dispose_schema("pl1", "hr"));
    EXPECT_ANY_THROW(db_1.dispose_schema("pl1", "hr"));

    EXPECT_NO_THROW(db_1.dispose_schemas_pool("pl1"));
    EXPECT_ANY_THROW(db_1.dispose_schemas_pool("pl1"));

    delete alc;
}

TEST(FilesystemStorageTest, test5)
{
    std::string db_name = "FilesystemStorage";

    data_base db_1(db_name, storage_interface<std::string, schemas_pool>::storage_strategy::filesystem);

    if (std::filesystem::exists(std::filesystem::absolute(db_name) / "pl1"))
    {
	db_1.dispose_schemas_pool("pl1");
    }

    db_1.insert_schemas_pool("pl1", schemas_pool());

    db_1.insert_schema("pl1", "hr", schema());

    db_1.insert_table("pl1", "hr", "try", table());

    auto rand_name = generate_random_string(5);
    auto rand_surname = generate_random_string(5);

    for (int i = 0; i < 100; ++i)
    {
	EXPECT_NO_THROW(db_1.insert_data("pl1", "hr", "try", "key" + std::to_string(i), user_data(i, rand_name, rand_surname)));
    }

    auto result = db_1.obtain_data("pl1", "hr", "try", "key33");

    EXPECT_EQ(result.get_id(), 33);
    EXPECT_EQ(result.get_name_value(), rand_name);
    EXPECT_EQ(result.get_surname_value(), rand_surname);

    /*
     * key not found
    */
    EXPECT_ANY_THROW(db_1.obtain_data("pl1", "hr", "try", "key19760"));

    /*
      * duplicate key
      */
    EXPECT_ANY_THROW(db_1.insert_data("pl1", "hr", "try", "key28", user_data()));

    auto prev_data = db_1.obtain_data("pl1", "hr", "try", "key22");

    db_1.update_data("pl1", "hr", "try", "key22", user_data(1984, "upd_name", "upd_surname"));

    auto new_data = db_1.obtain_data("pl1", "hr", "try", "key22");

    EXPECT_NE(prev_data.get_id(), new_data.get_id());
    EXPECT_NE(prev_data.get_name_value(), new_data.get_name_value());
    EXPECT_NE(prev_data.get_surname_value(), new_data.get_surname_value());

    for (int i = 0; i < 100; ++i)
    {
	EXPECT_NO_THROW(db_1.dispose_user_data("pl1", "hr", "try", "key" + std::to_string(i)));
    }

    EXPECT_NO_THROW(db_1.insert_data("pl1", "hr", "try", "key28", user_data(28, "Robert", "Robert")));
}

TEST(ObtainBetweenTest, test6)
{
    std::string db_name = "Test6DB";
    data_base db_1(db_name, storage_interface<std::string, schemas_pool>::storage_strategy::filesystem);

    if (std::filesystem::exists(std::filesystem::absolute(db_name) / "pl1"))
    {
	db_1.dispose_schemas_pool("pl1");
    }

    db_1.insert_schemas_pool("pl1", schemas_pool());
    db_1.insert_schema("pl1", "hr", schema());
    db_1.insert_table("pl1", "hr", "try", table());

    auto rand_name = generate_random_string(5);
    auto rand_surname = generate_random_string(5);

    for (int i = 0; i < 100; ++i)
    {
	EXPECT_NO_THROW(db_1.insert_data("pl1", "hr", "try", "key" + std::to_string(i), user_data(i, rand_name, rand_surname)));
    }

    std::map<std::string, user_data> expected_map;
    for (int i = 10; i < 50; ++i)
    {
	expected_map.emplace("key" + std::to_string(i), user_data(i, rand_name, rand_surname));
    }

    auto obtained_map = db_1.obtain_between_data("pl1", "hr", "try", "key10", "key50", true, false);

    for (int i = 10; i < 50; ++i)
    {
	auto expected_res = expected_map.find("key" + std::to_string(i));
	auto current_res = obtained_map.find("key" + std::to_string(i));

	EXPECT_EQ((*expected_res).first, (*current_res).first);
	EXPECT_EQ((*expected_res).second.get_id(), (*current_res).second.get_id());
	EXPECT_EQ((*expected_res).second.get_name_value(), (*current_res).second.get_name_value());
	EXPECT_EQ((*expected_res).second.get_surname_value(), (*current_res).second.get_surname_value());
    }

    std::map<std::string, user_data> expected_map2;
    for (int i = 98; i < 100; ++i)
    {
	expected_map2.emplace("key" + std::to_string(i), user_data(i, rand_name, rand_surname));
    }

    auto obtained_map2 = db_1.obtain_between_data("pl1", "hr", "try", "key98", "key99", true, true);

    for (int i = 98; i < 100; ++i)
    {
	auto expected_res = expected_map2.find("key" + std::to_string(i));
	auto current_res = obtained_map2.find("key" + std::to_string(i));

	if (expected_res == expected_map2.end() && current_res == obtained_map2.end())
	{
	    continue;
	}

	EXPECT_EQ((*expected_res).first, (*current_res).first);
	EXPECT_EQ((*expected_res).second.get_id(), (*current_res).second.get_id());
	EXPECT_EQ((*expected_res).second.get_name_value(), (*current_res).second.get_name_value());
	EXPECT_EQ((*expected_res).second.get_surname_value(), (*current_res).second.get_surname_value());
    }
}

TEST(PositiveTestSavingState, test7)
{
    data_base db_1("Test7DB", storage_interface<std::string, schemas_pool>::storage_strategy::in_memory);

    db_1.insert_schemas_pool("pl1", schemas_pool());
    db_1.insert_schema("pl1", "hr", schema());
    db_1.insert_table("pl1", "hr", "try", table());

    auto rand_name = generate_random_string(5);
    auto rand_surname = generate_random_string(5);

    for (int i = 0; i < 50; ++i)
    {
	EXPECT_NO_THROW(db_1.insert_data("pl1", "hr", "try", "key" + std::to_string(i + 1), user_data(i + 1, rand_name, rand_surname)));
    }

    EXPECT_NO_THROW(db_1.save_data_base_state());
}

TEST(PositiveTestLoadingState, test8)
{
    data_base db_1("Test7DB", storage_interface<std::string, schemas_pool>::storage_strategy::in_memory);

    EXPECT_NO_THROW(db_1.load_data_base_state());

    EXPECT_NO_THROW(db_1.obtain_data("pl1", "hr", "try", "key1"));
    EXPECT_NO_THROW(db_1.obtain_data("pl1", "hr", "try", "key50"));
}

TEST(ExecutingFileCommand, test9)
{
    data_base db_1("Test9DB", storage_interface<std::string, schemas_pool>::storage_strategy::in_memory);

    EXPECT_NO_THROW(db_1.execute_command_from_file(path.string()));

    /*
     * insert_data:main_pool:hr:schema_employees:1:1,John,Doe;
     * update_data:main_pool:hr:schema_employees:1:1,Johnny,Doe
    */
    auto obtained_data = db_1.obtain_data("main_pool", "hr", "schema_employees", "1");

    EXPECT_EQ(obtained_data.get_id(), 1);
    EXPECT_EQ(obtained_data.get_name_value(), "Johnny");
    EXPECT_EQ(obtained_data.get_surname_value(), "Doe");

    /*
     * insert_data:main_pool:finance:schema_salaries:4:4,Bob,Brown
     */
    auto obtained_data2 = db_1.obtain_data("main_pool", "finance", "schema_salaries", "4");

    EXPECT_EQ(obtained_data2.get_id(), 4);
    EXPECT_EQ(obtained_data2.get_name_value(), "Bob");
    EXPECT_EQ(obtained_data2.get_surname_value(), "Brown");

    /*
     * insert_data:main_pool:it:schema_assets:8:8,Faye,Valentine
     * update_data:main_pool:it:schema_assets:8:8,Faith,Valentino
     */

    auto obtained_data3 = db_1.obtain_data("main_pool", "it", "schema_assets", "8");

    EXPECT_EQ(obtained_data3.get_id(), 8);
    EXPECT_EQ(obtained_data3.get_name_value(), "Faith");
    EXPECT_EQ(obtained_data3.get_surname_value(), "Valentino");

    /*
     * insert_data:main_pool:operations:schema_logs:11:11,Ivan,Rodriguez
     */
    auto obtained_data4 = db_1.obtain_data("main_pool", "operations", "schema_logs", "11");

    EXPECT_EQ(obtained_data4.get_id(), 11);
    EXPECT_EQ(obtained_data4.get_name_value(), "Ivan");
    EXPECT_EQ(obtained_data4.get_surname_value(), "Rodriguez");

    /*
     * update_data:main_pool:finance:schema_salaries:14:14,Lewis,Cypher
     */
    auto obtained_data5 = db_1.obtain_data("main_pool", "finance", "schema_salaries", "14");

    EXPECT_EQ(obtained_data5.get_id(), 14);
    EXPECT_EQ(obtained_data5.get_name_value(), "Lewis");
    EXPECT_EQ(obtained_data5.get_surname_value(), "Cypher");
}

TEST(AnyInsertionPoolAndSchemas, test10)
{
    std::string db_name = "Test10DB";
    data_base db_1(db_name, storage_interface<std::string, schemas_pool>::storage_strategy::filesystem);

    EXPECT_NO_THROW( db_1.insert_schemas_pool("pl1", schemas_pool()));
    EXPECT_NO_THROW(db_1.insert_schema("pl1", "hr", schema()));
    EXPECT_NO_THROW(db_1.insert_table("pl1", "hr", "try", table()));
    EXPECT_NO_THROW(db_1.insert_data("pl1", "hr", "try", "new_key", user_data(1,"new_name", "new_surname")));

    /*
     *	pool does not exist
     */
    EXPECT_ANY_THROW(db_1.insert_table("pl", "hr", "try", table()));
    EXPECT_ANY_THROW(db_1.insert_schema("pl", "hr", schema()));
    EXPECT_ANY_THROW( db_1.insert_schema("pl", "it", schema()));

    /*
     * disposing instances
     */
    EXPECT_NO_THROW(db_1.dispose_user_data("pl1", "hr", "try", "new_key"));
    EXPECT_NO_THROW(db_1.dispose_table("pl1", "hr", "try"));
    EXPECT_NO_THROW(db_1.dispose_schema("pl1", "hr"));
    EXPECT_NO_THROW( db_1.dispose_schemas_pool("pl1"));

    /*
     * double disposing
     */
    EXPECT_ANY_THROW(db_1.dispose_user_data("pl1", "hr", "try", "new_key"));
    EXPECT_ANY_THROW(db_1.dispose_table("pl1", "hr", "try"));
    EXPECT_ANY_THROW(db_1.dispose_schema("pl1", "hr"));
    EXPECT_ANY_THROW( db_1.dispose_schemas_pool("pl1"));
}

TEST(WorkWithUnexistedData ,test11)
{
    std::string db_name = "Test10DB";
    data_base db_1(db_name, storage_interface<std::string, schemas_pool>::storage_strategy::filesystem);

    if (std::filesystem::exists(std::filesystem::absolute(db_name) / "pool1"))
    {
	db_1.dispose_schemas_pool("pool1");
    }

    db_1.insert_schemas_pool("pool1", schemas_pool());
    db_1.insert_schema("pool1", "schema", schema());
    db_1.insert_table("pool1", "schema", "table", table());

    for (int i = 0; i < 20; ++i)
    {
	EXPECT_NO_THROW(db_1.insert_data("pool1", "schema", "table", "key" + std::to_string(i), user_data(i, "name", "surname")));
    }

    for (int i = 0; i < 20; ++i)
    {
	EXPECT_ANY_THROW(db_1.dispose_user_data("pool", "schema", "table", "key" + std::to_string(i)));
    }

    for (int i = 0; i < 20; ++i)
    {
	EXPECT_ANY_THROW(db_1.dispose_user_data("pool1", "schema1", "table", "key" + std::to_string(i)));
    }

    for (int i = 0; i < 20; ++i)
    {
	EXPECT_ANY_THROW(db_1.dispose_user_data("pool1", "schema", "table1", "key" + std::to_string(i)));
    }

    for (int i = 0; i < 20; ++i)
    {
	EXPECT_ANY_THROW(db_1.dispose_user_data("pool1", "schema", "table", "1key" + std::to_string(i)));
    }
}

TEST(StringPoolTests, test12)
{
    user_data ud(1,"name", "surname");

    EXPECT_EQ(ud.get_id(), 1);
    EXPECT_EQ(ud.get_name_value(), std::string{"name"});
    EXPECT_EQ(ud.get_surname_value(), std::string{"surname"});

    EXPECT_NE(ud.get_name(), ud.get_name_value());
    EXPECT_NE(ud.get_surname(), ud.get_surname_value());

    EXPECT_FALSE(ud.get_name_value() == std::to_string(ud.get_name_ind()));
    EXPECT_FALSE(ud.get_surname_value() == std::to_string(ud.get_surname_ind()));
}

int main(int argc, char **argv)
{
    if (argc > 1)
    {
	path = std::filesystem::absolute(argv[1]);
    }

    data_base db_1("MyDB");
    db_1.insert_schemas_pool("pl1", schemas_pool());
    db_1.insert_schema("pl1", "hr", schema());
    db_1.insert_table("pl1", "hr", "try", table());
    db_1.start_console_dialog();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}