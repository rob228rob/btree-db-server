#include "../../../allocator/allocator_boundary_tags/include/allocator_boundary_tags.h"
#include "../../../b-tree/b_tree.h"
#include "../include/data_base.h"
#include <iostream>
#include <random>

struct CustomCompare {
    int operator()(int const &a, int const &b) const
    {
	if (a < b) return -1;// a меньше b
	else if (a > b)
	    return 1;// a больше b
	else
	    return 0;// a и b равны
    }
};

std::mt19937& getRandomGenerator() {
    static std::mt19937 generator(static_cast<unsigned long>(std::time(nullptr)));
    return generator;
}

std::string generateRandomString(size_t length) {
    const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			      "abcdefghijklmnopqrstuvwxyz"
			      "0123456789";

    std::mt19937& generator = getRandomGenerator();
    std::uniform_int_distribution<size_t> distribution(0, chars.size() - 1);

    std::string randomString;
    randomString.reserve(length);

    for (size_t i = 0; i < length; ++i) {
	randomString += chars[distribution(generator)];
    }

    return randomString;
}

int main()
{
    //    std::function<int(int const &, int const &)> keys_comparer = CustomCompare();
    //
    //    search_tree<int, std::string> *tree = new b_tree<int, std::string>(1024, keys_comparer, nullptr, nullptr);
    //
    //    tree->insert(0, "asdf");
    //    tree->insert(1, "bbb");
    //
    //    tree->insert(3, "bbb");
    //    tree->insert(5, "bbb");
    //    tree->insert(7, "bbb");
    //    tree->insert(12, "bbb");
    //
    //    b_tree<int, std::string>::infix_iterator it = reinterpret_cast<b_tree<int, std::string> *>(tree)->begin_infix();
    //    auto end_it = reinterpret_cast<b_tree<int, std::string> *>(tree)->end_infix();
    //    while (it != end_it)
    //    {
    //	std::cout << std::get<2>(*it) << " value :" << std::get<3>(*it) << std::endl;
    //	++it;
    //    }
    /*
    std::cout << "Hello, World!" << std::endl;

    data_base db;
    try {
	db.insert_schemas_pool("pl", schemas_pool());
	db.insert_schema("pl", "new_schema", schema());
	db.insert_table("pl", "new_schema", "tbl", table());
	for (int i = 3; i < 10'000; ++i)
	{
	    db.insert_data("pl", "new_schema", "tbl", std::string{"key"} + std::to_string(i), user_data(i, "Robsoon", "BBBAt"));

	    //db.dispose_user_data("pl", "new_schema", "tbl", std::string{"key"} + std::to_string(i));
	}
	db.insert_table("pl", "new_schema", "table2", table());
	db.insert_data("pl", "new_schema", "table2", "key123", user_data(1, "nm1", "srn1"));
	db.update_data("pl", "new_schema", "tbl", "key1234", user_data(0, "noootmrteregfhfbds", ""));
	//db.dispose_user_data("new_pool", "new_schema", "key12345", "asdf");
	auto data = db.obtain_data("pl", "new_schema", "tbl", "key1234");
	std::cout << data.get_id() << " " << data.get_name() << " " << data.get_surname() << std::endl;

	auto data2 = db.obtain_data("pl","new_schema", "tbl", "key123");
	std::cout << data2.get_name() << " " << data2.get_surname() << '\n';

    } catch (std::exception const &e)
    {
	std::cout << e.what() << std::endl;
    }


    return 0;
}
*/
    allocator* alc = new allocator_boundary_tags(100'000, nullptr, nullptr, allocator_with_fit_mode::fit_mode::the_worst_fit);

    try
    {
	//data_base db("OLDONWWWW", storage_interface<std::string, schemas_pool>::storage_strategy::filesystem);
	data_base db_1("NewDB", storage_interface<std::string, schemas_pool>::storage_strategy::filesystem);

	auto path = "C:\\Users\\rob22\\CLionProjects\\cw_os\\cw\\database\\src\\test_command.txt";
	//db.execute_command_from_file(path);
	db_1.insert_schemas_pool("main_pool1", schemas_pool());
	db_1.insert_schema("main_pool1", "hr", schema());
	db_1.insert_table("main_pool1", "hr", "schema_employees", table());
	//db_1.load_data_base_state();
	//db_1.start_console_dialog();
//	for (int i = 0; i < 1098; ++i)
//	{
//	    //db_1.insert_data("main_pool1", "hr", "schema_employees", std::to_string(i), user_data(i,"Robs", "Bats"));
//	}

	for (int i = 6666; i < 6667; ++i)
	{
	    //db_1.insert_data("main_pool1", "hr", "schema_employees", "key1", user_data(1, generateRandomString(5), "Hm"));
	}

//	for (int i = 0; i < 1800; ++i)
//	{
//	    db_1.dispose_user_data("main_pool1", "hr", "schema_employees", std::to_string(i));//, user_data(1, "Robs", "Hm"));
//	}
	//db_1.save_data_base_state();

	//db_1.save_data_base_state();
	db_1.start_console_dialog();
	//sch.insert("new_table", tbl);
	//sch_p.insert("new_schema", sch);
	//db.insert_schemas_pool("TEST", schemas_pool());
	//db.insert_schema("TEST", "SCHEEEMA", schema());
	//db.insert_schema("TEST", "FOURTH_SCHEMA", schema());
	//db.dispose_table("TEST", "THIRD_SCHEMA", "Table3");
	//db.update_data("TEST", "THIRD_SCHEMA", "Table2", "IBRITSKII_newHAHA0", user_data(1, "Pac", ":("));
	//auto item = db.obtain_between_data("TEST", "THIRD_SCHEMA", "Table2", "IBRTSKI_newHAHA4333", "IBRTSKI_newHAHA6050", false, false);
//	auto it_beg = item.begin();
//	while (it_beg != item.end())
//	{
//	    std::cout << (*it_beg).first << " " << (*it_beg).second.get_name() << std::endl;
//	    ++it_beg;
//	}
	//db.insert_table("TEST", "THIRD_SCHEMA", "Table3", table());
	//db.dispose_user_data("TEST", "THIRD_SCHEMA", "Table2", "IBRITSKII0");
	//auto item = db.obtain_data("TEST", "THIRD_SCHEMA", "Table2", "IBRITSKII1");
	//std::cout << item.get_surname() << " " << item.get_name() << " " << item.get_id() << std::endl;
	for (int i = 4000; i < 5'000; ++i)
	{
	    //db.dispose_user_data("TEST", "THIRD_SCHEMA", "Table2", "IBRITSKII_newHAHA" + std::to_string(i));//, user_data(1, "WOW", "NOWAAAAY"));

	    //db.insert_data("TEST", "THIRD_SCHEMA", "Table3", "IBRTSKI_newHAHA" + std::to_string(i), user_data(1, "WOW", "NOWAAAY"));
	}
	//db.update_data("TEST", "THIRD_SCHEMA", "Table2", "IBRITSKII_newHAHA" + std::to_string(3), user_data(1, "WOW", "NOWAAAAY"));
    }
    catch (std::exception const &e)
    {
	std::cout << "puc puc: " << e.what() << std::endl;
    }
    delete alc;
}