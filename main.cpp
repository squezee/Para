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
            mysql_free_result(res); // освобождаем результат запроса
            return true;
        }
        mysql_free_result(res);
    }
    return false;
}
void deleteFlight(MYSQL* conn) {
    int flightId;
    cout << "введите ID рейса, который хотите удалить: ";
    cin >> flightId;

    string query = "DELETE FROM flights WHERE id=" + to_string(flightId) + ";";
    if (mysql_query(conn, query.c_str()) == 0) {
        cout << "рейс успешно удален!\n";
    } else {
        cerr << "ошибка удаления рейса: " << mysql_error(conn) << endl;
    }
}

void editFlight(MYSQL* conn) {
    int flightId;
    string flight_number, bus_type, destination, departure_time, arrival_time;

    cout << "введите ID рейса, который хотите изменить: ";
    cin >> flightId;
    cin.ignore(); // очищаем буфер ввода
    cout << "введите новый номер рейса (оставьте пустым для сохранения текущего): ";
    getline(cin, flight_number);
    cout << "введите новый тип автобуса (оставьте пустым для сохранения текущего): ";
    getline(cin, bus_type);
    cout << "введите новый пункт назначения (оставьте пустым для сохранения текущего): ";
    getline(cin, destination);
    cout << "введите новое время отправления (оставьте пустым для сохранения текущего): ";
    getline(cin, departure_time);
    cout << "введите новое время прибытия (оставьте пустым для сохранения текущего): ";
    getline(cin, arrival_time);

    string query = "UPDATE flights SET ";
    if (!flight_number.empty()) query += "flight_number='" + flight_number + "', ";
    if (!bus_type.empty()) query += "bus_type='" + bus_type + "', ";
    if (!destination.empty()) query += "destination='" + destination + "', ";
    if (!departure_time.empty()) query += "departure_time='" + departure_time + "', ";
    if (!arrival_time.empty()) query += "arrival_time='" + arrival_time + "', ";
    query.pop_back(); // удаляем последнюю запятую
    query.pop_back();
    query += " WHERE id=" + to_string(flightId) + ";";

    if (mysql_query(conn, query.c_str()) == 0) {
        cout << "рейс успешно обновлен!\n";
    } else {
        cerr << "ошибка обновления рейса: " << mysql_error(conn) << endl;
    }
}

void sortFlights(MYSQL* conn, const string& criteria) {
    string query = "SELECT * FROM flights ORDER BY " + criteria + ";";
    if (mysql_query(conn, query.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row;

        cout << "\n===================================================\n";
        cout << "| ID | Номер рейса | Тип автобуса | Пункт назначения | Время отправления     | Время прибытия       |\n";
        cout << "===================================================\n";

        while ((row = mysql_fetch_row(res))) {
            printf("| %-2s | %-12s | %-12s | %-18s | %-20s | %-20s |\n",
                   row[0], row[1], row[2], row[3], row[4], row[5]);
        }
        cout << "===================================================\n";
        mysql_free_result(res);
    } else {
        cerr << "ошибка выполнения запроса: " << mysql_error(conn) << endl;
    }
}

void addUser(MYSQL* conn) {
    string login, password, role;
    cout << "введите логин: ";
    cin.ignore();
    getline(cin, login);
    cout << "введите пароль: ";
    getline(cin, password);
    cout << "введите роль (admin/user): ";
    getline(cin, role);

    string query = "INSERT INTO users (login, password, role) VALUES ('" + login + "', '" + password + "', '" + role + "');";
    if (mysql_query(conn, query.c_str()) == 0) {
        cout << "пользователь успешно добавлен!\n";
    } else {
        cerr << "ошибка добавления пользователя: " << mysql_error(conn) << endl;
    }
}

void editUser(MYSQL* conn) {
    int userId;
    string login, password, role;

    cout << "введите ID пользователя, которого хотите изменить: ";
    cin >> userId;
    cin.ignore(); // очищаем буфер ввода
    cout << "введите новый логин (оставьте пустым для сохранения текущего): ";
    getline(cin, login);
    cout << "введите новый пароль (оставьте пустым для сохранения текущего): ";
    getline(cin, password);
    cout << "введите новую роль (оставьте пустым для сохранения текущей): ";
    getline(cin, role);

    string query = "UPDATE users SET ";
    if (!login.empty()) query += "login='" + login + "', ";
    if (!password.empty()) query += "password='" + password + "', ";
    if (!role.empty()) query += "role='" + role + "', ";
    query.pop_back(); // удаляем последнюю запятую
    query.pop_back();
    query += " WHERE id=" + to_string(userId) + ";";

    if (mysql_query(conn, query.c_str()) == 0) {
        cout << "пользователь успешно обновлен!\n";
    } else {
        cerr << "ошибка обновления пользователя: " << mysql_error(conn) << endl;
    }
}

void deleteUser(MYSQL* conn) {
    int userId;
    cout << "введите ID пользователя, которого хотите удалить: ";
    cin >> userId;

    string query = "DELETE FROM users WHERE id=" + to_string(userId) + ";";
    if (mysql_query(conn, query.c_str()) == 0) {
        cout << "пользователь успешно удален!\n";
    } else {
        cerr << "ошибка удаления пользователя: " << mysql_error(conn) << endl;
    }
}
void loginUser(MYSQL* conn, User& currentUser) {
    string login, password;
    cout << "введите логин: ";
    cin.ignore(); // очищаем буфер ввода
    getline(cin, login);
    cout << "введите пароль: ";
    getline(cin, password);

    if (checkLogin(conn, login, password, currentUser)) {
        cout << "добро пожаловать, " << currentUser.login << "!\n";
    } else {
        cout << "неверный логин или пароль!\n";
    }
}


void registerUser(MYSQL* conn) {
    string login, password;
    cout << "введите логин: ";
    cin.ignore(); // очищаем буфер ввода
    getline(cin, login);
    cout << "введите пароль: ";
    getline(cin, password);

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

        cout << "\n===================================================\n";
        cout << "| ID | Номер рейса | Тип автобуса | Пункт назначения | Время отправления     | Время прибытия       |\n";
        cout << "===================================================\n";

        while ((row = mysql_fetch_row(res))) {
            printf("| %-2s | %-12s | %-12s | %-18s | %-20s | %-20s |\n",
                   row[0], row[1], row[2], row[3], row[4], row[5]);
        }
        cout << "===================================================\n";
        mysql_free_result(res);
    } else {
        cerr << "ошибка выполнения запроса: " << mysql_error(conn) << endl;
    }
}

void showAllUsers(MYSQL* conn) {
    string query = "SELECT id, login, role FROM users;";
    if (mysql_query(conn, query.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row;

        cout << "\nсписок пользователей:\n";
        cout << "id | логин | роль\n";
        while ((row = mysql_fetch_row(res))) {
            cout << row[0] << " | " << row[1] << " | " << row[2] << endl;
        }
        mysql_free_result(res);
    } else {
        cerr << "ошибка выполнения запроса: " << mysql_error(conn) << endl;
    }
}
void findFlightsByDeparture(MYSQL* conn) {
    string destination, departure_time;
    cout << "введите пункт назначения: ";
    cin.ignore(); // очищаем буфер ввода
    getline(cin, destination);
    cout << "введите время отправления (YYYY-MM-DD HH:MM): ";
    getline(cin, departure_time);

    string query = "SELECT * FROM flights WHERE destination = '" + destination +
                   "' AND departure_time >= '" + departure_time + "' ORDER BY departure_time ASC;";
    if (mysql_query(conn, query.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row;

        cout << "\nближайшие рейсы:\n";
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
void findFlightsByArrival(MYSQL* conn) {
    string destination, arrival_time;
    cout << "введите пункт назначения: ";
    cin.ignore(); // очищаем буфер ввода
    getline(cin, destination);
    cout << "введите время прибытия (YYYY-MM-DD HH:MM): ";
    getline(cin, arrival_time);

    string query = "SELECT * FROM flights WHERE destination = '" + destination +
                   "' AND arrival_time <= '" + arrival_time + "' ORDER BY arrival_time ASC;";
    if (mysql_query(conn, query.c_str()) == 0) {
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row;

        cout << "\nближайшие рейсы:\n";
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
    cin.ignore(); // очищаем буфер ввода
    getline(cin, flight_number);
    cout << "введите тип автобуса: ";
    getline(cin, bus_type);
    cout << "введите пункт назначения: ";
    getline(cin, destination);
    cout << "введите время отправления (YYYY-MM-DD HH:MM): ";
    getline(cin, departure_time);
    cout << "введите время прибытия (YYYY-MM-DD HH:MM): ";
    getline(cin, arrival_time);

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
        cout << "\nМодуль администратора:\n";
        cout << "1. показать все рейсы\n";
        cout << "2. добавить рейс\n";
        cout << "3. удалить рейс\n";
        cout << "4. редактировать рейс\n";
        cout << "5. отсортировать рейсы\n";
        cout << "6. показать всех пользователей\n";
        cout << "7. добавить пользователя\n";
        cout << "8. редактировать пользователя\n";
        cout << "9. удалить пользователя\n";
        cout << "10. выйти\n";
        cout << "выберите действие: ";
        cin >> choice;

        switch (choice) {
            case 1: showAllFlights(conn); break;
            case 2: addFlight(conn); break;
            case 3: deleteFlight(conn); break;
            case 4: editFlight(conn); break;
            case 5: {
                int sortOption;
                cout << "\nСортировать по:\n";
                cout << "1. номеру рейса\n";
                cout << "2. типу автобуса\n";
                cout << "3. пункту назначения\n";
                cout << "выберите действие: ";
                cin >> sortOption;
                if (sortOption == 1) sortFlights(conn, "flight_number");
                else if (sortOption == 2) sortFlights(conn, "bus_type");
                else if (sortOption == 3) sortFlights(conn, "destination");
                else cout << "неверный выбор!\n";
                break;
            }
            case 6: showAllUsers(conn); break;
            case 7: addUser(conn); break;
            case 8: editUser(conn); break;
            case 9: deleteUser(conn); break;
            case 10: return;
            default: cout << "неверный выбор!\n";
        }
    }
}



void userMenu(MYSQL* conn) {
    int choice;
    while (true) {
        cout << "\n1. показать все рейсы\n2. найти рейсы по отправлению\n3. найти рейсы по прибытию\n4. выйти\n";
        cout << "выберите действие: ";
        cin >> choice;

        switch (choice) {
            case 1: showAllFlights(conn); break;
            case 2: findFlightsByDeparture(conn); break;
            case 3: findFlightsByArrival(conn); break;
            case 4: return;
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
        loginUser(conn, currentUser);
        if (!currentUser.login.empty()) {
            if (currentUser.role == "admin") {
                adminMenu(conn);
            } else {
                userMenu(conn);
            }
        }
    } else if (action == 2) {
        registerUser(conn);
    } else {
        cout << "неверный выбор!\n";
    }

    mysql_close(conn);
    return 0;
}

