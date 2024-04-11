#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <mysql.h>

using namespace std;
// Class representing a User
class User {
public:
    User(const string& username, const string& password) : username(username), password(password) {}

    string getUsername() const { return username; }
    string getPassword() const { return password; }

private:
    string username;
    string password;
};

// Class representing a Booking
class Booking {
public:
    Booking(const string& username, const string& state, const string& city, const string& film,
        const string& price, const string& time, const string& hall, const vector<string>& seats,
        const string& language, const string& date)
        : username(username), state(state), city(city), film(film), price(price), time(time), hall(hall),
        seats(seats), language(language), date(date) {}

    string getUsername() const { return username; }
    string getState() const { return state; }
    string getCity() const { return city; }
    string getFilm() const { return film; }
    string getPrice() const { return price; }
    string getTime() const { return time; }
    string getHall() const { return hall; }
    vector<string> getSeats() const { return seats; }
    string getLanguage() const { return language; }
    string getDate() const { return date; }

private:
    string username;
    string state;
    string city;
    string film;
    string price;
    string time;
    string hall;
    vector<string> seats;
    string language;
    string date;
};

// Interface for database interaction
class Database {
public:
    virtual bool addUser(const User& user) = 0;
    virtual bool checkUser(const string& username) = 0;
    virtual bool authenticate(const string& username, const string& password) = 0;
    virtual bool saveBooking(const Booking& booking) = 0;
};

// MySQL implementation of the Database interface
class MySQLDatabase : public Database {
public:
    MySQLDatabase() {
        connection = mysql_init(NULL);
        if (!connection) {
            throw :runtime_error("Error initializing MySQL connection.");
        }
        if (!mysql_real_connect(connection, "localhost", "root", "", "booking", 3306, NULL, 0)) {
            throw :runtime_error("Error connecting to MySQL server.");
        }
    }

    ~MySQLDatabase() {
        mysql_close(connection);
    }

    bool addUser(const User& user) override {
        :string query = "INSERT INTO users (username, password) VALUES ('" + user.getUsername() + "', '" + user.getPassword() + "')";
        return executeQuery(query);
    }

    bool checkUser(const :string& username) override {
        :string query = "SELECT COUNT(*) FROM users WHERE username='" + username + "'";
        MYSQL_RES* result = executeSelectQuery(query);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            int count = atoi(row[0]);
            mysql_free_result(result);
            return count > 0;
        }
        return false;
    }

    bool authenticate(const :string& username, const :string& password) override {
        :string query = "SELECT COUNT(*) FROM users WHERE username='" + username + "' AND password='" + password + "'";
        MYSQL_RES* result = executeSelectQuery(query);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            int count = atoi(row[0]);
            mysql_free_result(result);
            return count > 0;
        }
        return false;
    }

    bool saveBooking(const Booking& booking) override {
        // Assuming there's a table called bookings with appropriate schema
        :string query = "INSERT INTO bookings (username, state, city, film, price, time, hall, seats, language, date) VALUES ('"
            + booking.getUsername() + "', '" + booking.getState() + "', '" + booking.getCity() + "', '"
            + booking.getFilm() + "', '" + booking.getPrice() + "', '" + booking.getTime() + "', '"
            + booking.getHall() + "', '" + seatsToString(booking.getSeats()) + "', '" + booking.getLanguage()
            + "', '" + booking.getDate() + "')";
        return executeQuery(query);
    }

private:
    MYSQL* connection;

    bool executeQuery(const :string& query) {
        if (mysql_query(connection, query.c_str()) != 0) {
            :cerr << "MySQL error: " << mysql_error(connection) << :endl;
            return false;
        }
        return true;
    }

    MYSQL_RES* executeSelectQuery(const :string& query) {
        if (mysql_query(connection, query.c_str()) != 0) {
            :cerr << "MySQL error: " << mysql_error(connection) << :endl;
            return nullptr;
        }
        return mysql_store_result(connection);
    }

    :string seatsToString(const :vector<:string>& seats) {
        :string seatsStr;
        for (const auto& seat : seats) {
            seatsStr += seat + ",";
        }
        if (!seatsStr.empty()) {
            seatsStr.pop_back(); // Remove the last comma
        }
        return seatsStr;
    }
};


// Class for handling booking operations
class BookingManager {
public:
    BookingManager(unique_ptr<Database> db) : db(move(db)) {}

    void bookTickets(const string& username) {
        // Booking logic
    }

private:
    unique_ptr<Database> db;
};

// Main function
int main() {
    try {
        unique_ptr<Database> db = make_unique<MySQLDatabase>();
        BookingManager bookingManager(move(db));
        string username, password;
        cout << "Enter username: ";
        cin >> username;
        cout << "Enter password: ";
        cin >> password;

        // Create User object
        User newUser(username, password);

        // Add user
        if (bookingManager.addUser(newUser)) {
            cout << "User added successfully." << endl;
        }
        else {
            cerr << "Failed to add user." << endl;
        }
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
