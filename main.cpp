#include <iostream>
#include <string>
#include <mysql/mysql.h>

// Параметры подключения к базе данных
const char* HOST = "localhost";
const char* USER = "root";
const char* PASSWORD = "7363"; 
const char* DATABASE = "testdb";

MYSQL* conn;
std::string currentUserRole;

void executeSQL(const std::string& sql) {
    if (mysql_query(conn, sql.c_str())) {
        std::cerr << "Ошибка MySQL: " << mysql_error(conn) << std::endl;
    }
}

void createTrip() {
    if (currentUserRole != "admin") {
        std::cout << "Доступ запрещен." << std::endl;
        return;
    }

    std::string number, busType, destination, departureTime, arrivalTime;
    std::cout << "Введите номер рейса: ";
    std::cin >> number;
    std::cout << "Введите тип автобуса: ";
    std::cin >> busType;
    std::cout << "Введите пункт назначения: ";
    std::cin >> destination;
    std::cout << "Введите время отправления (YYYY-MM-DD HH:MM:SS): ";
    std::cin >> departureTime;
    std::cout << "Введите время прибытия (YYYY-MM-DD HH:MM:SS): ";
    std::cin >> arrivalTime;

    std::string sql = "INSERT INTO trips (number, bus_type, destination, departure_time, arrival_time) VALUES ('" + 
                     number + "', '" + busType + "', '" + destination + "', '" + departureTime + "', '" + arrivalTime + "');";
    executeSQL(sql);

    std::cout << "Рейс добавлен." << std::endl;
}

void readTrips() {
    std::string sql = "SELECT * FROM trips;";
    if (mysql_query(conn, sql.c_str())) {
        std::cerr << "Ошибка MySQL: " << mysql_error(conn) << std::endl;
        return;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        std::cerr << "Ошибка получения результата: " << mysql_error(conn) << std::endl;
        return;
    }

    MYSQL_ROW row;
    std::cout << "Список рейсов:" << std::endl;
    while ((row = mysql_fetch_row(result))) {
        std::cout << "Номер: " << row[0] << " Тип: " << row[1] << " Назначение: " << row[2] 
                  << " Отправление: " << row[3] << " Прибытие: " << row[4] << std::endl;
    }

    mysql_free_result(result);
}

void findTripsByCriteria() {
    std::string destination, time;
    std::cout << "Введите пункт назначения: ";
    std::cin >> destination;
    std::cout << "Введите время (YYYY-MM-DD HH:MM:SS): ";
    std::cin >> time;

    std::string sql = "SELECT * FROM trips WHERE destination = '" + destination + "' AND (departure_time >= '" + time + "' OR arrival_time <= '" + time + "');";
    if (mysql_query(conn, sql.c_str())) {
        std::cerr << "Ошибка MySQL: " << mysql_error(conn) << std::endl;
        return;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        std::cerr << "Ошибка получения результата: " << mysql_error(conn) << std::endl;
        return;
    }

    MYSQL_ROW row;
    std::cout << "Найденные рейсы:" << std::endl;
    while ((row = mysql_fetch_row(result))) {
        std::cout << "Номер: " << row[0] << " Тип: " << row[1] << " Назначение: " << row[2] 
                  << " Отправление: " << row[3] << " Прибытие: " << row[4] << std::endl;
    }

    mysql_free_result(result);
}

void loginUser() {
    std::string login, password;
    std::cout << "Введите логин: ";
    std::cin >> login;
    std::cout << "Введите пароль: ";
    std::cin >> password;

    std::string sql = "SELECT role FROM users WHERE login = '" + login + "' AND password = '" + password + "';";
    if (mysql_query(conn, sql.c_str())) {
        std::cerr << "Ошибка MySQL: " << mysql_error(conn) << std::endl;
        return;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        std::cerr << "Ошибка получения результата: " << mysql_error(conn) << std::endl;
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (row) {
        currentUserRole = row[0];
        std::cout << "Успешный вход. Ваша роль: " << currentUserRole << std::endl;
    } else {
        std::cout << "Неверный логин или пароль." << std::endl;
        currentUserRole = "";
    }

    mysql_free_result(result);
}

void menu() {
    int choice;
    do {
        std::cout << "\nМеню:\n";
        std::cout << "1. Добавить рейс\n";
        std::cout << "2. Просмотреть все рейсы\n";
        std::cout << "3. Найти рейсы по критериям\n";
        std::cout << "0. Выход\n";
        std::cout << "Выберите действие: ";
        std::cin >> choice;

        switch (choice) {
            case 1:
                createTrip();
                break;
            case 2:
                readTrips();
                break;
            case 3:
                findTripsByCriteria();
                break;
            case 0:
                std::cout << "Выход из программы." << std::endl;
                break;
            default:
                std::cout << "Неверный выбор. Попробуйте снова." << std::endl;
        }
    } while (choice != 0);
}

int main() {
    conn = mysql_init(nullptr);
    if (!conn) {
        std::cerr << "Не удалось инициализировать MySQL." << std::endl;
        return 1;
    }

    if (!mysql_real_connect(conn, HOST, USER, PASSWORD, DATABASE, 0, nullptr, 0)) {
        std::cerr << "Ошибка подключения к базе данных: " << mysql_error(conn) << std::endl;
        return 1;
    }

    std::string createTripsTableSQL = "CREATE TABLE IF NOT EXISTS trips (id INT AUTO_INCREMENT PRIMARY KEY, number VARCHAR(10), bus_type VARCHAR(50), destination VARCHAR(100), departure_time DATETIME, arrival_time DATETIME);";
    executeSQL(createTripsTableSQL);

    std::string createUsersTableSQL = "CREATE TABLE IF NOT EXISTS users (id INT AUTO_INCREMENT PRIMARY KEY, login VARCHAR(50), password VARCHAR(50), role VARCHAR(10));";
    executeSQL(createUsersTableSQL);

    // Пример добавления администратора (запускается один раз)
    std::string insertAdminSQL = "INSERT IGNORE INTO users (login, password, role) VALUES ('admin', 'admin', 'admin');";
    executeSQL(insertAdminSQL);

    loginUser();
    if (!currentUserRole.empty()) {
        menu();
    }

    mysql_close(conn);
    return 0;
}
