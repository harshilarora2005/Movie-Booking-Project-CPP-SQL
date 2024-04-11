#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <time.h>
#include <sstream>
#include <iomanip>
#include <sstream>
#include <memory>
#include <mysql.h>
#include <fstream>
#include <cstdlib>
using namespace std;
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
        const string& language, const string& date, const string& startTime, const string& endTime, double budget)
        :username(username), state(state), city(city), film(film), price(price), time(time), hall(hall),
        seats(seats), language(language), date(date), startTime(startTime), endTime(endTime), budget(budget) {}
    Booking() {
        budget = 0;

    };
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
    string getStartTime() const { return startTime; }
    string getEndTime() const { return endTime; }
    double getBudget() const { return budget; }

protected:
    string username;
    string state;
    string city;
    string film;
    string price;
    string time;
    string hall;
    int quantity;
    vector<string> seats;
    string language;
    string date;
    string startTime;
    string endTime;
    double budget;
};
string getCurrentTime() {
    auto now = chrono::system_clock::now();

    // Convert the time point to a time_t object
    time_t now_c = chrono::system_clock::to_time_t(now);

    // Convert the time_t object to a struct tm
    tm parts;
    localtime_s(&parts, &now_c);

    // Create a string stream to format the date and time
    stringstream ss;
    ss << put_time(&parts, "%Y-%m-%d %H:%M:%S");

    return ss.str();
}
// Interface for database interaction

class Database {
public:
    virtual bool add(const string& tableName, const vector<string>& columns, const vector<string>& values) = 0;
    virtual bool check(const string& tableName, const string& condition) = 0;
    virtual string get(const string& tableName, const string& column, const string& condition) = 0;
    virtual vector<string> getall(const string& tableName, const string& column) = 0;
};
// MySQL implementation of the Database interface
class MySQLDatabase : public Database {
public:
    MySQLDatabase() {
        connection = mysql_init(NULL);
        if (!connection) {
            throw runtime_error("Error initializing MySQL connection.");
        }
        if (!mysql_real_connect(connection, "localhost", "root", "", "booking", 3306, NULL, 0)) {
            throw runtime_error("Error connecting to MySQL server.");
        }
    }

    ~MySQLDatabase() {
        mysql_close(connection);
    }
    bool add(const string& tableName, const vector<string>& columns, const vector<string>& values) override {
        string query = "INSERT INTO " + tableName + " (";
        for (size_t i = 0; i < columns.size(); ++i) {
            query += columns[i];
            if (i != columns.size() - 1)
                query += ", ";
        }
        query += ") VALUES (";
        for (size_t i = 0; i < values.size(); ++i) {
            query += "'" + values[i] + "'";
            if (i != values.size() - 1)
                query += ", ";
        }
        query += ")";

        return executeQuery(query);
    }

    bool check(const string& tableName, const string& condition) override {
        string query = "SELECT COUNT(*) FROM " + tableName + " WHERE " + condition;
        MYSQL_RES* result = executeSelectQuery(query);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            int count = atoi(row[0]);
            mysql_free_result(result);
            return count > 0;
        }
        return false;
    }
    vector<string> getall(const string& tableName, const string& column) {
        string query = "SELECT " + column + " FROM " + tableName;
        MYSQL_RES* result = executeSelectQuery(query);
        vector<string> values;

        if (result) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                values.push_back(row[0]); // Append the value from the current row to the vector
            }
            mysql_free_result(result);
        }
        return values;
    }
    string get(const string& tableName, const string& column, const string& condition) override {
        string query = "SELECT " + column + " FROM " + tableName + " WHERE " + condition;
        MYSQL_RES* result = executeSelectQuery(query);
        if (result) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row) {
                string value = row[0];
                mysql_free_result(result);
                return value;
            }
            mysql_free_result(result);
        }
        return ""; // Return empty string if not found
    }

private:
    MYSQL* connection;

    bool executeQuery(const string& query) {
        if (mysql_query(connection, query.c_str()) != 0) {
            cerr << "MySQL error: " << mysql_error(connection) << endl;
            return false;
        }
        return true;
    }

    MYSQL_RES* executeSelectQuery(const string& query) {
        if (mysql_query(connection, query.c_str()) != 0) {
            cerr << "MySQL error: " << mysql_error(connection) << endl;
            return nullptr;
        }
        return mysql_store_result(connection);
    }
};
class Login {
public:
    Login(unique_ptr<Database> db) : db(move(db)) {}

    string getPassword(const string& username) {
        return db->get("users", "password", "username='" + username + "'");
    }

    bool authenticate(const string& username, const string& password) {
        return getPassword(username) == password;
    }
    bool checkUser(const string& username) {
        return db->check("users", "username='" + username + "'");
    }

    bool addUser(const User& user) {
        vector<string> columns = { "username", "password" };
        vector<string> values = { user.getUsername(), user.getPassword() };
        return db->add("users",columns,values);
    }
    void signup() {
        string username, password;
        cout << "Enter a username: ";
        cin >> username;
        cout << "Enter a password: ";
        cin >> password;

        User newUser(username, password);
        if (addUser(newUser)) {
            cout << "User registered successfully.\n";
        }
        else {
            cerr << "Failed to register user.\n";
        }
    }

    // Function to prompt user for login
    bool login() {
        string username, password;
        cout << "Enter your username: ";
        cin >> username;
        if (!checkUser(username)) {
            cerr << "Username not found. Please signup or login again with a correct username\n";
            return false;
        }
        for (int attempt = 0; attempt < 3; ++attempt) {
            cout << "Enter your password: ";
            cin >> password;

            string storedPassword = getPassword(username);
            if (storedPassword == password) {
                cout << "Login successful! Welcome, " << username << "!\n";
                return true;
            }
            else {
                cerr << "Incorrect password. Please try again.\n";
            }
        }

        cerr << "Too many incorrect attempts. Returning to main menu.\n";
        return false;
    }

private:
    unique_ptr<Database> db;
};
// Class for handling booking operations
class BookingManager: public Booking {
public:
    BookingManager(unique_ptr<Database> db) : db(move(db)) {}

    void bookTickets() {
        // Booking logic
        
        display(selectmovie());

    }
    
    void getDetails() {
        // Get current system date
        string currentDate = getCurrentTime().substr(0, 10);
        cout << "Today's date is: " << currentDate << endl;

        // Prompt for date selection within the next 7 days
        do {
            cout << "Please select the date for your outing (up to 7 days from now, in format YYYY-MM-DD): ";
            cin >> date;
        }
        // Validate selected date
        while (!isValidDate(date, currentDate, 7));
         

        do {
            cout << "Please select the starting time for your outing (between 10 am and 5 pm, in format HH:MM): ";
            cin >> startTime;
        }
        // Validate selected time
        while (!isValidTime(startTime));

        while (true) {
            cout << "Enter your budget for the outing(budget should be greater than 500 Ruppees):";
            cin >> budget;
            if (budget < 500) {
                cout << "invalid budget! please enter an amount greater than 500.";
            }
            else {
                break;
            }
        }
        //address thing idk...
    }
    bool isValidDate(const string& dateStr, const string& currentDateStr, int nDays) {
        istringstream dateStream(dateStr);
        istringstream currentStream(currentDateStr);

        int year, month, day;
        char dash;

        // Extract year, month, and day from date strings
        dateStream >> year >> dash >> month >> dash >> day;
        int currentYear, currentMonth, currentDay;
        currentStream >> currentYear >> dash >> currentMonth >> dash >> currentDay;

        // Convert dates to days since epoch
        tm selectedDate = { 0, 0, 0, day, month - 1, year - 1900 };
        tm currentDate = { 0, 0, 0, currentDay, currentMonth - 1, currentYear - 1900 };

        time_t selectedTime = mktime(&selectedDate);
        time_t currentTime = mktime(&currentDate);

        // Calculate difference in seconds
        double difference = difftime(selectedTime, currentTime);

        // Check if difference is within nDays
        bool condition= difference >= 0 && difference <= nDays * 24 * 60 * 60;
        if (!condition) {
            cout << "ENTER A VALID DATE" << endl;
            return condition;
        }
        return condition;
    }
    bool isValidTime(const string& time) {
        stringstream ss(time);
        int hour, minute;
        char colon;
        ss >> hour >> colon >> minute;

        bool condition = (hour >= 10 && hour <= 16) && (minute >= 0 || minute <= 60) && (colon == ':');
        if (!condition) {
            cout << "ENTER VALID TIME!!" << endl;
            return condition;
        }
        return condition;
    }
    void display(int movie_id) {
        // Retrieve movie name from database
        film = db->get("movies", "movie_name", "sno='" + to_string(movie_id) + "'");

        // Generate filename from movie name
        string filename = film + ".txt";

        // Check if the file already exists
        ifstream file(filename);
        vector<vector<char>> seats_array(12, vector<char>(12));
        if (file) {
            // Read movie name from the first line (header)
            getline(file, film);

            // Read the seating arrangement from the file
            for (int i = 0; i < 12; ++i) {
                string line;
                getline(file, line);
                istringstream iss(line);
                for (int j = 0; j < 12; ++j) {
                    iss >> seats_array[i][j];
                }
            }

            file.close();

            // Display the seating arrangement

        }
        else {
            // File doesn't exist, generate new array
            srand(static_cast<unsigned>(std::time(nullptr))); //  Seed for random number generator

            // Generate random 2D array of 12x12 areas

            for (int i = 0; i < 12; ++i) {
                for (int j = 0; j < 12; ++j) {
                    // Randomly mark seat as occupied or unoccupied
                    seats_array[i][j] = (rand() % 2 == 0) ? ' ' : 'x';
                }
            }

            // Store the array in the file
            ofstream outfile(filename);
            if (outfile) {
                // Write movie name as header
                outfile << "Movie: " << film << endl;

                // Write array to file
                for (const auto& row : seats_array) {
                    for (char seat : row) {
                        outfile << setw(2) << seat << " ";
                    }
                    outfile << endl;
                }
                outfile.close();
            }
            else {
                cerr << "Error: Could not create file " << filename << endl;
                return;
            }
        }

        // Display the seating arrangement
        // For simplicity, we'll just print it to the console
        cout << "Seating arrangement for " << film << ":" << endl;
        cout << "---------------------Screen--------------------------" << endl;
        cout << "   ";
        for (char col = 'A'; col <= 'L'; ++col) {
            cout << col << "  ";
        }
        cout << endl;
        for (int i = 0; i < 12; ++i) {
            cout << setw(2) << (i + 1) << " ";
            cout << "|";
            for (int j = 0; j < 12; ++j) {
                cout << setw(2) << seats_array[i][j] << " ";
            }
            //cout << "|";
            cout << endl;
        }
        cout << "Enter number of seats you want to choose:";
        cin >> quantity;
        char c;
        int n;
        string seat;
        int i = 0;
        for (;i < quantity;) { 
            cout << "Enter a seat(in format like A6, B7, D10 etc";
            cin >> seat;
            istringstream mystream;
            mystream >> c >> n;
            c = (char)tolower(c);
            if (c >= 'a' && c <= 'l' && n >= 1 && n <= 11) {
                if (seats_array[n - 1][c - 97] == ' ') {
                    seats.push_back(seat);
                    i++;
                    //update seat matrix
                    seats_array[n - 1][c - 97] = 'x';
                    ofstream outfile(filename,ofstream::trunc);
                    //trunc is used to overrite file contents.
                    if (outfile) {
                        // Write movie name as header
                        outfile << "Movie: " << film << endl;

                        // Write array to file
                        for (const auto& row : seats_array) {
                            for (char seat : row) {
                                outfile << setw(2) << seat << " ";
                            }
                            outfile << endl;
                        }
                        outfile.close();
                    }
                    else {
                        cerr << "Error: Could not create file " << filename << endl;
                        return;
                    }
                }
                else {
                    cout << "ENTER VALID SEAT, its already picked" << endl;
                    continue;

                }
            }
            else {
                cout << "ENTER A VALID SEAT, ITS OUT OF THEATRE BOUNDS!!" << endl;
            }

        }
        
    }
    int selectmovie() {
        // Get all movie names along with their IDs from the database
        vector<string> movieNames = db->getall("movies", "movie_name");
        int numMovies = movieNames.size();

        // Display movie names along with their IDs
        cout << "Available Movies:" << endl;
        for (int i = 0; i < numMovies; ++i) {
            cout << (i + 1) << ". " << movieNames[i] << endl;
        }

        // Prompt user to select a movie ID
        int selectedMovieId;
        cout << "Enter the movie ID: ";
        cin >> selectedMovieId;

        // Validate user input
        while (selectedMovieId < 1 || selectedMovieId > numMovies) {
            cout << "Invalid movie ID. Please enter a valid ID: ";
            cin >> selectedMovieId;
        }

        return selectedMovieId;
    }

private:
    unique_ptr<Database> db;
    
};

// Main function
int main() {
        unique_ptr<Database> db = make_unique<MySQLDatabase>();
        unique_ptr<Database> db_log= make_unique<MySQLDatabase>();
        Login login(move(db_log));
        BookingManager bookingManager(move(db));
        string username, password;
        int choice;
        b:
        cout << "WELCOME TO CINEBLISS, AN EXPERT LOCATION TO HANG OUT WITH FRIENDS OR FAMILY." << endl << endl;
        do {
            cout << "\nMain Menu\n";
            cout << "\t1. Sign Up\n";
            cout << "\t2. Login\n";
            cout << "\t3. Exit\n";
            cout << "\tEnter your choice: ";
            cin >> choice;

            if (choice == 1) {
                login.signup();
            }
            else if (choice == 2) {
                if (login.login()) {
                    a:
                    cout << "\n\t\tENTER YOUR DETAILS BEFORE WE CONTINUE!!" << endl;
                    bookingManager.getDetails();
                    //code for choosing between cafe booking & movie booking.
                    cout << "\n\nWhat would you like to do?" << endl << endl;
                    cout << "\t1. MOVIES " << endl;
                    cout << "\t2. CAFE OR FINE DINING." << endl;
                    int choice1;
                    while (true) {
                        cout << "\nENTER CHOICE:";
                        cin >> choice1;
                        if (choice1 == 1 || choice1 == 2 || choice1 == 3 ){
                            break;
                        }
                        else {
                            cout << "\nCHOOSE A VALID OPTION!!";
                        }
                    }if (choice1 == 1) {
                        c:
                        cout << "\n \tWELCOME TO MOVIES! ~ spend your time watching the classics or the new jams\n\t\t\ttheres everything for everyone!" << endl;
                        cout << "\n WHAT WOULD YOU LIKE TO DO?" << endl;
                        cout << "\t1. BOOK A MOVIE." << endl;
                        cout << "\t2. MOVIE RECOMMENDATIONS." << endl;
                        cout << "\t3. CHECK YOUR TICKETS." << endl;
                        cout << "\t4. Exit" << endl;
                        int choice2;
                        while (true) {
                            cout << "\nENTER CHOICE:";
                            cin >> choice2;
                            if (choice2>0 && choice2<5) {
                                break;
                            }
                            else {
                                cout << "\nCHOOSE A VALID OPTION!!";
                            }
                        }if (choice2 == 1) {
                            bookingManager.bookTickets();
                            goto c;
                        }
                        else if (choice2 == 2) {
                            goto c;
                        }
                        else if (choice2 == 3) {
                            goto c;
                        
                        }
                        else if (choice2 == 4) {
                            goto a;//start of cinebliss
                        }


                    }else{
                        goto b;//start of page
                    
                    }

                }
            }
            else if (choice == 3) {
                cout << "Exiting program.\n";
                return 0;
            }
            else {
                cout << "ENTER A VALID CHOICE";
            }
        } while (choice != 3);

    return 0;
}
