#include "../../../b-tree/b_tree.h"
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
    std::function<int(int const &, int const &)> keys_comparer = CustomCompare();

    search_tree<int, std::string> *tree = new b_tree<int, std::string>(1024, keys_comparer, nullptr, nullptr);

    tree->insert(0, "asdf");
    tree->insert(1, "bbb");

    tree->insert(3, "bbb");
    tree->insert(5, "bbb");
    tree->insert(7, "bbb");
    tree->insert(12, "bbb");

    b_tree<int, std::string>::infix_iterator it = reinterpret_cast<b_tree<int, std::string> *>(tree)->begin_infix();
    auto end_it = reinterpret_cast<b_tree<int, std::string> *>(tree)->end_infix();
    while (it != end_it)
    {
	std::cout << std::get<2>(*it) << " value :" << std::get<3>(*it) << std::endl;
	++it;
    }

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
