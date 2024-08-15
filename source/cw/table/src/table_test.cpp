//
// Created by rob22 on 04.05.2024.
//
#include "../include/table.h"
#include <memory>

namespace comparison
{

    class int_comparer final
    {

    public:
	int operator()(
		int const &left,
		int const &right) const noexcept
	{
	    return left - right;
	}
    };

    class stdstring_comparer final
    {

    public:
	int operator()(
		std::string const &first,
		std::string const &second) const noexcept
	{
	    if (first == second)
	    {
		return 0;
	    }

	    if (first > second)
	    {
		return 1;
	    }

	    return -1;
	}
    };

    class ac_kvp_int_stdstring_comparer final
    {

    public:
	int operator()(
		typename associative_container<int, std::string>::key_value_pair const &first,
		typename associative_container<int, std::string>::key_value_pair const &second)
	{
	    auto keys_comparison_result = int_comparer()(first.key, second.key);
	    if (keys_comparison_result != 0) return keys_comparison_result;
	    return stdstring_comparer()(first.value, second.value);
	}
    };

}// namespace comparison

#include <random>
#include <string>

std::string generate_random_string(size_t length) {

    const char charset[] =
	    "0123456789"
	    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	    "abcdefghijklmnopqrstuvwxyz";

    std::string random_string;
    random_string.reserve(length);

    std::mt19937 generator{std::random_device{}()};

    std::uniform_int_distribution<std::string::size_type> distribution(0, sizeof(charset) - 2);

    for (size_t i = 0; i < length; ++i) {
	random_string += charset[distribution(generator)];
    }

    return random_string;
}

int main()
{
    std::function<int(std::string const &, std::string const &)> values_comparer = comparison::stdstring_comparer();

    table *tbl = new table(2, nullptr, nullptr, values_comparer);
    std::string s = "1";
    size_t id = 0;
    std::string random_key;
    for (int i = 0; i < 2'500; ++i)
    {
	try
	{
	    user_data ud(id++, std::string{"new"}, std::string{"olf"});
	    user_data us_d;
	    us_d.set_id(static_cast<size_t>(i));
	    us_d.set_name(generate_random_string(10));
	    us_d.set_surname(generate_random_string(10));
	    random_key = generate_random_string(50);
	    tbl->insert(random_key, us_d);
	}
	catch (std::exception const &e)
	{
	    std::cout << e.what() << std::endl;

	}
    }
    std::cout << "OK" << std::endl;
    //tbl->insert(std::string("hhhh"), user_data(1, std::string{"x"}, std::string{"y"}));

    user_data* fnd = new user_data;
    try
    {
	*fnd = tbl->obtain(std::string{"hhhh"});
    }
    catch (std::exception const &e)
    {
	std::cout << e.what() << std::endl;
    }

    //tbl->print_table();

    std::cout << "key was FOUND!!!!!!" << fnd->get_id() << " " << fnd->get_surname() << " " << fnd->get_name() << std::endl;



    delete tbl;


    auto tb = table();

    tb.insert("10", user_data(10, "name", "surname"));
    tb.insert("12", user_data(10, "name", "surname"));
    tb.insert("15", user_data(10, "name", "surname"));
    tb.print_table();
    try {
	tb.update("15", user_data(15, "asdfg", "ssssssssssssssssssssss"));
    }
    catch (std::exception const &e)
    {
	std::cout << "puc puc: " << e.what() << std::endl;
    }

    tb.print_table();

}
