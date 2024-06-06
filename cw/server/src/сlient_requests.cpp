#ifndef DATA_BASE_HTTP_CLIENT_H
#define DATA_BASE_HTTP_CLIENT_H

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <string>
#include <iostream>

class data_base_http_client {
public:
    data_base_http_client(const std::string &host, int port)
	: client_(host, port) {}

    // Вставка данных
    void insert_data(const std::string &pool_name, const std::string &schema_name, const std::string &table_name,
		     const std::string &user_data_key, const std::string &id, const std::string &name,
		     const std::string &surname) {
	nlohmann::json json;
	json["pool_name"] = pool_name;
	json["schema_name"] = schema_name;
	json["table_name"] = table_name;
	json["user_data_key"] = user_data_key;
	json["id"] = id;
	json["name"] = name;
	json["surname"] = surname;

	auto res = client_.Post("/insert_data", json.dump(), "application/json");
	handle_response(res);
    }

    // Получение данных
    void obtain_data(const std::string &pool_name, const std::string &schema_name, const std::string &table_name,
		     const std::string &user_data_key) {
	nlohmann::json json;
	json["pool_name"] = pool_name;
	json["schema_name"] = schema_name;
	json["table_name"] = table_name;
	json["user_data_key"] = user_data_key;

	auto res = client_.Get(("/obtain_data?" + build_query_string(json)));
	handle_response(res);
    }

    void obtain_between_data(const std::string &pool_name, const std::string &schema_name, const std::string &table_name,
			     const std::string &lower_bound, const std::string &upper_bound,
			     const std::string &lower_incl, const std::string &upper_incl) {
	nlohmann::json json;
	json["pool_name"] = pool_name;
	json["schema_name"] = schema_name;
	json["table_name"] = table_name;
	json["lower_bound"] = lower_bound;
	json["upper_bound"] = upper_bound;
	json["lower_incl"] = lower_incl;
	json["upper_incl"] = upper_incl;

	auto res = client_.Get(("/obtain_between_data?" + build_query_string(json)));
	handle_response(res);
    }

    // Обновление данных
    void update_data(const std::string &pool_name, const std::string &schema_name, const std::string &table_name,
		     const std::string &user_data_key, const std::string &id, const std::string &name,
		     const std::string &surname) {
	nlohmann::json json;
	json["pool_name"] = pool_name;
	json["schema_name"] = schema_name;
	json["table_name"] = table_name;
	json["key"] = user_data_key;
	json["id"] = id;
	json["name"] = name;
	json["surname"] = surname;

	auto res = client_.Put("/update_data", json.dump(), "application/json");
	handle_response(res);
    }

    // Удаление данных
    void dispose_data(const std::string &pool_name, const std::string &schema_name, const std::string &table_name,
		      const std::string &user_data_key) {
	nlohmann::json json;
	json["pool_name"] = pool_name;
	json["schema_name"] = schema_name;
	json["table_name"] = table_name;
	json["user_data_key"] = user_data_key;

	auto res = client_.Delete("/dispose_data", json.dump(), "application/json");
	handle_response(res);
    }

    // Вставка таблицы
    void insert_table(const std::string &pool_name, const std::string &schema_name, const std::string &table_name) {
	nlohmann::json json;
	json["pool_name"] = pool_name;
	json["schema_name"] = schema_name;
	json["table_name"] = table_name;

	auto res = client_.Post("/insert_table", json.dump(), "application/json");
	handle_response(res);
    }

    // Вставка схемы
    void insert_schema(const std::string &pool_name, const std::string &schema_name) {
	nlohmann::json json;
	json["pool_name"] = pool_name;
	json["schema_name"] = schema_name;

	auto res = client_.Post("/insert_schema", json.dump(), "application/json");
	handle_response(res);
    }

    // Вставка пула
    void insert_pool(const std::string &pool_name) {
	nlohmann::json json;
	json["pool_name"] = pool_name;

	auto res = client_.Post("/insert_pool", json.dump(), "application/json");
	handle_response(res);
    }

    // Удаление таблицы
    void dispose_table(const std::string &pool_name, const std::string &schema_name, const std::string &table_name) {
	nlohmann::json json;
	json["pool_name"] = pool_name;
	json["schema_name"] = schema_name;
	json["table_name"] = table_name;

	auto res = client_.Delete("/dispose_table", json.dump(), "application/json");
	handle_response(res);
    }

    // Удаление схемы
    void dispose_schema(const std::string &pool_name, const std::string &schema_name) {
	nlohmann::json json;
	json["pool_name"] = pool_name;
	json["schema_name"] = schema_name;

	auto res = client_.Delete("/dispose_schema", json.dump(), "application/json");
	handle_response(res);
    }

    // Удаление пула
    void dispose_pool(const std::string &pool_name) {
	nlohmann::json json;
	json["pool_name"] = pool_name;

	auto res = client_.Delete("/dispose_pool", json.dump(), "application/json");
	handle_response(res);
    }

private:
    httplib::Client client_;

    void handle_response(const httplib::Result &res) {
	if (res) {
	    std::cout << "Status: " << res->status << std::endl;
	    std::cout << "Response: " << res->body << std::endl;
	} else {
	    std::cerr << "HTTP Error: " << res.error() << std::endl;
	}
    }

    // Помощник для построения строки запроса
    std::string build_query_string(const nlohmann::json &json) const {
	std::string query_string;
	for (auto it = json.begin(); it != json.end(); ++it) {
	    if (it != json.begin()) {
		query_string += "&";
	    }
	    query_string += it.key() + "=" + it.value().get<std::string>();
	}
	return query_string;
    }
};




int main() {
    // Настройка клиента для подключения к серверу на localhost:8080
    data_base_http_client client("localhost", 8080);

    // Шаг 1: Вставка пула схем
    client.insert_pool("pool1");

    // Шаг 2: Вставка схемы в пул
    client.insert_schema("pool1", "schema1");

    // Шаг 3: Вставка таблицы в схему
    client.insert_table("pool1", "schema1", "table1");

    // Шаг 4: Вставка данных в таблицу
    client.insert_data("pool1", "schema1", "table1", "key1", "1", "John", "Doe");

    // Шаг 5: Получение данных из таблицы
    client.obtain_data("pool1", "schema1", "table1", "key1");

    // Шаг 6: Обновление данных в таблице
    client.update_data("pool1", "schema1", "table1", "key1", "1", "Jane", "Doe");

    // Шаг 7: Удаление данных из таблицы
    client.dispose_data("pool1", "schema1", "table1", "key1");

    // Шаг 8: Удаление таблицы
    client.dispose_table("pool1", "schema1", "table1");

    // Шаг 9: Удаление схемы
    client.dispose_schema("pool1", "schema1");

    // Шаг 10: Удаление пула схем
    client.dispose_pool("pool1");

    return 0;
}

#endif // DATA_BASE_HTTP_CLIENT_H