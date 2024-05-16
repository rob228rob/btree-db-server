#include "../../../b-tree/b_tree.h"
#include "../include/data_base.h"
#include <iostream>

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

    std::cout << "Hello, World!" << std::endl;

    data_base db;
    try {
	db.insert_schemas_pool("pl", schemas_pool());
	db.insert_schema("pl", "new_schema", schema());
	db.insert_table("pl", "new_schema", "tbl", table());
	for (int i = 3; i < 1'000'000; ++i)
	{
	    db.insert_data("pl", "new_schema", "tbl", std::string{"key"} + std::to_string(i), user_data(i, "Robsoon", "BBBAt"));

	    db.dispose_user_data("pl", "new_schema", "tbl", std::string{"key"} + std::to_string(i));
	}
	db.insert_table("pl", "new_schema", "table2", table());
	db.insert_data("pl", "new_schema", "table2", "key123", user_data(1, "nm1", "srn1"));
	db.update_data("pl", "new_schema", "tbl", "key1234", user_data(0, "noootmrteregfhfbds", ""));
	//db.dispose_user_data("new_pool", "new_schema", "key12345", "asdf");
	auto data = db.obtain_data("pl", "new_schema", "tbl", "key1234");
	std::cout << data.get_id() << " " << data.get_name() << " " << data.get_surname() << std::endl;

	auto data2 = db.obtain_data("pl","new_schema", "table2", "key123");
	std::cout << data2.get_name() << " " << data2.get_surname() << '\n';

    } catch (std::exception const &e)
    {
	std::cout << e.what() << std::endl;
    }


    return 0;
}
