#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <time.h>
#include <sstream>
#include <iomanip>
#include <memory>
#include <mysql.h>
#include <thread>
#include <fstream>
#include <cstdlib>
#include <random>
#include <limits>
#include <map>
#include <algorithm>
#include <conio.h>

using namespace std;
string REAL_USERNAME;
void delay()
{
	this_thread::sleep_for(chrono::seconds(2));
}

void clear()
{
	system("cls");
}

class User
{
public:
	User(const string& user, const string& pass) {
		username = user;
		string secure_pass;
		for (char i : pass) {
			secure_pass += (i + 5);
		}
		password = secure_pass;
	}

	string getUsername() const { return username; }
	string getPassword() const {
		string resolved_pass;
		for (char i : password) {
			resolved_pass += (i - 5);
		}
		return resolved_pass;
	}

private:
	string username;
	string password;
};

// Class representing a Booking
class Booking
{
public:
	Booking()
	{
		budget = 0;
		quantity = 0;
		hours = 0;
	}
	Booking(const string& state, const string& city, const string& landmark, const string& film,
		const string& price, const string& time, const string& hall, const vector<string>& seats,
		const string& language, const string& date, const string& startTime, const string& endTime, double budget, const int quan, int hours)
		:state(state), city(city), landmark(landmark), film(film), price(price), time(time), hall(hall),
		seats(seats), language(language), date(date), startTime(startTime), endTime(endTime), budget(budget), quantity(quan), hours(hours) {}

	string getState() const { return state; }
	string getCity() const { return city; }
	string getLandmark() const { return landmark; }
	string getFilm() const { return film; }
	string getPrice() const { return price; }
	string getTime() const { return time; }
	string getHall() const { return hall; }
	int getQuantity() const { return quantity; }
	vector<string> getSeats() const { return seats; }
	string getLanguage() const { return language; }
	string getDate() const { return date; }
	string getStartTime() const { return startTime; }
	string getEndTime() const { return endTime; }
	double getBudget() const { return budget; }
	int getHours() const { return hours; }
	void setBudget(int x) {
		budget = x;

	}

protected:
	string state;
	string city;
	string landmark;
	string film;
	string price;
	string time;
	string hall;
	int hours;
	int quantity;
	vector<string> seats;
	string language;
	string date;
	string startTime;
	string endTime;
	double budget;
};

string getCurrentTime()
{
	auto now = chrono::system_clock::now();
	time_t now_c = chrono::system_clock::to_time_t(now);
	tm parts;
	localtime_s(&parts, &now_c);
	stringstream ss;
	ss << put_time(&parts, "%Y-%m-%d %H:%M:%S");
	return ss.str();
}
// Interface for database interaction

class Database
{
public:
	virtual bool add(const string& tableName, const vector<string>& columns, const vector<string>& values) = 0;
	virtual bool check(const string& tableName, const string& condition) = 0;
	virtual string get(const string& tableName, const string& column, const string& condition) = 0;
	virtual vector<string> getall(const string& tableName, const string& column) = 0;
	virtual vector<map<string, string>> getBookingDetails(const string& username) = 0;
};


// MySQL implementation of the Database interface
class MySQLDatabase : public Database
{
public:
	MySQLDatabase()
	{
		connection = mysql_init(NULL);
		if (!connection)
		{
			throw runtime_error("Error initializing MySQL connection.");
		}
		if (!mysql_real_connect(connection, "localhost", "root", "", "booking", 3306, NULL, 0))
		{
			throw runtime_error("Error connecting to MySQL server.");
		}
	}

	~MySQLDatabase()
	{
		mysql_close(connection);
	}

	bool add(const string& tableName, const vector<string>& columns, const vector<string>& values) override
	{
		string query = "INSERT INTO " + tableName + " (";
		for (size_t i = 0; i < columns.size(); ++i)
		{
			query += columns[i];
			if (i != columns.size() - 1)
				query += ", ";
		}
		query += ") VALUES (";
		for (size_t i = 0; i < values.size(); ++i)
		{
			query += "'" + values[i] + "'";
			if (i != values.size() - 1)
				query += ", ";
		}
		query += ")";

		return executeQuery(query);
	}
	vector<map<string, string>> getBookingDetails(const string& username) {
		MYSQL_ROW row;
		string query = "SELECT * FROM Bookings WHERE username = '" + username + "'";
		MYSQL_RES* result = executeSelectQuery(query);
		vector<map<string, string>> bookings; // Vector to store multiple bookings
		if (result) {
			// Fetch rows
			while ((row = mysql_fetch_row(result))) {
				// Map to store booking details for each row
				map<string, string> bookingData;

				// Get column names
				MYSQL_FIELD* fields = mysql_fetch_fields(result);
				unsigned int num_fields = mysql_num_fields(result);

				// Iterate over columns
				for (unsigned int i = 0; i < num_fields; i++) {
					bookingData[fields[i].name] = row[i] ? row[i] : "NULL";
				}

				// Store booking details in the vector
				bookings.push_back(bookingData);
			}

			// Free result
			mysql_free_result(result);
		}
		else {
			cerr << "No bookings found for username: " << username << endl;
			mysql_free_result(result);
			return bookings;
		}
	}

	bool check(const string& tableName, const string& condition) override
	{
		string query = "SELECT COUNT(*) FROM " + tableName + " WHERE " + condition;
		MYSQL_RES* result = executeSelectQuery(query);
		if (result)
		{
			MYSQL_ROW row = mysql_fetch_row(result);
			int count = atoi(row[0]);
			mysql_free_result(result);
			return count > 0;
		}
		return false;
	}

	vector<string> getall(const string& tableName, const string& column)
	{
		string query = "SELECT " + column + " FROM " + tableName;
		MYSQL_RES* result = executeSelectQuery(query);
		vector<string> values;

		if (result)
		{
			MYSQL_ROW row;
			while ((row = mysql_fetch_row(result)))
			{
				values.push_back(row[0]); // Append the value from the current row to the vector
			}
			mysql_free_result(result);
		}
		return values;
	}

	string get(const string& tableName, const string& column, const string& condition) override
	{
		string query = "SELECT " + column + " FROM " + tableName + " WHERE " + condition;
		MYSQL_RES* result = executeSelectQuery(query);
		if (result)
		{
			MYSQL_ROW row = mysql_fetch_row(result);
			if (row)
			{
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

	bool executeQuery(const string& query)
	{
		if (mysql_query(connection, query.c_str()) != 0)
		{
			cerr << "MySQL error: " << mysql_error(connection) << endl;
			return false;
		}
		return true;
	}

	MYSQL_RES* executeSelectQuery(const string& query)
	{
		if (mysql_query(connection, query.c_str()) != 0)
		{
			cerr << "MySQL error: " << mysql_error(connection) << endl;
			return nullptr;
		}
		return mysql_store_result(connection);
	}
};

class Login :public Booking
{
public:
	Login(unique_ptr<Database> db) : db(move(db)) {}

	string getPassword(const string& username)
	{
		return db->get("users", "password", "username='" + username + "'");
	}

	bool authenticate(const string& username, const string& password)
	{
		return getPassword(username) == password;
	}

	bool checkUser(const string& username)
	{
		return db->check("users", "username='" + username + "'");
	}

	bool addUser(const User& user)
	{
		vector<string> columns = { "username", "password" };
		vector<string> values = { user.getUsername(), user.getPassword() };
		return db->add("users", columns, values);
	}

	void signup()
	{
		string username, password, confirmPassword;
		cout << "----------------------------------------------------\n";
		cout << "              Welcome to Signup Page                \n";
		cout << "----------------------------------------------------\n";
		cout << "Enter a username: ";
		cin >> username;

		cout << "Enter a password: ";
		// Masking password input with asterisks
		char ch;
		while ((ch = _getch()) != '\r')
		{
			password.push_back(ch);
			cout << '*';
		}
		cout << endl;

		cout << "Re-enter your password: ";
		// Masking password input with asterisks
		confirmPassword = "";
		while ((ch = _getch()) != '\r')
		{
			confirmPassword.push_back(ch);
			cout << '*';
		}
		cout << endl;

		if (password != confirmPassword)
		{
			cerr << "Passwords do not match. Please try again.\n";
			delay();
			clear();
			return;
		}

		User newUser(username, password);

		if (addUser(newUser))
		{
			cout << "----------------------------------------------------\n";
			cout << "       User registered successfully.                \n";
			cout << "----------------------------------------------------\n";
		}
		else
		{
			cerr << "Failed to register user.\n";
		}
		delay();
		clear();



	}

	// Function to prompt user for login
	bool login()
	{
		string password;
		cout << "----------------------------------------------------\n";
		cout << "                Welcome to Login Page               \n";
		cout << "----------------------------------------------------\n";
		cout << "Enter your username: ";
		cin >> REAL_USERNAME;

		if (!checkUser(REAL_USERNAME))
		{
			cerr << "Username not found. Please signup or login again with a correct username\n";
			return false;
		}

		for (int attempt = 0; attempt < 3; ++attempt)
		{
			cout << "Enter your password: ";
			// Masking password input with asterisks
			char ch;
			while ((ch = _getch()) != '\r')
			{
				password.push_back(ch);
				cout << '*';
			}
			cout << endl;

			string storedPassword = getPassword(REAL_USERNAME);
			if (storedPassword == password)
			{
				cout << "----------------------------------------------------\n";
				cout << "     Login successful! Welcome, " << REAL_USERNAME << "!     \n";
				cout << "----------------------------------------------------\n";
				delay();
				clear();
				return true;
			}
			else
			{
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
class BookingManager : public Booking
{
public:
	BookingManager(unique_ptr<Database> db) : db(move(db)) {}

	void saveTicket() {
		vector<string> columns = { "username", "state", "city", "film", "price", "time", "hall", "quantity", "seats", "language", "date", "startTime", "endTime" };
		string seatsString;
		for (const string& seat : seats) {
			seatsString += seat + ",";
		}
		// Remove the trailing comma
		seatsString.pop_back();
		vector<string> values = { REAL_USERNAME, getState(), getCity(), getFilm(), getPrice(), getTime(), getHall(), to_string(getQuantity()) };
		values.push_back(seatsString);
		values.push_back(getLanguage());
		values.push_back(getDate());
		values.push_back(getStartTime());
		values.push_back(getEndTime());
		bool success = db->add("Bookings", columns, values);
		if (success) {
			cout << "Data inserted successfully!" << endl;
		}
		else {
			cout << "Failed to insert data." << endl;
		}
	}

	void printTicket() {
		cout << "TICKETS:" << endl;
		vector<map<string, string>> bookings = db->getBookingDetails(REAL_USERNAME);
		if (!bookings.empty()) {
			for (size_t i = 0; i < bookings.size(); ++i) {
				cout << "=============================================" << endl;
				cout << "           Booking Details " << i + 1 << endl;
				cout << "=============================================" << endl;
				const map<string, string>& bookingData = bookings[i];
				for (const auto& entry : bookingData) {
					cout << entry.first << ": " << entry.second << endl;
				}
				cout << endl;
			}
		}
		else {
			cout << "No bookings found for username: " << REAL_USERNAME << endl;
		}
	}

	bool bookTickets() {
		MYSQL* conn;
		conn = mysql_init(0);

		conn = mysql_real_connect(conn, "localhost", "root", "", "booking", 3306, NULL, 0);
		string query = "SELECT sno, theater_name, place, landmark FROM theaters WHERE landmark = '" + landmark + "'";

		// Execute the query
		if (mysql_query(conn, query.c_str()) == 0) {
			MYSQL_RES* res = mysql_store_result(conn);
			if (res) {
				// Display the theaters and let the user choose a hall
				cout << "Theaters near your landmark:" << endl;
				cout << "+------+-------------------+--------------------------------------+----------+" << endl;
				cout << "| " << setw(4) << "sno" << " | " << setw(17) << "theater_name" << " | " << setw(36) << "place" << " | " << setw(8) << "landmark" << " |" << endl;
				cout << "+------+-------------------+--------------------------------------+----------+" << endl;

				// Process the result set
				MYSQL_ROW row;
				vector<string> halls;
				int count = 1;
				while ((row = mysql_fetch_row(res))) {
					cout << "| " << setw(4) << count << " | " << setw(17) << row[1] << " | " << setw(36) << row[2] << " | " << setw(8) << row[3] << " |" << endl;
					halls.push_back(row[1]); // Store theater names in a vector
					count++;
				}
				cout << "+------+-------------------+--------------------------------------+----------+" << endl << endl;

				// Let the user choose a hall
				int hallChoice;
			hallchoice:
				cout << "Enter the number corresponding to the hall you want to choose: ";
				cin >> hallChoice;
				cin.ignore(); // Ignore newline character

				// Validate the user's choice
				if (hallChoice < 1 || hallChoice > halls.size()) {
					cout << "Invalid hall choice!" << endl;
					goto hallchoice;
				}

				// Store the chosen hall
				hall = halls[hallChoice - 1];

				// Free the result set
				mysql_free_result(res);
			}
		}
		else {
			// Error handling if query execution fails
			cout << "Error: " << mysql_error(conn) << endl;
			return false;
		}

		int movie_id = display(selectmovie());
		cout << endl;
		int n;
		cout << "ENTER THE LANGUAGE YOU WANT TO WATCH THE MOVIE IN:" << endl;
		cout << "1. ENGLISH" << endl;
		cout << "2. HINDI" << endl;
		cout << "3. PUNJABI" << endl;
		cout << "4. TELUGU" << endl;
		do {
			cin.ignore();
			cout << "ENTER CHOICE:";
			cin >> n;
			if (cin.fail()) {
				cout << "Please enter a valid number." << endl;
				cin.clear();               // Clear error flags
				cin.ignore(); // Ignore all characters currently in the input buffer// Discard invalid input
				continue;
			}
			switch (n)
			{
			case 1:
				language = "English";
				break;
			case 2:
				language = "Hindi";
				break;
			case 3:
				language = "Punjabi";
				break;
			case 4:
				language = "Telugu";
				break;
			default:
				cout << "ENTER A VALID OPTION!" << endl;
				continue;
			}
			break;
		} while (true);

		//cin.ignore();
		vector<int>time_vec = { 172,70,94,140,68,99,131,97,118,116,124,177,128,67,74,146,127,71,162,80 };
		vector<int>price_vec = { 429,313,293,352,444,473,475,265,465,436,497,415,408,426,218,325,392,426,268,362 };

		price = to_string(price_vec[movie_id - 1]);//db->get("movies", "price", "movie_name='" + film + "'");
		int price_to_add = price_vec[movie_id - 1];
		time = to_string(time_vec[movie_id - 1]);//db->get("movies", "length", "movie_name='" + film + "'");
		int minutesToAdd = time_vec[movie_id - 1];
		// Parse startTime to extract hours and minutes
		int hours, minutes;
		char colon;
		istringstream startTimeStream(startTime);
		startTimeStream >> hours >> colon >> minutes;

		// Add minutes from endTime to startTime
		minutes += minutesToAdd;
		hours += minutes / 60; // Adjust hours if minutes exceed 60
		minutes %= 60; // Adjust minutes to be within 0-59 range

		// Format the result back to HH:MM format
		ostringstream resultStream;
		resultStream << (hours < 10 ? "0" : "") << hours << ":" << (minutes < 10 ? "0" : "") << minutes;
		endTime = resultStream.str();
		cout << "PRICE TO PAY IS: " << price_to_add * quantity << endl;
		delay();
		cout << "PROCESSING PAYMENT......" << endl << endl;
		budget = budget - price_to_add;
		budget1:
		if (budget < 0) {
			cout << "Your budget is insufficient to book a ticket." << endl;
			cout << "Do you want to increase your budget?" << endl;
			cout << "1. Yes" << endl << "2. No" << endl;
			int increaseBudgetChoice;
			cin >> increaseBudgetChoice;
			if (increaseBudgetChoice == 1)
			{
				cout << "Enter your new budget: ";
				cin >> budget;
				if (cin.fail()) {
					cout << "Please enter a valid number." << endl;
					cin.clear();               // Clear error flags
					cin.ignore(); // Ignore all characters currently in the input buffer// Discard invalid input
					goto budget1;
				}
				budget = budget - price_to_add;
				delay();
				cout << "THANKS FOR THE PAYMENT!" << endl;

				cout << "YOUR TICKET IS SAVED IN MY ORDERS TAB!" << endl << endl;;
				cout << "EXITING MOVIE BOOKING.....";
				delay();
				clear();
				return true;
			}
			else if (increaseBudgetChoice == 2)
			{
				cout << "Returning to the main menu." << endl;
				return false;
			}
			else
			{
				cout << "Invalid choice. Returning to the main menu." << endl;
				delay();
				clear();
				return false;
			}
		}cout << "PAYMENT SUCCESSFUL....." << endl;
		cout << "CHECK YOUR TICKET IN THE MY BOOKINGS TAB.." << endl;

	}


	void getDetails()
	{
		cout << "\n*************************************************************" << endl;
		string currentDate = getCurrentTime().substr(0, 10);
		cout << "Today's date is: " << currentDate << endl;
		do
		{
			cout << "Please select the date for your outing (up to 7 days from now, in format YYYY-MM-DD):- " << endl;
			cin >> date;
			if (cin.fail()) {
				cout << "Please enter a valid date." << endl;
				cin.clear();               // Clear error flags
				cin.ignore(); // Ignore all characters currently in the input buffer// Discard invalid input
				continue;
			}
		} while (!isValidDate(date, currentDate, 7));
		cout << endl;
		do
		{
			cout << "Please select the starting time for your outing (between 10 am and 5 pm,in 24 hour format(HH:MM):- " << endl;;
			cin >> startTime;
			if (cin.fail()) {
				cout << "Please enter a valid number." << endl;
				cin.clear();               // Clear error flags
				cin.ignore(); // Ignore all characters currently in the input buffer// Discard invalid input
				continue;
			}
		} while (!isValidTime(startTime));
		cout << endl;
		do
		{
			cout << "Please select how many hours can you spare for the experience: ";
			cin >> hours;
			if (cin.fail()) {
				cout << "Please enter a valid number." << endl;
				cin.clear();               // Clear error flags
				cin.ignore(); // Ignore all characters currently in the input buffer// Discard invalid input
				continue;
			}
		} while (!(hours >= 1 && hours <= 24));
		cout << endl;

		while (true)
		{
			//change budget add cases for movies and tv series where if a person wants to watch at home, he can set 0 budget and also if budget gets over update it before booking
			cout << "Enter your budget for the outing:-" << endl;
			cin >> budget;
			if (cin.fail()) {
				cout << "Please enter a valid number." << endl;
				cin.clear();               // Clear error flags
				cin.ignore(); // Ignore all characters currently in the input buffer// Discard invalid input
				continue;
			}

			if (budget < 500 && budget >= 0)
			{
				cout << "Insufficient Budget... \n\tmaybe you can try our recommending system to watch a movie or tv series at home :))." << endl;
				break;
			}
			else if (budget < 0)
			{
				cout << "Enter a valid amount!" << endl;
				break;
			}
			else {
				break;
			}
		}
		cout << endl;
		cout << "ENTER STATE:";
		cin.ignore();
		getline(cin, state);
		cout << "ENTER CITY:";
		getline(cin, city);
		cout << endl;
		do {
			cout << "CHOOSE THE CLOSEST PLACE TO YOUR LOCATION " << endl
				<< "A -> VASANT KUNJ" << endl
				<< "B -> GURGAON" << endl
				<< "C -> NOIDA" << endl
				<< "D -> TAGORE GARDEN" << endl
				<< "E -> INDIRAPURAM" << endl
				<< "F -> KAUSHAMBI" << endl
				<< "G -> KHEL GAON MARG" << endl
				<< "H -> GREATER NOIDA" << endl
				<< "ENTER THE CHARACTER: ";
				cin >> landmark;
				if (cin.fail()) {
					cout << "Please enter a valid option." << endl;
					cin.clear();               // Clear error flags
					cin.ignore(); // Ignore all characters currently in the input buffer// Discard invalid input
					continue;
				}
		} while (!(landmark >= "A" && landmark <= "H"));

		delay();
		clear();

	}

	bool isValidDate(const string& dateStr, const string& currentDateStr, int nDays)
	{
		istringstream dateStream(dateStr);
		istringstream currentStream(currentDateStr);

		int year, month, day;
		char dash;

		dateStream >> year >> dash >> month >> dash >> day;
		int currentYear, currentMonth, currentDay;
		currentStream >> currentYear >> dash >> currentMonth >> dash >> currentDay;

		tm selectedDate = { 0, 0, 0, day, month - 1, year - 1900 };
		tm currentDate = { 0, 0, 0, currentDay, currentMonth - 1, currentYear - 1900 };

		time_t selectedTime = mktime(&selectedDate);
		time_t currentTime = mktime(&currentDate);

		double difference = difftime(selectedTime, currentTime);

		bool condition = difference >= 0 && difference <= nDays * 24 * 60 * 60;

		if (!condition)
		{
			cout << "ENTER A VALID DATE" << endl;
			return condition;
		}

		return condition;
	}

	bool isValidTime(const string& time)
	{
		stringstream ss(time);
		int hour, minute;
		char colon;
		ss >> hour >> colon >> minute;

		bool condition = (hour >= 10 && hour <= 16) && (minute >= 0 || minute <= 60) && (colon == ':');

		if (!condition)
		{
			cout << "ENTER VALID TIME!!" << endl;
			return condition;
		}

		return condition;
	}

	int display(int movie_id) {
		// Retrieve movie name from database
		film = db->get("movies", "movie_name", "sno='" + to_string(movie_id) + "'");
		/*****************************************WE NEED MOVIE FILES LIKE DANGAL.TXT IN THE PROJECT TO MOVE FURTHER***************************************/

		// Generate filename from movie name
		string filename = "D:\\Project sem 2\\Movie_seats\\" + film + ".txt";

		// Check if the file already exists
		ifstream file(filename);
		vector<vector<char>> seats_array(12, vector<char>(12));
		if (file) {
			// Read movie name from the first line (header)
			getline(file, film);

			// Read the seating arrangement from the file
			string line;
			int row = 0;
			while (getline(file, line)) {
				if (row >= 12) {
					break; // Exit the loop
				}
				for (int col = 0; col < 12; ++col) {
					char occupancy_char = line[col * 2]; // The occupancy character is at every third position starting from index 1
					if (occupancy_char == 'x') {
						seats_array[row][col] = 'x';
					}
					else {
						seats_array[row][col] = ' ';
					}
				}
				++row;

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
			ofstream outfile(filename, ofstream::trunc);
			if (outfile) {
				// Write movie name as header
				outfile << "Movie: " << film << endl;

				// Write array to file
				for (const auto& row : seats_array) {
					for (char seat : row) {
						outfile << seat;
					}
					outfile << endl;
				}
				outfile.close();
			}
			else {
				cerr << "Error: Could not create file " << filename << endl;
			}
		}

		// Display the seating arrangement
		cout << "Seating arrangement for " << film << ":" << endl;
		cout << "---------------------Screen--------------------------" << endl;
		cout << "     ";
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
		seatchoice:
		try {
			cout << "\nEnter number of seats you want to choose:";
			cin.ignore();
			cin >> quantity;
			if (cin.fail()) {
				cout << "Please enter a valid number." << endl;
				cin.clear();               // Clear error flags
				cin.ignore(); // Ignore all characters currently in the input buffer// Discard invalid input
				goto seatchoice;
			}
			char c;
			int n;
			string seat;
			int i = 0;
			int num_of_seats_available = 0;
			for (int x = 0; x < 12; x++) {
				for (int y = 0; y < 12; y++) {
					if (seats_array[x][y] == ' ') {
						num_of_seats_available++;
					}

				}
			}if (quantity > num_of_seats_available) {
				throw invalid_argument("Number of seats picked is more than available!!");
			}


			for (; i < quantity;) {
				cout << "\nEnter seats(in format like A6, B7, D10 etc):" << endl;;
				cout << "SEAT " << i + 1 << ":";
				cin.ignore();
				cin >> seat;
				istringstream mystream(seat);
				mystream >> c >> n;
				c = (char)tolower(c);
				if (c >= 'a' && c <= 'l' && n >= 1 && n <= 12) {
					if (seats_array[n - 1][c - 'a'] == ' ') {
						seats.push_back(seat);
						i++;
						//update seat matrix
						seats_array[n - 1][c - 97] = 'x';
					}
					else {
						cout << "Already occupied!!" << endl;
					}

				}
				else {
					cout << "Invalid seat" << endl;
				}
			}
			return movie_id;
		}
		catch (const invalid_argument& e) {
			cout << e.what() << endl;
			cout << "EXITING......" << endl;
			exit(0);
		}
		catch (...) {
			cout << "INVALID INPUT!";
			goto seatchoice;
		}
		return 0;
	}

	int selectmovie()
	{
		vector<string> movieNames = db->getall("movies", "movie_name");
		int numMovies = movieNames.size();

		cout << "Available Movies:" << endl;
		for (int i = 0; i < numMovies; ++i)
		{
			cout << (i + 1) << ". " << movieNames[i] << endl;
		}

		int selectedMovieId;
		cout << "Enter the movie ID: ";
		cin >> selectedMovieId;

		while (selectedMovieId < 1 || selectedMovieId > numMovies)
		{
			cout << "Invalid movie ID. Please enter a valid ID: ";
			cin >> selectedMovieId;
		}

		return selectedMovieId;
	}


private:
	unique_ptr<Database> db;
};



// Base class for media items
class MediaItem
{
protected:
	int code;
	string title;
	string genre;
	string subgenre;
	string streamingPlatform;
	string language;
	double imdbRating;

public:
	//Constructor
	MediaItem(int _code, string _title, string _genre, string _subgenre, string _streamingPlatform, string _language, double _imdbRating)
		: code(_code), title(_title), genre(_genre), subgenre(_subgenre), streamingPlatform(_streamingPlatform), language(_language), imdbRating(_imdbRating) {};

	// Pure virtual functions
	//1. Display all details
	virtual void displayDetails() const = 0;
	//2. Determine whether media item is movie or tv series
	virtual string getType() const = 0;

	string get_t()
	{
		return title;
	}
	string get_g()
	{
		return genre;
	}
	string get_s()
	{
		return subgenre;
	}
	void getTitle()
	{
		cout << title << endl;
	}
	void getGenre()
	{
		cout << genre << endl;
	}
	void getSubgenre()
	{
		cout << subgenre << endl;
	}
	double getimdbRating()
	{
		return imdbRating;
	}
	string getLanguage()
	{
		return language;
	}
	string getStreamingPlatform()
	{
		return streamingPlatform;
	}
};

//Inherited class for movies
class Movie : public MediaItem
{
private:
	string type;
	vector<string> cast;
	string releaseDate;
	int duration; // in minutes
	string synopsis;
	string review;

public:
	//Constructor
	Movie(int _code, string _title, string _genre, string _subgenre, string _streamingPlatform, string _language, double _imdbRating,
		string _type, vector<string> _cast, string _releaseDate, int _duration, string _synopsis, string _review)
		: MediaItem(_code, _title, _genre, _subgenre, _streamingPlatform, _language, _imdbRating),
		type(_type), cast(_cast), releaseDate(_releaseDate), duration(_duration), synopsis(_synopsis), review(_review) {};

	void displayDetails() const override
	{
		cout << endl;
		cout << "************************************************************";
		cout << endl;
		cout << "Title: " << title << endl;
		cout << "Type: " << type << endl;
		cout << "Genre: " << genre << endl;
		cout << "Subgenre: " << subgenre << endl;
		cout << "Language: " << language << endl;
		cout << "IMDB Rating: " << imdbRating << endl;
		cout << "Streaming Platform: " << streamingPlatform << endl;
		cout << "Cast: ";
		for (const auto& actor : cast)
		{
			cout << actor << ", ";
		}
		cout << endl;
		cout << "Release Date: " << releaseDate << endl;
		cout << "Duration: " << duration << " minutes" << endl;
		cout << "Synopsis: " << synopsis << endl;
		cout << "Review: " << review << endl;
		cout << endl;
		cout << "************************************************************";
		cout << endl;
	}

	string getType() const override
	{
		return "Movie";
	}

	int getDuration()
	{
		return duration;
	}
};

//Inherited class for tv series
class TVSeries : public MediaItem
{
private:
	string type;
	vector<string> cast;
	string releaseDate;
	int numEpisodes;
	string synopsis;
	string review;

public:
	//Constructor
	TVSeries(int _code, string _title, string _genre, string _subgenre, string _streamingPlatform, string _language, double _imdbRating,
		string _type, vector<string> _cast, string _releaseDate, int _numEpisodes, string _synopsis, string _review)
		: MediaItem(_code, _title, _genre, _subgenre, _streamingPlatform, _language, _imdbRating),
		type(_type), cast(_cast), releaseDate(_releaseDate), numEpisodes(_numEpisodes), synopsis(_synopsis), review(_review) {};

	void displayDetails() const override
	{
		cout << endl;
		cout << "************************************************************";
		cout << endl;
		cout << "Title: " << title << endl;
		cout << "Type: " << type << endl;
		cout << "Genre: " << genre << endl;
		cout << "Subgenre: " << subgenre << endl;
		cout << "Language: " << language << endl;
		cout << "IMDB Rating: " << imdbRating << endl;
		cout << "Streaming Platform: " << streamingPlatform << endl;
		cout << "Cast: ";
		for (const auto& actor : cast)
		{
			cout << actor << ", ";
		}
		cout << endl;
		cout << "Release Date: " << releaseDate << endl;
		cout << "Number of Episodes: " << numEpisodes << endl;
		cout << "Synopsis: " << synopsis << endl;
		cout << "Review: " << review << endl;
		cout << endl;
		cout << "************************************************************";
		cout << endl;
	}

	string getType() const override
	{
		return "TV Series";
	}
};


// Class for managing recommendations
class Recommender
{
public:

	// 1. Function to display details of all media items
	void displayAllItems(vector <Movie>& movieItems, vector <TVSeries>& tvseriesItems)
	{
		cout << "List of titles of all Media Items:" << endl << endl;
		for (int i = 0; i < movieItems.size(); i++)
		{
			cout << "MOVIE " << i + 1 << ": " << endl;
			movieItems.at(i).getTitle();
			cout << endl;
		}
		cout << endl;
		for (int i = 0; i < tvseriesItems.size(); i++)
		{
			cout << "TV SERIES " << i + 1 << ": " << endl;
			tvseriesItems.at(i).getTitle();
			cout << endl;
		}
		cout << endl;
	r:
		cout << "Do you wish to see the details of the selected Media Items?" << endl;
		cout << "1. Display Details" << endl << "2. Return to Recommendation Menu" << endl;
		int ch1;
	choice:
		cout << "Enter your choice: ";
		cin >> ch1;
		cout << endl;
		if (ch1 == 1)
		{
			cout << "Enter the name of the movie/tv series you wish to explore more: ";
			string media_item;
			cin.ignore();
			int count = 0;
			getline(cin, media_item);
			transform(media_item.begin(), media_item.end(), media_item.begin(), toupper);
			cout << endl;
			for (int i = 0; i < movieItems.size(); i++)
			{
				if (media_item == movieItems.at(i).get_t())
				{
					movieItems.at(i).displayDetails();
					cout << endl;
					count++;
				}
			}
			for (int i = 0; i < tvseriesItems.size(); i++)
			{
				if (media_item == tvseriesItems.at(i).get_t())
				{
					tvseriesItems.at(i).displayDetails();
					cout << endl;
					count++;
				}
			}
			if (count == 0)
			{
				cout << "*****************************************************************";
				cout << endl;
				cout << "No Media Item found with title \"" << media_item << "\"" << endl;
				cout << "*****************************************************************";
				cout << endl;
			}
		q:
			cout << endl;
			cout << "Do you wish to view the details of any other movie/tv series? (Yes/ No) ";
			string yn2;
			cin >> yn2;
			int count1 = 0;
			cout << endl;
			transform(yn2.begin(), yn2.end(), yn2.begin(), toupper);
			cout << endl;
			if (yn2 == "YES")
			{
				cout << "Enter the name of the movie you wish to explore more: ";
				string movie_t1;
				cin.ignore();
				getline(cin, movie_t1);
				transform(movie_t1.begin(), movie_t1.end(), movie_t1.begin(), toupper);
				cout << endl;
				for (int i = 0; i < movieItems.size(); i++)
				{
					if (movie_t1 == movieItems.at(i).get_t())
					{
						movieItems.at(i).displayDetails();
						cout << endl;
						count1++;
					}
				}
				for (int i = 0; i < tvseriesItems.size(); i++)
				{
					if (movie_t1 == tvseriesItems.at(i).get_t())
					{
						tvseriesItems.at(i).displayDetails();
						cout << endl;
						count1++;
					}
				}
				if (count1 == 0)
				{
					cout << "*****************************************************************";
					cout << endl;
					cout << "No Media Item found with title \"" << movie_t1 << "\"" << endl;
					cout << "*****************************************************************";
					cout << endl;
				}
				goto q;
			}
			else
			{
				return;
			}
		}
		else if (ch1 == 2)
		{
			return;
		}
		else
		{
			cout << "INVALID INPUT!!!" << endl;
			cout << "Please enter either '1' or '2'" << endl;
			goto r;
		}
	}

	// 2. Function to display all movies or tv series
	void displayMoviesorSeries(vector <Movie>& movieItems, vector <TVSeries>& tvseriesItems)
	{
	u:
		cout << endl << "Do you wish to watch a MOVIE or a TV SERIES?" << endl;
		int ch2;
		cout << "1. MOVIE" << endl << "2. TV SERIES" << endl;
		cout << "Enter Your choice: ";
		cin >> ch2;
		cout << endl;
		if (ch2 == 1)
		{
			for (int i = 0; i < movieItems.size(); i++)
			{
				cout << "Recommendation " << i + 1 << ": " << endl;
				movieItems.at(i).getTitle();
				cout << endl;
			}
		}
		else if (ch2 == 2)
		{
			for (int i = 0; i < tvseriesItems.size(); i++)
			{
				cout << "Recommendation " << i + 1 << ": " << endl;
				tvseriesItems.at(i).getTitle();
				cout << endl;
			}
		}
		else
		{
			cout << "INVALID INPUT!!!" << endl;
			cout << "Please enter either '1' or '2'" << endl;
			goto u;
		}
	t:
		cout << "Do you wish to see the details of the selected Media Items?" << endl;
		cout << "1. Display Details" << endl << "2. Return to Recommendation Menu" << endl;
		int ch3;
		cout << "Enter your choice: ";
		cin >> ch3;
		cout << endl;
		if (ch3 == 1)
		{
			if (ch2 == 1)
			{
				cout << "Enter the name of the movie you wish to explore more: ";
				string movie_t;
				cin.ignore();
				int count = 0;
				getline(cin, movie_t);
				transform(movie_t.begin(), movie_t.end(), movie_t.begin(), toupper);
				cout << endl;
				for (int i = 0; i < movieItems.size(); i++)
				{
					if (movie_t == movieItems.at(i).get_t())
					{
						movieItems.at(i).displayDetails();
						cout << endl;
						count++;
					}
				}
				if (count == 0)
				{
					cout << "*****************************************************************";
					cout << endl;
					cout << "No Media Item found with title \"" << movie_t << "\"" << endl;
					cout << "*****************************************************************";
					cout << endl;
				}
			q:
				cout << "Do you wish to view the details of any other movie? (Yes/ No) ";
				string yn2;
				cin >> yn2;
				int count2 = 0;
				transform(yn2.begin(), yn2.end(), yn2.begin(), toupper);
				cout << endl;
				if (yn2 == "YES")
				{
					cout << "Enter the name of the movie you wish to explore more: ";
					string movie_t1;
					cin.ignore();
					getline(cin, movie_t1);
					cout << endl;
					transform(movie_t1.begin(), movie_t1.end(), movie_t1.begin(), toupper);
					for (int i = 0; i < movieItems.size(); i++)
					{
						if (movie_t1 == movieItems.at(i).get_t())
						{
							movieItems.at(i).displayDetails();
							cout << endl;
						}
					}
					if (count2 == 0)
					{
						cout << "*****************************************************************";
						cout << endl;
						cout << "No Media Item found with title \"" << movie_t1 << "\"" << endl;
						cout << "*****************************************************************";
						cout << endl;
					}
					goto q;
				}
			}
			if (ch2 == 2)
			{
				cout << "Enter the name of the tv series you wish to explore more: ";
				string tv_t;
				cin.ignore();
				int count = 0;
				getline(cin, tv_t);
				transform(tv_t.begin(), tv_t.end(), tv_t.begin(), toupper);
				cout << endl;
				for (int i = 0; i < tvseriesItems.size(); i++)
				{
					if (tv_t == tvseriesItems.at(i).get_t())
					{
						tvseriesItems.at(i).displayDetails();
						cout << endl;
						count++;
					}
				}
				if (count == 0)
				{
					cout << "*****************************************************************";
					cout << endl;
					cout << "No Media Item found with title \"" << tv_t << "\"" << endl;
					cout << "*****************************************************************";
					cout << endl;
				}
			s:
				cout << "Do you wish to view the details of any other tv series? (Yes/ No) ";
				string yn2;
				cin >> yn2;
				transform(yn2.begin(), yn2.end(), yn2.begin(), toupper);
				cout << endl;
				if (yn2 == "YES")
				{
					cout << "Enter the name of the tv series you wish to explore more: ";
					string tv_t1;
					cin.ignore();
					int count2 = 0;
					getline(cin, tv_t1);
					transform(tv_t1.begin(), tv_t1.end(), tv_t1.begin(), toupper);
					cout << endl;
					for (int i = 0; i < tvseriesItems.size(); i++)
					{
						if (tv_t1 == tvseriesItems.at(i).get_t())
						{
							tvseriesItems.at(i).displayDetails();
							cout << endl;
						}
					}
					if (count2 == 0)
					{
						cout << "*****************************************************************";
						cout << endl;
						cout << "No Media Item found with title \"" << tv_t1 << "\"" << endl;
						cout << "*****************************************************************";
						cout << endl;
					}
					goto s;
				}
				else
				{
					return;
				}
			}
		}
		else if (ch3 == 2)
		{
			return;
		}
		else
		{
			cout << "INVALID INPUT!!!" << endl;
			cout << "Please enter either '1' or '2'" << endl;
			goto t;
		}
	}

	// 4. Function to search for a specific media item by title
	void searchByTitle(vector <Movie>& movieItems, vector <TVSeries>& tvseriesItems)
	{
		string t;
		cout << "Enter the name of the Media Item you want to search: ";
		cin.ignore();
		getline(cin, t);
		transform(t.begin(), t.end(), t.begin(), toupper);
		int count = 0;
		cout << endl;
		for (auto& item : movieItems)
		{
			if (item.get_t() == t)
			{
				item.displayDetails();
				count++;
			}
		}
		for (auto& item : tvseriesItems)
		{
			if (item.get_t() == t)
			{
				item.displayDetails();
				count++;
			}
		}
		if (count == 1)
		{
			return;
		}
		else
		{
			cout << "*****************************************************************";
			cout << endl;
			cout << "No Media Item found with title \"" << t << "\"" << endl;
			cout << "*****************************************************************";
			cout << endl;
		}
	}
	// 9. Function to make wishlist
	void wishlist(vector <Movie>& movieItems, vector <TVSeries>& tvseriesItems)
	{
		//FIXING
		vector <string> wishlist;
		cout << "MY WISHLIST" << endl;
		cout << "1. Add a new Media Item" << endl;
		cout << "2. Delete a Media Item" << endl;
		cout << "3. Display names of all Media Items present" << endl;
		cout << "4. Display the details of all the Media Items present" << endl;
		int ch5;
		cout << "Enter your Choice: ";
		cin >> ch5;
		string title;
		if (ch5 == 1)
		{
			cout << "Enter the name of the Media Item to be added: ";
			cin.ignore();
			getline(cin, title);
			transform(title.begin(), title.end(), title.begin(), toupper);
			// Check if the title already exists in the wishlist
			if (find(wishlist.begin(), wishlist.end(), title) != wishlist.end()) {
				cout << "Item \"" << title << "\" is already in the wishlist." << endl;
			}
			else {
				wishlist.push_back(title);
				{
					ofstream outputFile("cinebliss_wishlist.txt", ios::app);
					if (!outputFile.is_open()) {
						cerr << "Error: Unable to open file for writing." << endl;
						return;
					}
					outputFile << title << endl;
					cout << "Added \"" << title << "\" to wishlist." << endl;
				}
			}
		}
		else if (ch5 == 2)
		{
			cin.ignore();
			cout << "Enter the name of the Media Item to be deleted: ";
			getline(cin, title);
			transform(title.begin(), title.end(), title.begin(), toupper);
			{
				ifstream inputFile("cinebliss_wishlist.txt");
				ofstream outputFile("temp_wishlist.txt");
				if (!inputFile.is_open() || !outputFile.is_open()) {
					cerr << "Error: Unable to open files." << endl;
					return;
				}
				string line;
				bool found = false;
				while (getline(inputFile, line)) {
					if (line != title) {
						outputFile << line << endl;
					}
					else {
						found = true;
					}
				}
				inputFile.close();
				outputFile.close();
				if (found) {
					remove("cinebliss_wishlist.txt");
					rename("temp_wishlist.txt", "cinebliss_wishlist.txt");
					cout << "Removed \"" << title << "\" from wishlist." << endl;
				}
				else {
					cout << "Item \"" << title << "\" not found in wishlist." << endl;
				}
			}
		}
		else if (ch5 == 3)
		{
			ifstream inputFile("cinebliss_wishlist.txt");
			if (!inputFile.is_open()) {
				cerr << "Error: Unable to open file for reading." << endl;
				return;
			}
			string line;
			while (getline(inputFile, line)) {
				wishlist.push_back(line);
			}
			inputFile.close();

			cout << "Names of all Media Items present in the wishlist:" << endl;
			for (const auto& item : wishlist) {
				cout << item << endl;
			}
		}
		else if (ch5 == 4)
		{
			ifstream inputFile("cinebliss_wishlist.txt");
			int count = 0;
			if (!inputFile.is_open()) {
				cerr << "Error: Unable to open file for reading." << endl;
				return;
			}
			string line;
			while (getline(inputFile, line)) {
				wishlist.push_back(line);
			}
			inputFile.close();

			cout << "Details of Wishlist Items:" << endl;
			for (const auto& item : wishlist)
			{
				for (int i = 0; i < movieItems.size(); i++)
				{
					if (item == movieItems.at(i).get_t())
					{
						movieItems.at(i).displayDetails();
						count++;
					}
				}
			}for (const auto& item : wishlist)
			{
				for (int i = 0; i < tvseriesItems.size(); i++)
				{
					if (item == tvseriesItems.at(i).get_t())
					{
						tvseriesItems.at(i).displayDetails();
						count++;
					}
				}
			}if (count == 0) {
				cout << "NO SUCH MOVIE IN OUR DATABASE!!" << endl;;

			}
		}
	}

	//3. Function to filter the media items
	void filter(vector <Movie>& movieItems, vector <TVSeries>& tvseriesItems)
	{
		cout << endl;
		cout << "*************************************";
		cout << endl;
		cout << "FILTER MENU" << endl;
		cout << "1. By Genre" << endl;
		cout << "2. By Streaming Platform" << endl;
		cout << "3. By Language" << endl;
		cout << "4. By IMDb Rating" << endl;
		cout << endl;
		cout << "*************************************";
		cout << endl;
		int ch6;
		cout << endl;
		cout << "Enter your choice: ";
		cin >> ch6;
		cout << endl;
		if (ch6 == 1)
		{
			cout << endl;
			cout << "*************************************";
			cout << endl;
			cout << "GENRE MENU" << endl;
			cout << "1. Action" << endl;
			cout << "2. Comedy" << endl;
			cout << "3. Drama" << endl;
			cout << "4. Romance" << endl;
			cout << "5. Thriller" << endl;
			cout << "6. Horror" << endl;
			cout << "7. Historical" << endl;
			cout << endl;
			cout << "*************************************";
			cout << endl;
			string g, s;
			cout << "Enter your choice for Genre: ";
			cin >> g;
			transform(g.begin(), g.end(), g.begin(), toupper);
			cout << endl;
			cout << "Enter your choice for Subgenre: ";
			cin >> s;
			transform(s.begin(), s.end(), s.begin(), toupper);
			cout << endl;
			cout << endl;
			cout << "*************************************";
			cout << endl;
			cout << "MOVIES:" << endl << endl;
			for (auto& movie : movieItems)
			{
				if (movie.get_g() == g && movie.get_s() == s)
				{
					movie.displayDetails();
					cout << endl;
				}
			}
			cout << endl;
			cout << "*************************************";
			cout << endl;
			cout << endl;
			cout << "*************************************";
			cout << endl;
			cout << "TV SERIES:" << endl << endl;
			for (auto& tv : tvseriesItems)
			{
				if (tv.get_g() == g && tv.get_s() == s)
				{
					tv.displayDetails();
					cout << endl;
				}
			}
			cout << endl;
			cout << "*************************************";
			cout << endl;

		}
		else if (ch6 == 2)
		{
			cout << endl;
			cout << endl;
			cout << "*************************************";
			cout << endl;
			cout << "STREAMING PLATFORM MENU" << endl;
			cout << "1. Netflix" << endl;
			cout << "2. Amazon Prime" << endl;
			cout << "3. Disney+ Hotstar" << endl;
			cout << "4. SonyLiv" << endl;
			cout << "5. ZEE5" << endl << endl;
			cout << endl;
			cout << "*************************************";
			cout << endl;
			string sp;
			cout << "Enter your choice for Streaming Platform: ";
			cin.ignore();
			getline(cin, sp);
			transform(sp.begin(), sp.end(), sp.begin(), toupper);
			cout << endl;
			cout << endl;
			cout << "*************************************";
			cout << endl;
			cout << "MOVIES:" << endl << endl;
			for (auto& movie : movieItems)
			{
				if (movie.getStreamingPlatform() == sp)
				{
					movie.getTitle();
					cout << endl;
				}
			}
			cout << endl;
			cout << "*************************************";
			cout << endl;
			cout << endl;
			cout << "*************************************";
			cout << endl;

			cout << "TV SERIES:" << endl << endl;
			for (auto& tv : tvseriesItems)
			{
				if (tv.getStreamingPlatform() == sp)
				{
					tv.getTitle();
					cout << endl;
				}
			}
			cout << endl;
			cout << "*************************************";
			cout << endl;
		}
		else if (ch6 == 3)
		{
			cout << endl;
			cout << endl;
			cout << "*************************************";
			cout << endl;
			cout << "LANGUAGE MENU" << endl;
			cout << "1. English" << endl;
			cout << "2. Hindi" << endl;
			cout << "3. Korean" << endl;
			cout << "4. Chinese" << endl << endl;
			cout << endl;
			cout << "*************************************";
			cout << endl;
			string l;
			cout << "Enter your choice for Language: ";
			cin >> l;
			transform(l.begin(), l.end(), l.begin(), toupper);
			cout << endl;
			cout << endl;
			cout << "*************************************";
			cout << endl;
			cout << "MOVIES:" << endl << endl;
			for (auto& movie : movieItems)
			{
				if (movie.getLanguage() == l)
				{
					movie.getTitle();
					cout << endl;
				}
			}
			cout << endl;
			cout << "*************************************";
			cout << endl;
			cout << endl;
			cout << "*************************************";
			cout << endl;
			cout << "TV SERIES:" << endl << endl;
			for (auto& tv : tvseriesItems)
			{
				if (tv.getLanguage() == l)
				{
					tv.getTitle();
					cout << endl;
				}
			}
			cout << endl;
			cout << "*************************************";
			cout << endl;
		}
		else if (ch6 == 4)
		{
			cout << endl;
			cout << endl;
			cout << "*************************************";
			cout << endl;
			cout << "IMDB RATING MENU" << endl;
			cout << "1. Greater than 9" << endl;
			cout << "2. Greater than 8" << endl;
			cout << "3. Greater than 7" << endl;
			cout << "4. Greater than 6" << endl;
			cout << "5. Greater than 5" << endl;
			cout << endl;
			cout << "*************************************";
			cout << endl;
			double i;
			cout << "Enter your choice for IMDb Rating (9, 8, 7, 6, 5): ";
			cin >> i;
			cout << endl;
			cout << endl;
			cout << "*************************************";
			cout << endl;
			cout << "MOVIES:" << endl << endl;
			for (auto& movie : movieItems)
			{
				if (movie.getimdbRating() >= i)
				{
					movie.getTitle();
					cout << endl;
				}
			}
			cout << endl;
			cout << "*************************************";
			cout << endl;
			cout << endl;
			cout << "*************************************";
			cout << endl;
			cout << "TV SERIES:" << endl << endl;
			for (auto& tv : tvseriesItems)
			{
				if (tv.getimdbRating() >= i)
				{
					tv.getTitle();
					cout << endl;
				}
			}
			cout << endl;
			cout << "*************************************";
			cout << endl;
		}
	}

	// 6. Function to recommend a random movie
	void recommendRandom(vector <Movie>& movieItems, vector <TVSeries>& tvseriesItems)
	{
		cout << endl;
		cout << "************************************************************";
		cout << endl;
		cout << "MOVIE OF THE DAY: " << endl << endl;
		if (movieItems.empty())
		{
			cout << "No media items available for recommendation." << endl;
			return;
		}

		random_device rd;
		mt19937 gen(rd());
		uniform_int_distribution<int> dist(0, movieItems.size() - 1);

		int randomIndex = dist(gen);
		movieItems[randomIndex].displayDetails();
	}

	// 7. Function to recommend a random tv series
	void recommendRandomtv(vector <Movie>& movieItems, vector <TVSeries>& tvseriesItems)
	{
		cout << endl;
		cout << "************************************************************";
		cout << endl;
		cout << "TV SERIES OF THE DAY: " << endl;
		if (tvseriesItems.empty())
		{
			cout << "No media items available for recommendation." << endl;
			return;
		}

		random_device rd;
		mt19937 gen(rd());
		uniform_int_distribution<int> dist(0, tvseriesItems.size() - 1);

		int randomIndex = dist(gen);
		tvseriesItems[randomIndex].displayDetails();
	}

	// Function to sort movies by duration
	void sortDuration(vector <Movie>& movieItems)
	{
		vector <int> mduration;
		for (int i = 0; i < movieItems.size(); i++)
		{
			mduration.push_back(movieItems[i].getDuration());
		}
		sort(mduration.begin(), mduration.end());
		cout << endl << "Movie Titles sorted according to Duration: " << endl << endl;
		for (int i = 0; i < movieItems.size(); i++)
		{
			cout << i + 1 << ". ";
			movieItems.at(i).getTitle();
			cout << ": ";
			cout << mduration[i] << " minutes";
			cout << endl << endl;
		}
	p:
		cout << "\nDo you wish to see the details of the selected Media Items?" << endl;
		cout << "1. Display Details" << endl << "2. Go to Recommendation Menu" << endl;
		int ch1;
		cout << "Enter your choice: ";
		cin >> ch1;
		cout << endl;
		if (ch1 == 1)
		{
			cout << "Enter the name of the movie you wish to explore more: ";
			string movie_t;
			cin.ignore();
			getline(cin, movie_t);
			transform(movie_t.begin(), movie_t.end(), movie_t.begin(), toupper);
			cout << endl;
			for (int i = 0; i < movieItems.size(); i++)
			{
				if (movie_t == movieItems.at(i).get_t())
				{
					movieItems.at(i).displayDetails();
					cout << endl;
				}
			}
		q:
			cout << "Do you wish to view the details of any other movie? (Yes/ No) ";
			string yn2;
			cin >> yn2;
			transform(yn2.begin(), yn2.end(), yn2.begin(), toupper);
			cout << endl;
			if (yn2 == "YES")
			{
				cout << "Enter the name of the movie you wish to explore more: ";
				string movie_t1;
				cin.ignore();
				getline(cin, movie_t1);
				transform(movie_t1.begin(), movie_t1.end(), movie_t1.begin(), toupper);
				cout << endl;
				for (int i = 0; i < movieItems.size(); i++)
				{
					if (movie_t1 == movieItems.at(i).get_t())
					{
						movieItems.at(i).displayDetails();
						cout << endl;
					}
				}
				goto q;
			}
			else
			{
				return;
			}
		}
		else if (ch1 == 2)
		{
			return;
		}
		else
		{
			cout << "INVALID INPUT!!!" << endl;
			cout << "Please enter either '1' or '2'" << endl;
			goto p;
		}
	}

};

class CafeRecommendationSystem {
private:
	unique_ptr<Database> db;

public:
	CafeRecommendationSystem(unique_ptr<Database> db) : db(move(db)) {}

	void recommendCafes(int budget) {
		MYSQL* conn;
		conn = mysql_init(0);

		conn = mysql_real_connect(conn, "localhost", "root", "", "booking", 3306, NULL, 0);

		string query = "SELECT sno, cafe_name, price_per_head FROM cafes WHERE price_per_head <= " + to_string(budget);
		int count = 0;
		if (mysql_query(conn, query.c_str()) == 0) {
			MYSQL_RES* res = mysql_store_result(conn);
			if (res) {
				cout << "Cafe Recommendations within your budget:" << endl;
				cout << "+-----+---------------------------+-------------------------+" << endl;
				cout << "| SNo | Cafe Name                 | Price Per Head (Ruppees)|" << endl;
				cout << "+-----+---------------------------+-------------------------+" << endl;

				MYSQL_ROW row;
				while ((row = mysql_fetch_row(res))) {
					count++;
					cout << "| " << setw(3) << row[0] << " | " << setw(25) << left << row[1] << " | " << setw(17) << right << row[2] << " |" << endl;
				}

				cout << "+-----+---------------------------+-------------------+" << endl;

				mysql_free_result(res);
			}
			else {
				cerr << "Failed to fetch results from MySQL." << endl;
			}
		}
		else {
			cerr << "Error executing query: " << mysql_error(conn) << endl;
		}
	}

	int bookCafe(int sno, int numPeople, int budget) {
		MYSQL* conn;
		conn = mysql_init(0);

		conn = mysql_real_connect(conn, "localhost", "root", "", "booking", 3306, NULL, 0);
		string query = "SELECT cafe_name, price_per_head FROM cafes WHERE sno = " + to_string(sno);
		if (mysql_query(conn, query.c_str()) == 0) {
			MYSQL_RES* res = mysql_store_result(conn);
			if (res) {
				MYSQL_ROW row = mysql_fetch_row(res);
				string cafeName = row[0];
				int pricePerHead = atoi(row[1]);
				int totalPrice = pricePerHead * numPeople;

				cout << "Booking Confirmation:" << endl;
				cout << "---------------------" << endl;
				cout << "Cafe Name: " << cafeName << endl;
				cout << "Number of People: " << numPeople << endl;
				cout << "Total Price: Rs. " << totalPrice << endl;
				if (totalPrice > budget) {
					cout << "\n Sorry for the inconvinience but your budget is less than the total price";
					return 0;
				}
				else {

					return budget - totalPrice;
				}
				mysql_free_result(res);
			}
			else {
				cerr << "Failed to fetch cafe details." << endl;
			}
		}
		else {
			cerr << "Error executing query: " << mysql_error(conn) << endl;
		}
	}
};


// Main function
int main()
{
	try
	{
		unique_ptr<Database> db = make_unique<MySQLDatabase>();
		unique_ptr<Database> db_log = make_unique<MySQLDatabase>();
		unique_ptr<Database> db_caf = make_unique<MySQLDatabase>();
		Login login(move(db_log));
		BookingManager bookingManager(move(db));
		CafeRecommendationSystem Cafemanager(move(db_caf));
		string username, password;
		int choice;
	b:
		cout << "*******************************************************" << endl;
		cout << "\tCINEBLISS: WHERE EVERY MOVIE IS A HIT!" << endl;
		cout << "*******************************************************" << endl;
		do
		{
			cout << "\nMain Menu\n";
			cout << "\tPRESS 1->Sign Up\n";
			cout << "\tPRESS 2->Login\n";
			cout << "\tPRESS 3->Exit\n";
			cout << "\tEnter your choice: ";
			cin >> choice;

			if (choice == 1)
			{
				login.signup();
				delay();
				clear();
			}
			else if (choice == 2)
			{
				if (login.login())
				{

					cout << "\n\t\tENTER YOUR DETAILS BEFORE WE CONTINUE!!" << endl;
					bookingManager.getDetails();
					// code for choosing between cafe booking & movie booking.
				a:
					cout << "\n\nWhat would you like to do?" << endl<< endl;
					cout << "\tPRESS 1->MOVIE / TV SERIES " << endl;
					cout << "\tPRESS 2->CAFE " << endl;
					cout << "\tPRESS 3->LOG OUT " << endl;
					int choice1;
					while (true)
					{
						cout << "\nENTER CHOICE:";
						cin >> choice1;
						if (choice1 == 1 || choice1 == 2 || choice1 == 3)
						{
							break;
						}
						else
						{
							cout << "\nCHOOSE A VALID OPTION!!";
						}
					}
					if (choice1 == 1)
					{
					c:
						cout << "\n \tWELCOME TO MOVIES / TV SERIES! ~ spend your time watching the classics or the new jams\n\t\t\ttheres everything for everyone!" << endl;
						cout << "WHAT ARE YOU UPTO TODAY?" << endl;
						cout << "PRESS 1->MOVIE BOOKING" << endl;
						cout << "PRESS 2->MY BOOKINGS" << endl;
						cout << "PRESS 3->MOVIE/TV SERIES RECOMMENDATIONS" << endl;
						cout << "PRESS 4->EXIT" << endl;
						int choice2;
						while (true)
						{
							cout << "\nENTER CHOICE:";
							cin.ignore();
							cin >> choice2;
							if (choice2 > 0 && choice2 < 5)
							{
								break;
							}
							else
							{
								cout << "\nCHOOSE A VALID OPTION!!";
							}
						}
						if (choice2 == 1) {

							if (bookingManager.bookTickets()) {
								bookingManager.saveTicket();
								goto c;
							}
							else {
								goto c;
							}

						}
						else if (choice2 == 3)
						{
							//data initialization
							vector <Movie> movieItems;
							vector <TVSeries> tvseriesItems;

							Movie movie1(1, "INCEPTION", "ACTION", "DRAMA", "NETFLIX", "ENGLISH", 8.8,
								"Movie", { "Leonardo DiCaprio", "Joseph Gordon-Levitt" }, "2010-07-16", 148,
								"A thief who steals corporate secrets through the use of dream-sharing technology is given the inverse task of planting an idea into the mind of a CEO. But his tragic past may doom the project and his team to disaster.",
								"It focuses on the emotional journey of its lead character Cobb but at the same time thrusts the audience into multiple levels of action-packed storytelling very distinct from one another but all finely connected.");
							movieItems.push_back(movie1);

							Movie movie2(2, "EXTRACTION", "ACTION", "THRILLER", "NETFLIX", "ENGLISH", 6.7,
								"Movie", { "Chris Hemsworth", "Rudhraksh Jaiswal", "Randeep Hooda" }, "2020-04-24", 117,
								"A black ops mercenary embarks on a dangerous mission to rescue a kidnapped boy in Dhaka, Bangladesh.",
								"Extraction delivers adrenaline-pumping action sequences with Chris Hemsworth''s compelling performance, making it an engaging watch despite its predictable plot.");
							movieItems.push_back(movie2);

							TVSeries TVSeries1(3, "BREAKING BAD", "DRAMA", "THRILLER", "NETFLIX", "ENGLISH", 9.5,
								"TV Series", { "Bryan Cranston", "Aaron Paul" }, "2008-01-20", 62,
								"A chemistry teacher diagnosed with inoperable lung cancer turns to manufacturing and selling methamphetamine with a former student in order to secure his family's future.",
								"Breaking Bad is every bit as good as everyone says it is. It's easily one of my favorite shows of all time and one of the best shows in the history of television.");
							tvseriesItems.push_back(TVSeries1);

							Movie movie3(4, "KINGDOM", "ACTION", "HORROR", "NETFLIX", "KOREAN", 8.4,
								"Movie", { "Ju Ji-hoon", "Ryu Seung-ryong", "Bae Doona" }, "2019-01-25", 153,
								"Set in Korea's Joseon era, a prince investigates a mysterious plague outbreak that turns people into zombies, threatening the kingdom's survival.",
								"Kingdom masterfully blends historical drama with horror elements, offering intense action and suspenseful storytelling that keeps viewers hooked from start to finish.");
							movieItems.push_back(movie3);

							TVSeries TVSeries2(5, "JACK RYAN", "ACTION", "HISTORICAL", "PRIME VIDEO", "ENGLISH", 8.1,
								"TV Series", { "John Krasinski", "Wendell Pierce", "Abbie Cornish" }, "2018-08-31", 16,
								"CIA analyst Jack Ryan uncovers terrorist plots and navigates global threats while working to protect the United States.",
								"Jack Ryan offers a gripping portrayal of modern geopolitical tensions, complemented by John Krasinski's charismatic performance and thrilling action sequences.");
							tvseriesItems.push_back(TVSeries2);

							TVSeries TVSeries3(6, "MIRZAPUR", "ACTION", "DRAMA", "PRIME VIDEO", "HINDI", 8.4,
								"TV Series", { "Pankaj Tripathi", "Ali Fazal", "Divyendu Sharma" }, "2018-11-16", 19,
								"In the lawless town of Mirzapur, power struggles and violence escalate as crime lords vie for control over the underworld.",
								"Mirzapur captivates with its gritty narrative, stellar performances, and intense action, making it a must-watch for fans of the crime genre.");
							tvseriesItems.push_back(TVSeries3);

							TVSeries TVSeries4(7, "SPECIAL OPS", "ACTION", "ROMANCE", "DISNEY+ HOTSTAR", "HINDI", 8.7,
								"TV Series", { "Kay Kay Menon", "Karan Tacker", "Divya Dutta" }, "2020-03-17", 8,
								"A RAW agent investigates a conspiracy spanning multiple countries and decades, uncovering a deadly terrorist plot.",
								"Special Ops delivers a gripping narrative filled with twists and turns, enhanced by Kay Kay Menon's brilliant performance and high-octane action sequences.");
							tvseriesItems.push_back(TVSeries4);

							TVSeries TVSeries5(8, "RANGBAAZ", "ACTION", "COMEDY", "ZEE5", "HINDI", 8.4,
								"TV Series", { "Saqib Saleem", "Tigmanshu Dhulia", "Ranvir Shorey" }, "2018-12-22", 18,
								"Inspired by true events, Rangbaaz follows the rise and fall of a young gangster in Uttar Pradesh, India, amidst political intrigue and violence.",
								"Rangbaaz offers a compelling narrative supported by strong performances and gritty realism, making it a captivating watch for fans of crime dramas.");
							tvseriesItems.push_back(TVSeries5);

							TVSeries TVSeries6(9, "AVRODH: THE SIEGE WITHIN", "ACTION", "DRAMA", "SONYLIV", "HINDI", 9.0,
								"TV Series", { "Amit Sadh", "Darshan Kumaar", "Neeraj Kabi" }, "2020-07-31", 9,
								"Based on the 2016 Uri attack, Avrodh depicts the Indian Army's surgical strike on terrorist camps, exploring the strategic planning and execution behind the operation.",
								"Avrodh offers a gripping portrayal of real-life events, with strong performances and intense action sequences that honor the bravery of India's armed forces.");
							tvseriesItems.push_back(TVSeries6);

							Movie movie4(10, "DHAMAAL", "COMEDY", "ACTION", "SONYLIV", "HINDI", 7.4,
								"Movie", { "Sanjay Dutt", "Riteish Deshmukh", "Arshad Warsi" }, "2007-09-07", 136,
								"Four friends set out on a quest to find a hidden treasure with hilarious consequences.",
								"Dhamaal is a laugh riot from start to finish, with hilarious performances and memorable moments.");
							movieItems.push_back(movie4);

							Movie movie5(11, "SUPERBAD", "COMEDY", "ROMANCE", "ZEE5", "ENGLISH", 7.6,
								"Movie", { "Jonah Hill", "Michael Cera", "Christopher Mintz-Plasse" }, "2007-08-17", 113,
								"Two high school friends embark on a wild journey to buy alcohol for a party, encountering various obstacles along the way.",
								"Superbad is a hilarious coming-of-age comedy with witty dialogue and endearing characters.");
							movieItems.push_back(movie5);

							Movie movie6(12, "MY SASSY GIRL", "COMEDY", "DRAMA", "DISNEY+ HOTSTAR", "KOREAN", 8.0,
								"Movie", { "Cha Tae-hyun", "Jun Ji-hyun" }, "2001-07-27", 123,
								"A young man falls in love with a quirky and unpredictable girl, leading to a series of comedic and heartwarming adventures.",
								"My Sassy Girl is a charming romantic comedy with laugh-out-loud moments and a heartwarming story.");
							movieItems.push_back(movie6);

							Movie movie7(13, "CRAZY ASS GIRL", "COMEDY", "HISTORICAL", "PRIME VIDEO", "ENGLISH", 7.0,
								"Movie", { "Constance Wu", "Henry Golding", "Michelle Yeoh" }, "2018-08-15", 120,
								"A woman travels to Singapore to meet her boyfriend's family and discovers that they are insanely wealthy, leading to comedic and dramatic situations.",
								"Crazy Rich Asians is a delightful romantic comedy with stunning visuals and a talented ensemble cast.");
							movieItems.push_back(movie7);

							TVSeries TVSeries7(14, "THE OFFICE", "COMEDY", "HORROR", "NETFLIX", "ENGLISH", 8.9,
								"TV Series", { "Steve Carell", "Rainn Wilson", "John Krasinski" }, "2005-03-24", 201,
								"A mockumentary-style sitcom following the daily lives of office employees working at the Dunder Mifflin Paper Company.",
								"The Office is a hilarious and heartwarming series with memorable characters and moments.");
							tvseriesItems.push_back(TVSeries7);

							TVSeries TVSeries8(15, "BROOKLYN NINE-NINE", "COMEDY", "ACTION", "NETFLIX", "ENGLISH", 8.4,
								"TV Series", { "Andy Samberg", "Terry Crews", "Stephanie Beatriz" }, "2013-09-17", 153,
								"A group of detectives in the fictional 99th precinct of the New York City Police Department tackle crime and absurd situations.",
								"Brooklyn Nine-Nine is a witty and entertaining series with a diverse cast and clever writing.");
							tvseriesItems.push_back(TVSeries8);

							TVSeries TVSeries9(16, "THE KING'S AVATAR", "COMEDY", "ACTION", "ZEE5", "CHINESE", 8.5,
								"TV Series", { "Yang Yang", "Jiang Shuying", "Lai Yumeng" }, "2019-07-24", 40,
								"A former professional eSports player returns to the gaming world to reclaim his title and build a new team.",
								"The King's Avatar is a refreshing blend of comedy and action, with stunning animation and engaging characters.");
							tvseriesItems.push_back(TVSeries9);

							TVSeries TVSeries10(17, "REPLY 1988", "COMEDY", "ROMANCE", "SONYLIV", "KOREAN", 9.1,
								"TV Series", { "Lee Hye-ri", "Park Bo-gum", "Ryu Jun-yeol" }, "2015-11-06", 20,
								"A heartwarming story set in the late 1980s, following the lives of families living in a neighborhood in Seoul.",
								"Reply 1988 is a nostalgic and heartwarming series with lovable characters and a strong sense of community.");
							tvseriesItems.push_back(TVSeries10);

							Movie movie8(18, "THE WANDERING EARTH", "ACTION", "HORROR", "NETFLIX", "CHINESE", 6.0,
								"Movie", { "Qu Chuxiao", "Li Guangjie", "Ng Man-tat" }, "2019-02-05", 125,
								"In the distant future, humanity attempts to move the Earth out of its orbit to avoid an expanding sun, leading to a perilous journey and unexpected challenges.",
								"The Wandering Earth is a visually stunning sci-fi epic with thrilling action sequences and a thought-provoking premise.");
							movieItems.push_back(movie8);

							Movie movie9(19, "RAISE THE RED LANTERN", "DRAMA", "ROMANCE", "PRIME VIDEO", "CHINESE", 8.1,
								"Movie", { "Gong Li", "Ma Jingwu", "He Saifei" }, "1991-12-20", 125,
								"A young woman becomes the fourth wife of a wealthy man and must navigate the complex power dynamics and rivalries among his concubines.",
								"Raise the Red Lantern is a beautifully crafted drama with exquisite cinematography and powerful performances.");
							movieItems.push_back(movie9);

							Movie movie10(20, "MONSTER HUNT", "COMEDY", "HISTORICAL", "ZEE5", "CHINESE", 6.1,
								"Movie", { "Jing Boran", "Bai Baihe", "Jiang Wu" }, "2015-07-16", 117,
								"In a world where humans and monsters coexist, a young man becomes entangled in a quest to protect an unborn monster prince from evil forces.",
								"Monster Hunt is a whimsical and entertaining fantasy film with charming characters and imaginative world-building.");
							movieItems.push_back(movie10);

							Movie movie11(21, "CROUCHING TIGER, HIDDEN DRAGON", "ACTION", "ROMANCE", "PRIME VIDEO", "CHINESE", 7.8,
								"Movie", { "Chow Yun-fat", "Michelle Yeoh", "Zhang Ziyi" }, "2000-05-12", 120,
								"A legendary warrior gives up his sword and becomes embroiled in a quest to recover a stolen sword, leading to breathtaking martial arts battles and romantic entanglements.",
								"Crouching Tiger, Hidden Dragon is a martial arts masterpiece with stunning choreography and a captivating story.");
							movieItems.push_back(movie11);

							TVSeries TVSeries11(22, "STORY OF YANXI PALACE", "DRAMA", "ROMANCE", "NETFLIX", "CHINESE", 8.0,
								"TV Series", { "Wu Jinyan", "Charmaine Sheh", "Nie Yuan" }, "2018-07-19", 70,
								"A young woman rises from lowly maid to the most powerful concubine in the Qing Dynasty's Forbidden City, navigating treachery and intrigue along the way.",
								"Story of Yanxi Palace is an addictive historical drama with compelling characters and lavish production values.");
							tvseriesItems.push_back(TVSeries11);

							TVSeries TVSeries12(23, "THE UNTAMED", "DRAMA", "ROMANCE", "NETFLIX", "CHINESE", 8.7,
								"TV Series", { "Xiao Zhan", "Wang Yibo", "Sean Xiao" }, "2019-06-27", 50,
								"In a world of magic and cultivation, two young men form an unlikely alliance to unravel dark secrets and uncover the truth about their pasts.",
								"The Untamed is an epic fantasy series with breathtaking visuals and a captivating story.");
							tvseriesItems.push_back(TVSeries12);

							TVSeries TVSeries13(24, "NIRVANA IN FIRE", "DRAMA", "HISTORICAL", "PRIME VIDEO", "CHINESE", 9.1,
								"TV Series", { "Hu Ge", "Liu Tao", "Wang Kai" }, "2015-09-19", 54,
								"A master strategist seeks revenge against those who framed him for treason, using his intellect and wit to navigate the treacherous political landscape of ancient China.",
								"Nirvana in Fire is a sweeping historical drama with intricate plotting and compelling characters.");
							tvseriesItems.push_back(TVSeries13);

							Movie movie12(25, "TITANIC", "ROMANCE", "ACTION", "NETFLIX", "ENGLISH", 7.8,
								"Movie", { "Leonardo DiCaprio", "Kate Winslet", "Billy Zane" }, "1997-12-19", 195,
								"A young couple from different social classes fall in love aboard the ill-fated RMS Titanic.",
								"Titanic is a timeless romantic drama with breathtaking visuals and unforgettable performances.");
							movieItems.push_back(movie12);

							Movie movie13(26, "500 DAYS OF SUMMER", "ROMANCE", "THRILLER", "PRIME VIDEO", "ENGLISH", 7.7,
								"Movie", { "Joseph Gordon-Levitt", "Zooey Deschanel", "Geoffrey Arend" }, "2009-07-17", 95,
								"A young man reflects on his failed relationship with the quirky and elusive woman he fell in love with.",
								"500 Days of Summer is a refreshingly honest and bittersweet romantic comedy that explores the complexities of love and relationships.");
							movieItems.push_back(movie13);

							Movie movie14(27, "THE NOTEBOOK", "ROMANCE", "DRAMA", "ZEE5", "HINDI", 7.8,
								"Movie", { "Ryan Gosling", "Rachel McAdams", "James Garner" }, "2004-06-25", 123,
								"A young couple falls in love during the early 1940s, but their relationship faces obstacles as they grow older.",
								"The Notebook is a heartfelt romantic drama with beautiful performances and an emotional storyline.");
							movieItems.push_back(movie14);

							TVSeries TVSeries14(28, "CRASH LANDING ON YOU", "ROMANCE", "DRAMA", "NETFLIX", "KOREAN", 8.7,
								"TV Series", { "Hyun Bin", "Son Ye-jin", "Seo Ji-hye" }, "2019-12-14", 16,
								"A South Korean heiress crash-lands in North Korea and falls in love with a North Korean army officer, leading to a forbidden romance and political intrigue.",
								"Crash Landing on You is a captivating romantic drama with a unique premise and strong chemistry between the leads.");
							tvseriesItems.push_back(TVSeries14);

							TVSeries TVSeries15(29, "TOUCH YOUR HEART", "ROMANCE", "HISTORICAL", "PRIME VIDEO", "KOREAN", 8.0,
								"TV Series", { "Yoo In-na", "Lee Dong-wook", "Lee Sang-woo" }, "2019-02-06", 16,
								"A popular actress goes undercover as a secretary to a prickly lawyer in order to prepare for her next role, leading to unexpected romance and heartwarming moments.",
								"Touch Your Heart is a delightful romantic comedy with charming characters and hilarious situations.");
							tvseriesItems.push_back(TVSeries15);

							TVSeries TVSeries16(30, "ETERNAL LOVE", "ROMANCE", "HORROR", "SONYLIV", "HINDI", 8.3,
								"TV Series", { "Yang Mi", "Mark Chao", "Ken Chang" }, "2017-01-30", 58,
								"A thousand-year-old fox spirit and a powerful deity reincarnate multiple times in search of true love and redemption, facing obstacles and enemies along the way.",
								"Eternal Love is a mesmerizing fantasy romance with stunning visuals and a compelling love story.");
							tvseriesItems.push_back(TVSeries16);

							TVSeries TVSeries17(31, "A LOVE SO BEAUTIFUL", "ROMANCE", "COMEDY", "DISNEY+ HOTSTAR", "CHINESE", 8.1,
								"TV Series", { "Hu Yitian", "Shen Yue", "Gao Zhi Ting" }, "2017-11-09", 23,
								"Two childhood friends navigate the ups and downs of adolescence and young adulthood as they discover love and friendship.",
								"A Love So Beautiful is a sweet and nostalgic romantic comedy with endearing characters and heartfelt moments.");
							tvseriesItems.push_back(TVSeries17);

							Movie movie15(32, "TRAIN TO BUSAN", "HORROR", "DRAMA", "PRIME VIDEO", "KOREAN", 7.6,
								"Movie", { "Gong Yoo", "Ma Dong-seok" }, "2016-07-20", 118,
								"While a zombie virus breaks out in South Korea, passengers struggle to survive on the train from Seoul to Busan.",
								"Train to Busan offers heart-pounding thrills and emotional depth, setting a new standard for zombie movies.");
							movieItems.push_back(movie15);

							Movie movie16(33, "GET OUT", "HORROR", "ACTION", "ZEE5", "ENGLISH", 7.7,
								"Movie", { "Daniel Kaluuya", "Allison Williams" }, "2017-02-24", 104,
								"A young African-American visits his white girlfriend's parents for the weekend, where his simmering uneasiness about their reception of him eventually reaches a boiling point.",
								"Get Out is a smart, chilling, and thought-provoking horror film that delivers an unforgettable experience.");
							movieItems.push_back(movie16);

							TVSeries TVSeries18(34, "THE HAUNTING OF HILL HOUSE", "HORROR", "HISTORICAL", "NETFLIX", "HINDI", 8.6,
								"TV Series", { "Michiel Huisman", "Elizabeth Reaser" }, "2018-10-12", 10,
								"Flashing between past and present, a fractured family confronts haunting memories of their old home and the terrifying events that drove them from it.",
								"The Haunting of Hill House is a chilling and emotionally resonant horror series that redefines the genre.");
							tvseriesItems.push_back(TVSeries18);

							Movie movie17(35, "THE WITCH", "HORROR", "COMEDY", "SONYLIV", "ENGLISH", 6.9,
								"Movie", { "Anya Taylor-Joy", "Ralph Ineson" }, "2015-01-23", 92,
								"A family in 1630s New England is torn apart by the forces of witchcraft, black magic, and possession.",
								"The Witch mesmerizes with its eerie atmosphere, unsettling mood, and masterful storytelling.");
							movieItems.push_back(movie17);

							TVSeries TVSeries19(36, "AMERICAN HORROR STORY", "HORROR", "ROMANCE", "PRIME VIDEO", "ENGLISH", 8.0,
								"TV Series", { "Sarah Paulson", "Evan Peters" }, "2011-10-05", 103,
								"An anthology series that centers on different characters and locations, including a haunted house, an insane asylum, a witch coven, a freak show, and a hotel.",
								"American Horror Story delivers thrills, scares, and captivating storytelling across its various seasons, each with its own unique setting and characters.");
							tvseriesItems.push_back(TVSeries19);

							TVSeries TVSeries20(37, "MR. SUNSHINE", "HISTORICAL", "ROMANCE", "DISNEY+ HOTSTAR", "HINDI", 8.7,
								"TV Series", { "Lee Byung-hun", "Kim Tae-ri" }, "2018-07-07", 24,
								"Set during the early 20th century in Korea under Japanese rule, a Korean-born U.S. Marine officer discovers a plot to colonize the country.",
								"Mr. Sunshine is a visually stunning and emotionally gripping historical drama with a compelling narrative and exceptional performances.");
							tvseriesItems.push_back(TVSeries20);

							TVSeries TVSeries21(38, "NIRVANA IN FIRE", "HISTORICAL", "COMEDY", "NETFLIX", "CHINESE", 9.0,
								"TV Series", { "Hu Ge", "Wang Kai" }, "2015-09-19", 54,
								"A tale of revenge, strategy, and political intrigue set in the fictional kingdom of Liang during the tumultuous Five Dynasties and Ten Kingdoms period, following the journey of a brilliant strategist seeking justice for his family.",
								"Nirvana in Fire is a masterfully crafted historical drama that combines intricate plotting, compelling characters, and breathtaking visuals to deliver a captivating story of loyalty, betrayal, and redemption.");
							tvseriesItems.push_back(TVSeries21);

							TVSeries TVSeries22(39, "THE LAST KINGDOM", "HISTORICAL", "ACTION", "ZEE5", "ENGLISH", 8.4,
								"TV Series", { "Alexander Dreymon", "David Dawson" }, "2015-10-10", 45,
								"Based on Bernard Cornwell's 'The Saxon Stories,' follows Uhtred of Bebbanburg as he navigates the turbulent events of 9th-century England, torn between his Saxon heritage and Viking upbringing.",
								"The Last Kingdom is a gripping historical epic that offers a visceral portrayal of the Viking Age, complete with intense battles, complex characters, and intricate political machinations.");
							tvseriesItems.push_back(TVSeries22);

							TVSeries TVSeries23(40, "CHANDRAGUPTA MAURYA", "HISTORICAL", "THRILLER", "PRIME VIDEO", "HINDI", 8.2,
								"TV Series", { "Rajat Tokas", "Shweta Basu Prasad" }, "2011-03-01", 124,
								"Chronicles the rise of Chandragupta Maurya, the founder of the Maurya Empire, as he overcomes various challenges and adversaries to establish one of the most powerful dynasties in ancient India.",
								"Chandragupta Maurya is a riveting historical drama that brings to life the epic tale of a legendary king and the founding of one of India's greatest empires, with gripping storytelling and stellar performances.");
							tvseriesItems.push_back(TVSeries23);

							Movie movie18(41, "THE CURSED EMPEROR", "HISTORICAL", "HORROR", "SONYLIV", "ENGLISH", 7.9,
								"Movie", { "Emma Stone", "Tom Hiddleston", "Benedict Wong" }, "2023-10-15", 132,
								"Set in ancient China during the Han Dynasty, 'The Cursed Emperor' tells the chilling tale of Emperor Qin Shi Huang's quest for immortality. Obsessed with cheating death, he delves into dark rituals and forbidden sorcery, unleashing a supernatural terror upon his empire. As the curse spreads, a group of brave warriors and scholars must unravel the mystery behind the emperor's obsession before it consumes them all.",
								"The Cursed Emperor combines historical intrigue with spine-tingling horror, offering a unique and unsettling cinematic experience. With stellar performances and atmospheric storytelling, it transports audiences to a world where the line between myth and reality blurs, leaving them on the edge of their seats till the very end.");
							movieItems.push_back(movie18);

							Movie movie19(42, "THE SHAWSHANK REDEMPTION", "DRAMA", "COMEDY", "NETFLIX", "ENGLISH", 9.3,
								"Movie", { "Tim Robbins", "Morgan Freeman", "Bob Gunton" }, "1994-09-23", 142,
								"Two imprisoned men bond over a number of years, finding solace and eventual redemption through acts of common decency.",
								"The Shawshank Redemption is a timeless classic with powerful performances and a moving story of hope and resilience.");
							movieItems.push_back(movie19);

							Movie movie20(43, "GONE WITH THE WIND", "DRAMA", "ACTION", "PRIME VIDEO", "ENGLISH", 8.1,
								"Movie", { "Clark Gable", "Vivien Leigh", "Leslie Howard" }, "1939-01-17", 238,
								"A manipulative woman and a roguish man conduct a turbulent romance during the American Civil War and Reconstruction periods.",
								"Gone with the Wind is an epic romance with sweeping cinematography and unforgettable characters.");
							movieItems.push_back(movie20);

							Movie movie21(44, "PARASITE", "DRAMA", "THRILLER", "DISNEY+ HOTSTAR", "KOREAN", 8.6,
								"Movie", { "Song Kang-ho", "Lee Sun-kyun", "Cho Yeo-jeong" }, "2019-05-30", 132,
								"A poor family cunningly ingratiates themselves into the lives of a wealthy household, but their deception soon unravels with devastating consequences.",
								"Parasite is a masterful thriller with brilliant direction and social commentary, earning its place as one of the greatest films of the 21st century.");
							movieItems.push_back(movie21);

							Movie movie22(45, "TO LIVE", "DRAMA", "HISTORICAL", "ZEE5", "HINDI", 8.3,
								"Movie", { "Ge You", "Gong Li", "Niu Ben" }, "1994-05-18", 125,
								"Spanning several decades, a Chinese family endures the trials and tribulations of political upheaval and societal change in 20th-century China.",
								"To Live is a poignant and powerful drama that captures the resilience and spirit of the human condition amidst turbulent times.");
							movieItems.push_back(movie22);

							TVSeries TVSeries24(46, "MR. SUNSHINE", "DRAMA", "ROMANCE", "NETFLIX", "KOREAN", 8.7,
								"TV Series", { "Lee Byung-hun", "Kim Tae-ri", "Yoo Yeon-seok" }, "2018-07-07", 24,
								"A Korean-born U.S. Marine returns to his homeland as a conscripted soldier during the late 19th century, where he falls in love and becomes embroiled in the fight for Korean independence.",
								"Mr. Sunshine is a visually stunning epic with a compelling historical backdrop and richly developed characters.");
							tvseriesItems.push_back(TVSeries24);

							TVSeries TVSeries25(47, "THE STORY OF MING LAN", "DRAMA", "ROMANCE", "DISNEY+ HOTSTAR", "CHINESE", 8.6,
								"TV Series", { "Zhao Liying", "Feng Shaofeng", "Zhu Yilong" }, "2018-12-25", 73,
								"A young noblewoman navigates the intricacies of court politics and family intrigue to secure her own happiness and protect her loved ones.",
								"The Story of Ming Lan is a captivating period drama with strong performances and a compelling storyline.");
							tvseriesItems.push_back(TVSeries25);

							Movie movie23(48, "SEVEN", "THRILLER", "ROMANCE", "ZEE5", "HINDI", 8.6,
								"Movie", { "Brad Pitt", "Morgan Freeman", "Kevin Spacey" }, "1995-09-22", 127,
								"Two detectives track a serial killer who uses the seven deadly sins as his motives in a series of gruesome murders.",
								"Seven is a gripping and intense thriller with a shocking twist, anchored by strong performances and a dark atmosphere.");
							movieItems.push_back(movie23);

							Movie movie24(49, "SHUTTER ISLAND", "THRILLER", "DRAMA", "PRIME VIDEO", "ENGLISH", 8.1,
								"Movie", { "Leonardo DiCaprio", "Mark Ruffalo", "Ben Kingsley" }, "2010-02-19", 138,
								"Two U.S. Marshals investigate the disappearance of a patient from a mental institution, uncovering dark secrets and confronting their own past traumas.",
								"Shutter Island is a mind-bending psychological thriller with haunting visuals and a compelling narrative that keeps viewers guessing until the very end.");
							movieItems.push_back(movie24);

							Movie movie25(50, "OLD BOY", "THRILLER", "HISTORICAL", "DISNEY+ HOTSTAR", "KOREAN", 8.4,
								"Movie", { "Choi Min-sik", "Yoo Ji-tae", "Kang Hye-jung" }, "2003-11-21", 120,
								"A man is inexplicably imprisoned in a cell for 15 years and released, seeking revenge on those responsible while uncovering the truth behind his captivity.",
								"Old Boy is a visceral and suspenseful thriller with shocking twists and stunning cinematography, leaving a lasting impact on its audience.");
							movieItems.push_back(movie25);

							Movie movie26(51, "THE WAILING", "THRILLER", "HORROR", "SONYLIV", "ENGLISH", 7.4,
								"Movie", { "Kwak Do-won", "Hwang Jung-min", "Chun Woo-hee" }, "2016-05-12", 156,
								"A small village is plagued by a series of mysterious deaths attributed to a supernatural entity, leading a bumbling policeman to investigate the sinister forces at play.",
								"The Wailing is a chilling and atmospheric horror thriller with complex characters and a sense of dread that lingers long after the credits roll.");
							movieItems.push_back(movie26);

							Movie movie27(52, "MONEY HEIST", "THRILLER", "ACTION", "NETFLIX", "SPANISH", 8.3,
								"TV Series", { "Úrsula Corberó", "Álvaro Morte", "Itziar Ituño" }, "2017-05-02", 41,
								"A criminal mastermind recruits eight thieves to carry out an ambitious plan to rob the Royal Mint of Spain, while a mysterious figure manipulates events from behind the scenes.",
								"Money Heist is a high-octane thriller with intricate plot twists and charismatic characters, keeping viewers on the edge of their seats.");
							movieItems.push_back(movie27);

							Movie movie28(53, "SIGNAL", "THRILLER", "COMEDY", "NETFLIX", "KOREAN", 8.6,
								"TV Series", { "Lee Je-hoon", "Kim Hye-soo", "Cho Jin-woong" }, "2016-01-22", 16,
								"A detective communicates with a profiler from the past through a mysterious walkie-talkie, teaming up to solve cold cases and prevent future tragedies.",
								"Signal is a gripping and thought-provoking thriller with an innovative premise and compelling storytelling.");
							movieItems.push_back(movie28);

							Movie movie29(54, "SACRED GAMES", "THRILLER", "COMEDY", "NETFLIX", "HINDI", 8.7,
								"TV Series", { "Saif Ali Khan", "Nawazuddin Siddiqui", "Radhika Apte" }, "2018-07-06", 16,
								"A troubled police officer is drawn into a web of conspiracy and corruption when he receives a cryptic message from a notorious crime lord, setting off a chain of events with far-reaching consequences.",
								"Sacred Games is a gritty and atmospheric thriller with complex characters and a gripping narrative that delves into the dark underbelly of Mumbai.");
							movieItems.push_back(movie29);

							Movie movie30(55, "THE BAD KIDS", "THRILLER", "ROMANCE", "DISNEY+ HOTSTAR", "CHINESE", 8.1,
								"TV Series", { "Qin Hao", "Wang Jingchun", "Zhu Zhu" }, "2020-06-16", 12,
								"A group of seemingly unrelated people are drawn together by a tragic incident involving a child, leading to shocking revelations and unexpected connections.",
								"The Bad Kids is a tense and compelling thriller with riveting performances and a complex storyline that keeps viewers guessing until the very end.");
							movieItems.push_back(movie30);




							//Object of Recommender Class
							Recommender rec;


							// Output Format
							cout << "WELCOME TO THE CINEBLISS RECOMMENDATION SYSTEM!!!" << endl;
							cout << "We are glad to assist you to find your PERFECT WATCH!!!" << endl;
							int time = bookingManager.getHours();

							if (time < 4 && time >= 1) // starting time should be in hours
							{
								cout << "According to the time constraints we suggest you should go for a movie!" << endl;
								cout << "Do you wish to see a list of movies based on their duration?(Yes/No)";
								string yn;
								cin.ignore();
								getline(cin, yn);
								transform(yn.begin(), yn.end(), yn.begin(), toupper);
								if (yn == "YES")
								{
									rec.sortDuration(movieItems);
									cout << "Please find a suitable option for further recommendations from the RECOMMENDATION MENU" << endl;
								}
								else
								{
									cout << "Please select a suitable option from the RECOMMENDATION MENU" << endl;
								}

							}

							else if (time >= 4)
							{
								cout << "You have a lot of free time in your hands." << endl;
								cout << "We assure you a pleasant watching experince!!!" << endl;
								cout << "Please select a suitable option from the RECOMMENDATION MENU" << endl;
							}

							//Main Menu
						mainmenu:
							cout << endl;
							cout << "******************************************************************";
							cout << endl;
							cout << "\t\t\tRECOMMENDATION MENU" << endl;
							cout << "1. Display a list of all Media Items" << endl; //DONE
							cout << "2. Display a list of either MOVIES or TV SERIES (as per choice)" << endl; //DONE
							cout << "3. Filter Media Items" << endl; //
							cout << "4. Search Media Items" << endl; //DONE
							cout << "5. MOVIE of the Day" << endl;
							cout << "6. TV SERIES of the Day" << endl;
							cout << "7. Wishlist" << endl; // WRITTEN BUT NO OUTPUT
							cout << "8. Exit" << endl; //DONE
							cout << endl;
							cout << "******************************************************************";
							cout << endl;

							int ch4;
							cout << "Enter You choice: ";
							cin >> ch4;

							if (ch4 == 1)
							{
								rec.displayAllItems(movieItems, tvseriesItems);
								goto mainmenu;
							}
							else if (ch4 == 2)
							{
								rec.displayMoviesorSeries(movieItems, tvseriesItems);
								goto mainmenu;
							}
							else if (ch4 == 3)
							{
								rec.filter(movieItems, tvseriesItems);
								goto mainmenu;
							}
							else if (ch4 == 4)
							{
								rec.searchByTitle(movieItems, tvseriesItems);
								goto mainmenu;
							}
							else if (ch4 == 5)
							{
								rec.recommendRandom(movieItems, tvseriesItems);
								goto mainmenu;
							}
							else if (ch4 == 6)
							{
								rec.recommendRandomtv(movieItems, tvseriesItems);
								goto mainmenu;
							}
							else if (ch4 == 7)
							{
								rec.wishlist(movieItems, tvseriesItems);
								goto mainmenu;
							}
							else if (ch4 == 8)
							{
								cout << "Thanks a lot for using CINEBLISS RECOMMENDATION SYSTEM!!!" << endl;
							}
							else
							{
								cout << "INVALID INPUT!!!" << endl;
								cout << "Please enter a number between '1' and '8'" << endl;
								goto mainmenu;
							}

							goto c;


						}
						else if (choice2 == 2)
						{
							bookingManager.printTicket();
							goto c;
						}
						else if (choice2 == 4)
						{
							goto a; // start of cinebliss
						}
					}

					else if (choice1 == 2)
					{
						int budget = bookingManager.getBudget();
						if (budget <= 200)
						{
							cout << "Your budget is insufficient to book a cafe." << endl;
							cout << "Do you want to increase your budget?" << endl;
							cout << "1. Yes" << endl << "2. No : ";
							int increaseBudgetChoice;
							cin >> increaseBudgetChoice;
							if (increaseBudgetChoice == 1)
							{
								// Add code here to increase the budget
								// For example, you can prompt the user to enter a new budget
								cout << "Enter your new budget: ";
								cin >> budget;
								bookingManager.setBudget(budget);
							}
							else if (increaseBudgetChoice == 2)
							{
								cout << "Returning to the main menu." << endl;
								delay();
								clear();
								goto a;
							}
							else
							{
								cout << "Invalid choice. Returning to the main menu." << endl;
								delay();
								clear();
								goto a;
							}
						}
					cafemenu1:
						Cafemanager.recommendCafes(budget);

						int choice, numPeople;
					cafemenu2:
						cout << "Enter the serial number of the cafe you want to book: ";
						cin >> choice;
						cout << "Enter the number of people coming: ";
						cin >> numPeople;

						int condition = Cafemanager.bookCafe(choice, numPeople, budget);
						if (condition) {
							cout << endl;
							delay();
							cout << "PROCESSING PAYMENT......" << endl << endl;
							budget = budget - condition;
							bookingManager.setBudget(budget);
							delay();
							cout << "THANKS FOR THE PAYMENT!" << endl;
							cout << "VISIT THE CAFE AT " << bookingManager.getStartTime() << " and ENJOY YOUR STAY!" << endl;

							cout << "EXITING...";
							delay();
							clear();
							goto a;
						}
						else {
							cout << "Your budget is insufficient to book a cafe." << endl;
							cout << "Do you want to increase your budget?" << endl;
							cout << "1. Yes" << endl << "2. No : ";
							int increaseBudgetChoice;
							cin >> increaseBudgetChoice;
							if (increaseBudgetChoice == 1)
							{
								// Add code here to increase the budget
								// For example, you can prompt the user to enter a new budget
								cout << "Enter your new budget: ";
								cin >> budget;
								bookingManager.setBudget(budget);
								goto cafemenu1;
							}
							else if (increaseBudgetChoice == 2)
							{
								cout << "Returning to the main menu." << endl;
								delay();
								clear();
								goto a;
							}
							else
							{
								cout << "Invalid choice. Returning to the main menu." << endl;
								delay();
								clear();
								goto a;
							}
						}

					}
					else if (choice1 == 3)
					{
						goto b; // start of page
					}
					else
					{
						cout << "Enter valid option.";
						goto a;
					}
				}
			}
			else if (choice == 3)
			{
				cout << "Exiting program.\n";
				return 0;
			}
			else
			{
				cout << "ENTER A VALID CHOICE";
			}
		} while (choice != 3);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception caught: " << e.what() << std::endl;
		// Handle the exception gracefully, such as displaying an error message
		return 1; // Return a non-zero value to indicate failure
	}

	return 0;
}


