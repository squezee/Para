#include <iostream>
#include <string>
#include <mysql/mysql.h>
#include <limits>

// параметры подключения к базе данных
const char* HOST = "localhost";
const char* USER = "root";
const char* PASSWORD = "password"; // замените на ваш пароль
const char* DATABASE = "testdb";

MYSQL* conn;
std::string currentUserRole;

void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void executeSQL(const std::string& sql) {
    if (mysql_query(conn, sql.c_str())) {
        std::cerr << "ошибка MySQL: " << mysql_error(conn) << std::endl;
    }
}

void createTrip() {
    if (currentUserRole != "admin") {
        std::cout << "доступ запрещен." << std::endl;
        return;
    }

    std::string number, busType, destination, departureTime, arrivalTime;
    std::cout << "введите номер рейса: ";
    clearInputBuffer();
    std::getline(std::cin, number);
    std::cout << "введите тип автобуса: ";
    std::getline(std::cin, busType);
    std::cout << "введите пункт назначения: ";
    std::getline(std::cin, destination);
    std::cout << "введите время отправления (YYYY-MM-DD HH:MM:SS): ";
    std::getline(std::cin, departureTime);
    std::cout << "введите время прибытия (YYYY-MM-DD HH:MM:SS): ";
    std::getline(std::cin, arrivalTime);

    std::string sql = "INSERT INTO trips (number, bus_type, destination, departure_time, arrival_time) VALUES ('" + 
                     number + "', '" + busType + "', '" + destination + "', '" + departureTime + "', '" + arrivalTime + "');";
    executeSQL(sql);

    std::cout << "рейс добавлен." << std::endl;
}

void registerUser() {
    std::string login, password, role;
    std::cout << "введите логин: ";
    clearInputBuffer();
    std::getline(std::cin, login);
    std::cout << "введите пароль: ";
    std::getline(std::cin, password);
    std::cout << "введите роль (admin/user): ";
    std::getline(std::cin, role);

    std::string sql = "INSERT INTO users (login, password, role) VALUES ('" + login + "', '" + password + "', '" + role + "');";
    executeSQL(sql);

    std::cout << "юзер успешно зарегистрирован." << std::endl;
}

void readTrips() {
    std::string query = "SELECT * FROM trips;";
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "ошибка MySQL: " << mysql_error(conn) << std::endl;
        return;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;

    std::cout << "список рейсов:" << std::endl;
    while ((row = mysql_fetch_row(res))) {
        std::cout << "номер рейса: " << row[1] << ", тип автобуса: " << row[2]
                  << ", пункт назначения: " << row[3] << ", отправление: " << row[4]
                  << ", прибытие: " << row[5] << std::endl;
    }
    mysql_free_result(res);
}

void findTripsByCriteria() {
    std::string destination, time;
    std::cout << "введите пункт назначения: ";
    clearInputBuffer();
    std::getline(std::cin, destination);
    std::cout << "введите время отправления или прибытия (YYYY-MM-DD HH:MM:SS): ";
    std::getline(std::cin, time);

    std::string query = "SELECT * FROM trips WHERE destination = '" + destination + "' AND "
                        "(departure_time = '" + time + "' OR arrival_time = '" + time + "');";
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "ошибка MySQL: " << mysql_error(conn) << std::endl;
        return;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;

    std::cout << "найденные рейсы:" << std::endl;
    while ((row = mysql_fetch_row(res))) {
        std::cout << "номер рейса: " << row[1] << ", тип автобуса: " << row[2]
                  << ", пункт назначения: " << row[3] << ", отправление: " << row[4]
                  << ", прибытие: " << row[5] << std::endl;
    }
    mysql_free_result(res);
}

bool loginUser() {
    std::string login, password;
    std::cout << "логин: ";
    clearInputBuffer();
    std::getline(std::cin, login);
    std::cout << "пароль: ";
    std::getline(std::cin, password);

    std::string query = "SELECT role FROM users WHERE login = '" + login + "' AND password = '" + password + "' LIMIT 1;";
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "ошибка MySQL: " << mysql_error(conn) << std::endl;
        return false;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row) {
        currentUserRole = row[0];
        std::cout << "успешный вход! ваша роль: " << currentUserRole << std::endl;
        mysql_free_result(res);
        return true;
    }

    std::cout << "неверные логин или пароль." << std::endl;
    mysql_free_result(res);
    return false;
}

void menu() {
    int choice;
    do {
        std::cout << "\nменю:\n";
        if (currentUserRole == "admin") {
            std::cout << "1. добавить рейс\n";
            std::cout << "2. регистрация юзера\n";
        }
        std::cout << "3. просмотр всех рейсов\n";
        std::cout << "4. найти рейсы по критериям\n";
        std::cout << "0. выход\n";
        std::cout << "выберите действие: ";
        std::cin >> choice;
        clearInputBuffer();

        switch (choice) {
            case 1: if (currentUserRole == "admin") createTrip(); break;
            case 2: if (currentUserRole == "admin") registerUser(); break;
            case 3: readTrips(); break;
            case 4: findTripsByCriteria(); break;
            case 0: std::cout << "выход из программы.\n"; break;
            default: std::cout << "неверный выбор.\n";
        }
    } while (choice != 0);
}

int main() {
    conn = mysql_init(nullptr);
    if (!mysql_real_connect(conn, HOST, USER, PASSWORD, DATABASE, 0, nullptr, 0)) {
        std::cerr << "ошибка подключения: " << mysql_error(conn) << std::endl;
        return 1;
    }

    if (loginUser()) menu();

    mysql_close(conn);
    return 0;
}
