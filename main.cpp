#include <iostream>
#include <string>
#include <vector>
#include <mysql/mysql.h> // подключаем библиотеку mysql
using namespace std;

// данные для подключения к БД
const string DB_HOST = "localhost";
const string DB_USER = "root";
const string DB_PASS = "password";
const string DB_NAME = "bus_schedule";

// структура для рейсов
struct Flight {
    int id;
    string flight_number;
    string bus_type;
    string destination;
    string departure_time;
    string arrival_time;
};

// структура для пользователей
struct User {
    int id;
    string login;
    string password;
    string role;
};

MYSQL* connectDB() {
    MYSQL* conn = mysql_init(0);
    conn = mysql_real_connect(conn, DB_HOST.c_str(), DB_USER.c_str(), DB_PASS.c_str(), DB_NAME.c_str(), 0, NULL, 0);
    if (conn) {
        cout << "успешное подключение к базе данных!" << endl;
    } else {
        cerr << "ошибка подключения: " << mysql_error(conn) << endl;
        exit(1);
    }
    return conn;
}

bool checkLogin(MYSQL* conn, const string& login, const string& password, User& user) {
    string query = "SELECT id, role FROM users WHERE login='" + login + "' AND password='" + password + "';";
    if (mysql_query(conn, query.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row) {
            user.id = stoi(row[0]);
            user.login = login;
            user.role = row[1];
            return true;
        }
    }
    return false;
}

void registerUser(MYSQL* conn) {
    string login, password;
    cout << "введите логин: ";
    cin >> login;
    cout << "введите пароль: ";
    cin >> password;

    string query = "INSERT INTO users (login, password, role) VALUES ('" + login + "', '" + password + "', 'user');";
    if (mysql_query(conn, query.c_str()) == 0) {
        cout << "пользователь успешно зарегистрирован!\n";
    } else {
        cerr << "ошибка регистрации: " << mysql_error(conn) << endl;
    }
}

void showAllFlights(MYSQL* conn) {
    string query = "SELECT * FROM flights;";
    if (mysql_query(conn, query.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row;
        cout << "список рейсов:\n";
        while ((row = mysql_fetch_row(res))) {
            cout << "номер рейса: " << row[1]
                 << ", тип автобуса: " << row[2]
                 << ", пункт назначения: " << row[3]
                 << ", время отправления: " << row[4]
                 << ", время прибытия: " << row[5] << endl;
        }
        mysql_free_result(res);
    } else {
        cerr << "ошибка выполнения запроса: " << mysql_error(conn) << endl;
    }
}

void addFlight(MYSQL* conn) {
    string flight_number, bus_type, destination, departure_time, arrival_time;
    cout << "введите номер рейса: ";
    cin >> flight_number;
    cout << "введите тип автобуса: ";
    cin >> bus_type;
    cout << "введите пункт назначения: ";
    cin >> destination;
    cout << "введите время отправления (YYYY-MM-DD HH:MM): ";
    cin >> departure_time;
    cout << "введите время прибытия (YYYY-MM-DD HH:MM): ";
    cin >> arrival_time;

    string query = "INSERT INTO flights (flight_number, bus_type, destination, departure_time, arrival_time) VALUES ('" +
                   flight_number + "', '" + bus_type + "', '" + destination + "', '" + departure_time + "', '" + arrival_time + "');";

    if (mysql_query(conn, query.c_str()) == 0) {
        cout << "рейс успешно добавлен!\n";
    } else {
        cerr << "ошибка добавления рейса: " << mysql_error(conn) << endl;
    }
}

void adminMenu(MYSQL* conn) {
    int choice;
    while (true) {
        cout << "\n1. показать все рейсы\n2. добавить рейс\n3. выйти\n";
        cout << "выберите действие: ";
        cin >> choice;

        switch (choice) {
            case 1: showAllFlights(conn); break;
            case 2: addFlight(conn); break;
            case 3: return;
            default: cout << "неверный выбор!\n";
        }
    }
}

void userMenu(MYSQL* conn) {
    int choice;
    while (true) {
        cout << "\n1. показать все рейсы\n2. выйти\n";
        cout << "выберите действие: ";
        cin >> choice;

        switch (choice) {
            case 1: showAllFlights(conn); break;
            case 2: return;
            default: cout << "неверный выбор!\n";
        }
    }
}

int main() {
    MYSQL* conn = connectDB();
    User currentUser;

    int action;
    cout << "1. войти\n2. зарегистрироваться\nвыберите действие: ";
    cin >> action;

    if (action == 1) {
        string login, password;
        cout << "введите логин: ";
        cin >> login;
        cout << "введите пароль: ";
        cin >> password;

        if (checkLogin(conn, login, password, currentUser)) {
            cout << "добро пожаловать, " << currentUser.login << "!\n";
            if (currentUser.role == "admin") {
                adminMenu(conn);
            } else {
                userMenu(conn);
            }
        } else {
            cout << "неверный логин или пароль!\n";
        }
    } else if (action == 2) {
        registerUser(conn);
    } else {
        cout << "неверный выбор!\n";
    }

    mysql_close(conn);
    return 0;
}
