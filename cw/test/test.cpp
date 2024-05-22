//
// Created by rob22 on 13.05.2024.
//


#include "../table/include/table.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>

user_data obtain_in_filesystem_test(const std::string &key)
{
    auto filename = "C:\\Users\\rob22\\CLionProjects\\cw_os\\cw\\filesystem\\tables\\HADNAKLD.txt";
    auto index_filename = "C:\\Users\\rob22\\CLionProjects\\cw_os\\cw\\filesystem\\tables\\index_HADNAKLD.txt";

    std::ifstream index_file(index_filename);
    if (!index_file.is_open())
    {
	throw std::runtime_error("file did not open");
    }

    std::vector<std::streamoff> index_array;
    std::string index_str;
    while (std::getline(index_file, index_str, '#'))
    {
	index_array.push_back(std::stol(index_str));
    }
    index_file.close();

    std::ifstream data_file(filename);
    if (!data_file.is_open())
    {
	throw std::runtime_error("file did not open");
    }

    size_t left = 0;
    size_t right = index_array.size() - 1;

    while (left <= right)
    {
	size_t mid = left + (right - left) / 2;

	data_file.seekg(index_array[mid]);

	std::string file_key;
	std::getline(data_file, file_key, '#');
	std::cout << file_key << std::endl;
	if (key == file_key)
	{
	    std::string user_info;
	    std::getline(data_file, user_info, '|');
	    std::istringstream iss(user_info);
	    std::string id_str, name, surname;

	    std::getline(iss, id_str, '#');
	    std::getline(iss, name, '#');
	    std::getline(iss, surname, '#');


	    size_t id = std::stoul(id_str);

	    data_file.close();

	    return user_data(id, name, surname);
	}

	if (file_key < key)
	{
	    left = mid + 1;
	}
	else
	{
	    right = mid - 1;
	}
    }


    data_file.close();
    throw std::logic_error("key not found");
}

int main()
{
    std::string filename = std::string{"C:\\Users\\rob22\\CLionProjects\\cw_os\\cw\\filesystem\\tables\\"} + "new_table" + ".txt";
    std::ofstream file(filename);
    file << "sdfdfgbh tegregf rewfbg rfgb rvbrfv\n";
    file.close();

    table tbl = table(4, nullptr, nullptr, table::_default_string_comparer, storage_interface<std::string, user_data>::storage_strategy::filesystem, "mb_LASTONE");


    try
    {
	std::string key = "keyZzzzzzzzz";
	//auto ud = tbl.obtain_in_filesystem("keyZ8064");
	//std::cout << "DATA:" << ud.get_surname() << " " << ud.get_name() << " " << ud.get_id() << std::endl;

	//tbl.insert_to_filesystem(key, user_data(1, "INSERT", "INSERT"));

    }
    catch (std::exception const &e)
    {
	std::cout << e.what() << '\n';
    }
    catch (...)
    {
	std::cout << "puc puc " << std::endl;
    }

//    try
//    {
//	user_data ud = tbl.obtain_in_filesystem("key1235");
//	//	user_data ud = obtain_in_filesystem_test("key11764");
//	std::cout << ud.get_id() << " " << ud.get_name() << " " << ud.get_surname() << std::endl;
//	//tbl.obtain_in_filesystem("z");
//	//tbl.insert_to_filesystem("z", user_data(123, "INSERT", "INSERT"));
//    }
//    catch (std::exception const &e)
//    {
//	std::cout << e.what() << std::endl;
//    }
    std::ios_base::sync_with_stdio(false);

    auto start = std::chrono::high_resolution_clock::now();
    try
    {
	for (int i = 159'350; i < 160'001; ++i)
	{
	    tbl.insert("GOOOL" + std::to_string(i), user_data(i, "NOHAHA", "DEDINCIIKSMK"));
	    //tbl.update_in_filesystem("wwWOWOWO" + std::to_string(i), user_data(0, "Gamnouuuuu", "AiSauuuuu"));
	    //tbl.obtain_in_filesystem("ke" + std::to_string(i));//, user_data(0, "ILYA", "IBRITSKI"));
	    //user_data ud(i, "AIKAMEN", "STEKLOBETONKA");
	    //tbl.update_in_filesystem("wwWOWOWO" + std::to_string(i), ud);
	    //tbl.dispose_from_filesystem("wwWOWOWO" + std::to_string(i));//, user_data(0, "ILYA", "IBRITSKI"));
	    //tbl.dispose_from_filesystem("zov" + std::to_string(i));
	    //tbl.insert("key" + std::to_string(i), user_data(0, "Robs", "Bat"));
	}
//	auto mp = tbl.obtain_between_in_filesystem("wwWOWOWO7516", "wwWOWOWO7515", true, true);
//
//	auto it = mp.begin();
//	while (it != mp.end())
//	{
//	    std::cout << (*it).first << std::endl;
//	    ++it;
//	}

	//tbl.dispose_from_filesystem("zzzzz6998");

    } catch (std::exception const &e)
    {
	std::cout << "puc srenjk;   " << e.what() << std::endl;
    }
    try
    {
	//tbl.update_in_filesystem("wwWOWOWO6888",user_data(0, "NO_ILYA_HA", "NOE_IBRITSKiY"));
    }
    catch (std::exception const &e)
    {
	std::cout << e.what() << std::endl;
    }


    //tbl.insert("key24", user_data(0, "Robs", "Bat"));
    //tbl.insert("aa", user_data(1, "NAMEE", "SRNNNSNRSRNSNRSNR"));
    //tbl.insert("zz", user_data(1, "NAMEE", "SRNNNSNRSRNSNRSNR"));
    //tbl.insert("xxxx", user_data(1, "NAMEE", "SRNNNSNRSRNSNRSNR"));


    // Здесь выполняется ваш код, время которого вы хотите измерить
    // ...

    // Записываем время остановки
    auto finish = std::chrono::high_resolution_clock::now();

    // Рассчитываем продолжительность выполнения
    std::chrono::duration<double> elapsed = finish - start;

    std::cout << "Duration: " << elapsed.count() << " sec" << std::endl;

    return 0;
}