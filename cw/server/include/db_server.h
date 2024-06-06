#ifndef DATA_BASE_HTTP_SERVER_H
#define DATA_BASE_HTTP_SERVER_H

#include "../../database/include/data_base.h"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <thread>


class db_server
{

public:
    static std::atomic<bool> stop_signal;

private:
    std::unique_ptr<data_base> _db;

    std::unique_ptr<httplib::Server> _server;

public:
    explicit db_server(std::string const &db_name, bool filesystem_strategy = false)
	: _server(std::make_unique<httplib::Server>())
    {
	if (filesystem_strategy)
	{
	    _db = std::make_unique<data_base>(db_name, storage_interface<std::string, schemas_pool>::storage_strategy::filesystem);
	}
	else
	{
	    _db = std::make_unique<data_base>(db_name, storage_interface<std::string, schemas_pool>::storage_strategy::in_memory);
	}
    }

    void do_work(int port)
    {

	_server->Post("/insert_pool", [&](const httplib::Request &req, httplib::Response &res) {
	    try
	    {
		handle_insert_pool(req, res);
		res.status = 201;// Created
	    }
	    catch (const std::exception &e)
	    {
		res.status = 500;// Internal Server Error
		res.set_content(e.what(), "text/plain");
	    }
	});

	_server->Post("/insert_schema", [&](const httplib::Request &req, httplib::Response &res) {
	    try
	    {
		handle_insert_schema(req, res);
		res.status = 201;// Created
	    }
	    catch (const std::exception &e)
	    {
		res.status = 500;// Internal Server Error
		res.set_content(e.what(), "text/plain");
	    }
	});

	_server->Post("/insert_table", [&](const httplib::Request &req, httplib::Response &res) {
	    try
	    {
		handle_insert_table(req, res);
		res.status = 201;// Created
	    }
	    catch (const std::exception &e)
	    {
		res.status = 500;// Internal Server Error
		res.set_content(e.what(), "text/plain");
	    }
	});

	_server->Post("/insert_data", [&](const httplib::Request &req, httplib::Response &res) {
	    try
	    {
		handle_insert_data(req, res);
		res.status = 201;// Created
	    }
	    catch (const std::exception &e)
	    {
		res.status = 500;// Internal Server Error
		res.set_content(e.what(), "text/plain");
	    }
	});

	_server->Get("/obtain_data", [&](const httplib::Request &req, httplib::Response &res) {
	    try
	    {
		handle_obtain_data(req, res);
		res.status = 200;// OK
	    }
	    catch (const std::exception &e)
	    {
		res.status = 404;// Not Found
		res.set_content(e.what(), "text/plain");
	    }
	});

	_server->Get("/obtain_between_data", [&](const httplib::Request &req, httplib::Response &res) {
	    try
	    {
		handle_obtain_between_data(req, res);
		res.status = 200;// OK
	    }
	    catch (const std::exception &e)
	    {
		res.status = 404;// Not Found
		res.set_content(e.what(), "text/plain");
	    }
	});

	_server->Put("/update_data", [&](const httplib::Request &req, httplib::Response &res) {
	    try
	    {
		handle_update_data(req, res);
		res.status = 200;// OK
	    }
	    catch (const std::exception &e)
	    {
		res.status = 404;// Not Found
		res.set_content(e.what(), "text/plain");
	    }
	});

	_server->Delete("/dispose_data", [&](const httplib::Request &req, httplib::Response &res) {
	    try
	    {
		handle_dispose_data(req, res);
		res.status = 200;// OK
	    }
	    catch (const std::exception &e)
	    {
		res.status = 404;// Not Found
		res.set_content(e.what(), "text/plain");
	    }
	});

	_server->Delete("/dispose_table", [&](const httplib::Request &req, httplib::Response &res) {
	    try
	    {
		handle_dispose_table(req, res);
		res.status = 200;// OK
	    }
	    catch (const std::exception &e)
	    {
		res.status = 404;// Not Found
		res.set_content(e.what(), "text/plain");
	    }
	});

	_server->Delete("/dispose_schema", [&](const httplib::Request &req, httplib::Response &res) {
	    try
	    {
		handle_dispose_schema(req, res);
		res.status = 200;// OK
	    }
	    catch (const std::exception &e)
	    {
		res.status = 404;// Not Found
		res.set_content(e.what(), "text/plain");
	    }
	});

	_server->Delete("/dispose_pool", [&](const httplib::Request &req, httplib::Response &res) {
	    try
	    {
		handle_dispose_pool(req, res);
		res.status = 200;// OK
	    }
	    catch (const std::exception &e)
	    {
		res.status = 404;// Not Found
		res.set_content(e.what(), "text/plain");
	    }
	});

	_server->Post("/save_db_state", [&](const httplib::Request &req, httplib::Response &res) {
	    try
	    {
		handle_save_db_state(req, res);
		res.status = 200;// OK
	    }
	    catch (const std::exception &e)
	    {
		res.status = 404;// Not Found
		res.set_content(e.what(), "text/plain");
	    }
	});

	_server->Get("/load_db_state", [&](const httplib::Request &req, httplib::Response &res) {
	    try
	    {
		handle_load_db_state(req, res);
		res.status = 200;// OK
	    }
	    catch (const std::exception &e)
	    {
		res.status = 404;// Not Found
		res.set_content(e.what(), "text/plain");
	    }
	});

	_server->Get("/alive", [&](const httplib::Request &req, httplib::Response &res) {
	    res.set_content("Server is alive", "text/plain");
	    res.status = 200;// OK
	});

	_server->Get("/heartbeat", [=](const httplib::Request &, httplib::Response &res) {
	    res.status = 200;// OK - alive
	    res.set_content("heartbeat received", "text/plain");
	});

	_server->listen("localhost", port);
    }

public:
    static bool is_server_alive(std::string const &host, int port)
    {
	httplib::Client client(host, port);
	auto res = client.Get("/alive");
	return res && res->status == 200;
    }


public:
    ~db_server()
    {

    }

    db_server(const db_server &other) = delete;

    db_server &operator=(const db_server &other) = delete;

    db_server(db_server &&other) noexcept
	: _db(std::move(other._db)), _server(std::move(other._server))
    {
	other._db = nullptr;
	other._server = nullptr;
    }

    db_server &operator=(db_server &&other) noexcept
    {
	if (this != &other)
	{
	    _db = std::move(other._db);
	    _server = std::move(other._server);
	    other._db = nullptr;
	    other._server = nullptr;
	}
	return *this;
    }


private:
    void handle_load_db_state(const httplib::Request &req, httplib::Response &res)
    {
	_db->load_data_base_state();
	res.set_content("Data inserted successfully", "text/plain");
    }

    void handle_save_db_state(const httplib::Request &req, httplib::Response &res)
    {
	_db->save_data_base_state();
	res.set_content("Data inserted successfully", "text/plain");
    }

    void handle_insert_data(const httplib::Request &req, httplib::Response &res)
    {
	auto json = nlohmann::json::parse(req.body);
	std::string pool_name = json["pool_name"];
	std::string schema_name = json["schema_name"];
	std::string table_name = json["table_name"];
	std::string user_data_key = json["key"];
	user_data data(std::stol(json["id"].get<std::string>()), json["name"], json["surname"]);
	_db->insert_data(pool_name, schema_name, table_name, user_data_key, std::move(data));
	res.set_content("Data inserted successfully", "text/plain");
    }

    void handle_insert_table(const httplib::Request &req, httplib::Response &res)
    {
	auto json = nlohmann::json::parse(req.body);
	std::string pool_name = json["pool_name"];
	std::string schema_name = json["schema_name"];
	std::string table_name = json["table_name"];
	table tbl;
	_db->insert_table(pool_name, schema_name, table_name, std::move(tbl));
	res.set_content("Table add successfully", "text/plain");
    }

    void handle_obtain_data(const httplib::Request &req, httplib::Response &res)
    {
	try
	{
	    // Получаем параметры из URL запроса
	    std::string pool_name = req.get_param_value("pool_name");
	    std::string schema_name = req.get_param_value("schema_name");
	    std::string table_name = req.get_param_value("table_name");
	    std::string user_data_key = req.get_param_value("key");

	    auto const &data = _db->obtain_data(pool_name, schema_name, table_name, user_data_key);

	    nlohmann::json response_json = {
		    {"id", data.get_id()},
		    {"name", data.get_name_value()},
		    {"surname", data.get_surname_value()}};

	    res.set_content(response_json.dump(), "application/json");
	    res.status = 200;
	}
	catch (const std::exception &e)
	{
	    res.status = 404;
	    res.set_content(e.what(), "text/plain");
	}
    }

    void handle_obtain_between_data(const httplib::Request &req, httplib::Response &response)
    {
	try
	{
	    std::string pool_name = req.get_param_value("pool_name");
	    std::string schema_name = req.get_param_value("schema_name");
	    std::string table_name = req.get_param_value("table_name");
	    std::string lb = req.get_param_value("lower_bound");
	    std::string ub = req.get_param_value("upper_bound");
	    std::string lb_incl = req.get_param_value("lower_incl");
	    std::string ub_incl = req.get_param_value("upper_incl");

	    auto lb_bool = !(lb_incl == "0");
	    auto ub_bool = !(lb_incl == "0");
	    auto const &data_vector = _db->obtain_between_data(pool_name, schema_name, table_name, lb, ub, lb_bool, ub_bool);

	    nlohmann::json response_json = nlohmann::json::array();

	    for (const auto &data: data_vector)
	    {
		nlohmann::json data_json = {
			{"key", data.first},
			{"id", data.second.get_id()},
			{"name", data.second.get_name_value()},
			{"surname", data.second.get_surname_value()}};

		response_json.push_back(data_json);
	    }

	    response.set_content(response_json.dump(), "application/json");
	    response.status = 200;
	}
	catch (const std::exception &e)
	{
	    response.status = 404;
	    response.set_content(e.what(), "text/plain");
	}
    }

    void handle_update_data(const httplib::Request &req, httplib::Response &res)
    {
	auto json = nlohmann::json::parse(req.body);
	std::string pool_name = json["pool_name"];
	std::string schema_name = json["schema_name"];
	std::string table_name = json["table_name"];
	std::string user_data_key = json["key"];
	user_data data(std::stoul(json["id"].get<std::string>()), json["name"], json["surname"]);
	_db->update_data(pool_name, schema_name, table_name, user_data_key, std::move(data));
	res.set_content("Data updated successfully", "text/plain");
    }

    void handle_dispose_data(const httplib::Request &req, httplib::Response &res)
    {
	auto json = nlohmann::json::parse(req.body);
	std::string pool_name = json["pool_name"];
	std::string schema_name = json["schema_name"];
	std::string table_name = json["table_name"];
	std::string user_data_key = json["user_data_key"];
	_db->dispose_user_data(pool_name, schema_name, table_name, user_data_key);
	res.set_content("Data removed successfully", "text/plain");
    }

    void handle_dispose_table(const httplib::Request &req, httplib::Response &res)
    {
	auto json = nlohmann::json::parse(req.body);
	std::string pool_name = json["pool_name"];
	std::string schema_name = json["schema_name"];
	std::string table_name = json["table_name"];
	_db->dispose_table(pool_name, schema_name, table_name);
	res.set_content("Table removed successfully", "text/plain");
    }

    void handle_dispose_schema(const httplib::Request &req, httplib::Response &res)
    {
	auto json = nlohmann::json::parse(req.body);
	std::string pool_name = json["pool_name"];
	std::string schema_name = json["schema_name"];
	_db->dispose_schema(pool_name, schema_name);
	res.set_content("Schema removed successfully", "text/plain");
    }

    void handle_dispose_pool(const httplib::Request &req, httplib::Response &res)
    {
	auto json = nlohmann::json::parse(req.body);
	std::string pool_name = json["pool_name"];
	_db->dispose_schemas_pool(pool_name);
	res.set_content("Pool removed successfully", "text/plain");
    }

    void handle_insert_schema(const httplib::Request &req, httplib::Response &res)
    {
	auto json = nlohmann::json::parse(req.body);
	std::string pool_name = json["pool_name"];
	std::string schema_name = json["schema_name"];
	schema new_schema;
	_db->insert_schema(pool_name, schema_name, std::move(new_schema));
	res.set_content("Schema inserted successfully", "text/plain");
    }

    void handle_insert_pool(const httplib::Request &req, httplib::Response &res)
    {
	auto json = nlohmann::json::parse(req.body);
	std::string pool_name = json["pool_name"];
	schemas_pool pool;
	_db->insert_schemas_pool(pool_name, std::move(pool));
	res.set_content("Pool inserted successfully", "text/plain");
    }

public:
    void stop()
    {

    }

    void restart(int port)
    {
	stop();
	do_work(port);
	std::cout << "Server restarted on port " << port << std::endl;
    }

    class monitor_service
    {
    public:
	static void heartbeat(db_server &server, const int port)
	{

	    const int check_interval = 5;
	    const int max_retries = 3;
	    int retries = 0;

	    httplib::Client cli("localhost", port);

	    while (!stop_signal)
	    {
		auto res = cli.Get("/heartbeat");
		if (res && res->status == 200)
		{

		    retries = 0;
		}
		else
		{
		    if (++retries > max_retries)
		    {

			std::cout << "Server is not responding. Attempting to restart..." << std::endl;

			server.stop();
			server.do_work(port);
			retries = 0;
		    }
		}

		std::this_thread::sleep_for(std::chrono::seconds(check_interval));
	    }
	}
    };
};


void make_requests(int port)
{
    httplib::Client cli("localhost", port);

    cli.set_connection_timeout(30, 0);// 30 seconds
    cli.set_read_timeout(30, 0);      // 30 seconds
    cli.set_write_timeout(30, 0);     // 30 seconds

    {
    }

    // Вставка пула схем
    {
	nlohmann::json j;
	j["pool_name"] = "pool1";
	auto res = cli.Post("/insert_pool", j.dump(), "application/json");
	if (res)
	{
	    if (res->status == 201)
	    {
		std::cout << "Inserted pool1"
			  << " status: " << res->status << std::endl;
	    }
	    else
	    {
		std::cout << "Error on /insert_pool, Status Code: " << res->status << ", Message: " << res->body << std::endl;
	    }
	}
	else
	{
	    std::cout << "HTTP Error on /insert_pool: " << res.error() << std::endl;
	}
    }

    // Вставка схемы в пул
    {
	nlohmann::json j;
	j["pool_name"] = "pool1";
	j["schema_name"] = "schema1";
	auto res = cli.Post("/insert_schema", j.dump(), "application/json");
	if (res)
	{
	    if (res->status == 201)
	    {
		std::cout << "Inserted schema1 into pool1"
			  << " status: " << res->status << std::endl;
	    }
	    else
	    {
		std::cout << "Error on /insert_schema, Status Code: " << res->status << ", Message: " << res->body << std::endl;
	    }
	}
	else
	{
	    std::cout << "HTTP Error on /insert_schema: " << res.error() << std::endl;
	}
    }

    // Вставка таблицы в схему
    {
	nlohmann::json j;
	j["pool_name"] = "pool1";
	j["schema_name"] = "schema1";
	j["table_name"] = "table1";
	auto res = cli.Post("/insert_table", j.dump(), "application/json");
	if (res)
	{
	    if (res->status == 201)
	    {
		std::cout << "Inserted table1 into schema1"
			  << " status: " << res->status << std::endl;
	    }
	    else
	    {
		std::cout << "Error on /insert_table, Status Code: " << res->status << ", Message: " << res->body << std::endl;
	    }
	}
	else
	{
	    std::cout << "HTTP Error on /insert_table: " << res.error() << std::endl;
	}
    }

    // Вставка данных в таблицу
    {
	nlohmann::json j;
	j["pool_name"] = "pool1";
	j["schema_name"] = "schema1";
	j["table_name"] = "table1";


	j["name"] = "John";
	j["surname"] = "Doe";
	for (int i = 0; i < 8; ++i)
	{
	    j["id"] = std::to_string(i);
	    j["key"] = "key" + std::to_string(i);
	    auto res = cli.Post("/insert_data", j.dump(), "application/json");
	    if (res)
	    {
		if (res->status == 201)
		{
		    std::cout << "Inserted data into table1"
			      << " status: " << res->status << std::endl;
		}
		else
		{
		    std::cout << "Error on /insert_data, Status Code: " << res->status << ", Message: " << res->body << std::endl;
		}
	    }
	    else
	    {
		std::cout << "HTTP Error on /insert_data: " << res.error() << std::endl;
	    }
	}
    }

    {
	nlohmann::json j;
	j["pool_name"] = "pool1";
	j["schema_name"] = "schema1";
	j["table_name"] = "table1";
	j["user_data_key"] = "key16";
	j["id"] = "777";
	j["name"] = "RIght";
	j["surname"] = "SKXmlsm";
	auto res = cli.Put("/update_data", j.dump(), "application/json");
	if (res)
	{
	    if (res->status == 200)
	    {
		std::cout << "Updated data in table1"
			  << " status: " << res->status << std::endl;
	    }
	    else
	    {
		std::cout << "Error on /update_data, Status Code: " << res->status << ", Message: " << res->body << std::endl;
	    }
	}
	else
	{
	    std::cout << "HTTP Error on /update_data: " << res.error() << std::endl;
	}
    }

    // Получение данных из таблицы
    {
	auto res = cli.Get("/obtain_data?pool_name=pool1&schema_name=schema1&table_name=table1&key=key6");
	if (res)
	{
	    if (res->status == 200)
	    {
		std::cout << "Obtained data: " << res->body << " status: " << res->status << std::endl;
	    }
	    else
	    {
		std::cout << "Error on /obtain_data, Status Code: " << res->status << ", Message: " << res->body << std::endl;
	    }
	}
	else
	{
	    std::cout << "HTTP Error on /obtain_data: " << res.error() << std::endl;
	}
    }
    // Обновление данных в таблице
}


void test_dispose_func(int port)
{
    httplib::Client cli("localhost", 8080);

    // Удаление данных из таблицы
    {
	nlohmann::json j;
	j["pool_name"] = "pool1";
	j["schema_name"] = "schema1";
	j["table_name"] = "table1";
	j["key"] = "key1";
	auto res = cli.Delete("/dispose_data", j.dump(), "application/json");
	if (res && res->status == 200)
	{
	    std::cout << "Disposed data from table1" << std::endl;
	}
	else
	{
	    std::cout << "Error on /dispose_data: " << res.error() << std::endl;
	}
    }

    // Удаление таблицы
    {
	nlohmann::json j;
	j["pool_name"] = "pool1";
	j["schema_name"] = "schema1";
	j["table_name"] = "table1";
	auto res = cli.Delete("/dispose_table", j.dump(), "application/json");
	if (res && res->status == 200)
	{
	    std::cout << "Disposed table1 from schema1" << std::endl;
	}
	else
	{
	    std::cout << "Error on /dispose_table: " << res.error() << std::endl;
	}
    }

    // Удаление схемы
    {
	nlohmann::json j;
	j["pool_name"] = "pool1";
	j["schema_name"] = "schema1";
	auto res = cli.Delete("/dispose_schema", j.dump(), "application/json");
	if (res && res->status == 200)
	{
	    std::cout << "Disposed schema1 from pool1" << std::endl;
	}
	else
	{
	    std::cout << "Error on /dispose_schema: " << res.error() << std::endl;
	}
    }

    // Удаление пула схем
    {
	nlohmann::json j;
	j["pool_name"] = "pool1";
	auto res = cli.Delete("/dispose_pool", j.dump(), "application/json");
	if (res && res->status == 200)
	{
	    std::cout << "Disposed pool1" << std::endl;
	}
	else
	{
	    std::cout << "Error on /dispose_pool: " << res.error() << std::endl;
	}
    }
}

#endif// DATA_BASE_HTTP_SERVER_H