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

using namespace std;

void delay()
{
	this_thread::sleep_for(chrono::seconds(3));
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
	}
	Booking(const string& username, const string& state, const string& city, const string& landmark, const string& film,
		const string& price, const string& time, const string& hall, const vector<string>& seats,
		const string& language, const string& date, const string& startTime, const string& endTime, double budget, const int quan)
		: username(username), state(state), city(city), landmark(landmark), film(film), price(price), time(time), hall(hall),
		seats(seats), language(language), date(date), startTime(startTime), endTime(endTime), budget(budget), quantity(quan) {}

	string getUsername() const { return username; }
	string getState() const { return state; }
	string getCity() const { return city; }
	string getLandmark() const { return landmark; }
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
	string landmark;
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
		if (!mysql_real_connect(connection, "localhost", "root", "Daksh@2705", "booking", 3306, NULL, 0))
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

class Login
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
		string username, password;
		cout << "Enter a username: ";
		cin >> username;
		cout << "Enter a password: ";
		cin >> password;

		User newUser(username, password);

		if (addUser(newUser))
		{
			cout << "User registered successfully.\n";
		}
		else
		{
			cerr << "Failed to register user.\n";
		}
	}

	// Function to prompt user for login
	bool login()
	{
		string username, password;

		cout << "Enter your username: ";
		cin >> username;

		if (!checkUser(username))
		{
			cerr << "Username not found. Please signup or login again with a correct username\n";
			return false;
		}

		for (int attempt = 0; attempt < 3; ++attempt)
		{
			cout << "Enter your password: ";
			cin >> password;

			string storedPassword = getPassword(username);
			if (storedPassword == password)
			{
				cout << "Login successful! Welcome, " << username << "!\n";
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

	void bookTickets() {
		MYSQL* conn;
		conn = mysql_init(0);

		conn = mysql_real_connect(conn, "localhost", "root", "", "booking", 3306, NULL, 0);
		string query = "SELECT sno, theater_name, place, landmark FROM theaters";

		// Execute the query
		if (mysql_query(conn, query.c_str()) == 0) {
			MYSQL_RES* res = mysql_store_result(conn);
			if (res) {
				// Display the header
				cout << "Theaters:" << endl;
				cout << "+-----+-------------------+--------------------------------------+----------+" << endl;
				cout << "| " << setw(4) << "sno" << " | " << setw(17) << "theater_name" << " | " << setw(36) << "place" << " | " << setw(8) << "landmark" << " |" << endl;
				cout << "+-----+-------------------+--------------------------------------+----------+" << endl;

				// Process the result set
				MYSQL_ROW row;
				while ((row = mysql_fetch_row(res))) {
					cout << "| " << setw(4) << row[0] << " | " << setw(17) << row[1] << " | " << setw(36) << row[2] << " | " << setw(8) << row[3] << " |" << endl;
				}
				cout << "+-----+-------------------+--------------------------------------+----------+" << endl << endl;
				// Free the result set
				mysql_free_result(res);
			}
		}
		else {
			// Error handling if query execution fails
			cout << "Error: " << mysql_error(conn) << endl;
		}

		string hall;
		int n;
		int x = 0;
		do {
			cout << "ENTER CHOICE:";
			cin >> n;
			cin.ignore();
			switch (n) {
			case 1:
				hall = "DLF Promenade, Vasant Kunj";
				x++;
				break;
			case 2:
				hall = "Ambience Mall, Vasant Kunj";
				x++;
				break;
			case 3:
				hall = "DLF City Centre, Gurgaon";
				x++;
				break;
			case 4:
				hall = "Wave Mall, Noida";
				x++;
				break;
			case 5:
				hall = "DLF Mall of India, Noida";
				x++;
				break;
			case 6:
				hall = "Pacific Mall, Tagore Garden";
				x++;
				break;
			case 7:
				hall = "DLF Cyber Hub, Gurgaon";
				x++;
				break;
			case 8:
				hall = "Shipra Mall, Indirapuram";
				x++;
				break;
			case 9:
				hall = "Wave Mall, Kaushambi";
				x++;
				break;
			case 10:
				hall = "DLF Emporio, Vasant Kunj";
				x++;
				break;
			case 11:
				hall = "Ansal Plaza, Khel Gaon Marg";
				x++;
				break;
			case 12:
				hall = "Logix City Center Mall, Noida";
				x++;
				break;
			case 13:
				hall = "The Grand Venice Mall, Greater Noida";
				x++;
				break;
			case 14:
				hall = "Ambience Mall, Gurgaon";
				x++;
				break;
			case 15:
				hall = "Great India Place Mall, Noida";
				x++;
				break;
			default:
				cout << "ENTER A VALID OPTION!" << endl;
			}
		} while (x == 0);


		display(selectmovie());
		cout << endl;
		cout << "ENTER THE LANGUAGE YOU WANT TO WATCH THE MOVIE IN:" << endl;
		cout << "1. ENGLISH" << endl;
		cout << "2. HINDI" << endl;
		cout << "3. PUNJABI" << endl;
		cout << "4. TELUGU" << endl;
		do {
			cout << "ENTER CHOICE:";
			cin >> n;
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
		/**************************************ERROR OVER HERE...STOI CONVERSION ERROR*************************************************************************/
		price = db->get("movies", "price", "movie_name='" + film + "'");
		time = db->get("movies", "length", "movie_name='" + film + "'");
		int minutesToAdd = stoi(time);
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
		cout << "PRICE TO PAY IS: " << stoi(price) * quantity << endl;
		delay();
		cout << "PROCESSING PAYMENT......" << endl << endl;
		delay();
		cout << "THANKS FOR THE PAYMENT!" << endl;
		cout << "YOUR TICKET IS SAVED IN MY ORDERS TAB!" << endl << endl;;
		cout << "EXITING MOVIE BOOKING.....";
		delay();
		clear();
		return;
	}


	void getDetails()
	{
		string currentDate = getCurrentTime().substr(0, 10);
		cout << "Today's date is: " << currentDate << endl;

		do
		{
			cout << "Please select the date for your outing (up to 7 days from now, in format YYYY-MM-DD): ";
			cin >> date;
		} while (!isValidDate(date, currentDate, 7));

		do
		{
			cout << "Please select the starting time for your outing (between 10 am and 5 pm, in format HH:MM): ";
			cin >> startTime;
		} while (!isValidTime(startTime));

		while (true)
		{
			cout << "Enter your budget for the outing(budget should be greater than 500 Ruppees):";
			cin >> budget;

			if (budget < 500)
			{
				cout << "invalid budget! please enter an amount greater than 500.";
			}
			else
			{
				break;
			}
		}

		cout << "ENTER STATE:";
		cin.ignore();
		getline(cin, state);
		cout << "ENTER CITY:";
		getline(cin, city);
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
		getline(cin, landmark);

		// address thing idk... ********************************* NEED EDITING HERE ***************************************
		/*
			take 12 places
			and give menu for the person to choose
			can also appoint distance and time of travel
			use address
			nearest landmark (metro station)
			ek aur table services
		*/
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

	void display(int movie_id) {
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
			int line_index = 0; // To keep track of line indices
			while (getline(file, line)) {
				if (line_index % 3 == 1 && row < 12) { // Check if this line contains occupancy data
					for (int col = 0; col < 12; ++col) {
						char occupancy_char = line[col * 3 + 1]; // The occupancy character is at every third position starting from index 1
						if (occupancy_char == 'x') {
							seats_array[row][col] = 'x';
						}
						else {
							seats_array[row][col] = ' ';
						}
					}
					++row;
				}
				++line_index;
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
						outfile << setw(1) << seat;
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
		try {
			cout << "Enter number of seats you want to choose:";
			cin >> quantity;
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
				cout << "Enter seats(in format like A6, B7, D10 etc):" << endl;;
				cout << "SEAT " << i + 1 << ":";
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
						ofstream outfile(filename, ofstream::trunc);
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
						cout << "Seat " << seat << " is already picked. Please choose another seat." << endl;
						continue;

					}
				}
				else {
					cout << "ENTER A VALID SEAT, ITS OUT OF THEATRE BOUNDS!!" << endl;
				}

			}
		}
		catch (invalid_argument& e) {
			cout << "Error!!" << e.what() << endl;
		}

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
	}

	string getType() const override
	{
		return "Movie";
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
	void displayAllItems(vector <Movie*> movieItems, vector <TVSeries*> tvseriesItems)
	{
		cout << "List of titles of all Media Items:" << endl << endl;
		for (int i = 0; i < movieItems.size(); i++)
		{
			cout << i + 1 << ". ";
			movieItems.at(i)->getTitle();
			cout << endl;
		}
		for (int i = 0; i < tvseriesItems.size(); i++)
		{
			cout << i + 1 << ". ";
			tvseriesItems.at(i)->getTitle();
			cout << endl;
		}
		cout << "Do you wish to see the details of the selected Media Items?" << endl;
		cout << "1. Display Details" << endl << "2. Return to Main Menu" << endl;
		int ch1;
	choice:
		cout << "Enter your choice: ";
		cin >> ch1;
		if (ch1 == 1)
		{
			for (int i = 0; i < movieItems.size(); i++)
			{
				cout << i + 1 << ". " << endl;
				movieItems.at(i)->displayDetails();
				cout << endl;
			}
			for (int i = 0; i < tvseriesItems.size(); i++)
			{
				cout << i + 1 << ". " << endl;
				tvseriesItems.at(i)->displayDetails();
				cout << endl;
			}
		}
		else if (ch1 == 2)
		{
			return;
		}
		else
		{
			throw ch1;
		}
	}

	// 2. Function to display all movies or tv series
	void displayMoviesorSeries(vector <Movie*> movieItems, vector <TVSeries*> tvseriesItems)
	{
		cout << endl << "Do you wish to watch a MOVIE or a TV SERIES?" << endl;
		int ch2;
		cout << "1. MOVIE" << endl << "2. TV SERIES" << endl;
		cout << "Enter Your choice: ";
		cin >> ch2;
		if (ch2 == 1)
		{
			for (int i = 0; i < movieItems.size(); i++)
			{
				cout << "Recommendation " << i + 1 << ": " << endl;
				movieItems.at(i)->getTitle();
				cout << endl;
			}
		}
		else if (ch2 == 2)
		{
			for (int i = 0; i < tvseriesItems.size(); i++)
			{
				cout << "Recommendation " << i + 1 << ": " << endl;
				tvseriesItems.at(i)->getTitle();
				cout << endl;
			}
		}
		cout << "Do you wish to see the details of the selected Media Items?" << endl;
		cout << "1. Display Details" << endl << "2. Return to Main Menu" << endl;
		int ch3;
		cout << "Enter your choice: ";
		cin >> ch3;
		if (ch3 == 1)
		{
			if (ch2 == 1)
			{
				for (int i = 0; i < movieItems.size(); i++)
				{
					cout << "MOVIE " << i + 1 << ": " << endl;
					movieItems.at(i)->displayDetails();
					cout << endl;
				}
			}
			if (ch2 == 2)
			{
				for (int i = 0; i < tvseriesItems.size(); i++)
				{
					cout << "TV SERIES " << i + 1 << ": " << endl;
					tvseriesItems.at(i)->displayDetails();
					cout << endl;
				}
			}
		}
		else if (ch3 == 2)
		{
			return;
		}
		else
		{
			throw ch3;
		}
	}

	// 4. Function to search for a specific media item by title
	void searchByTitle(vector <Movie*> movieItems, vector <TVSeries*> tvseriesItems)
	{
		string t;
		cout << "Enter the name of the Media Item you want to search (in capital letters): ";
		cin.ignore();
		getline(cin, t);
		int count = 0;
		for (const auto& item : movieItems)
		{
			if (item->get_t() == t)
			{
				item->displayDetails();
				count++;
			}
		}
		for (const auto& item : tvseriesItems)
		{
			if (item->get_t() == t)
			{
				item->displayDetails();
				count++;
			}
		}
		if (count == 1)
		{
			return;
		}
		else
		{
			cout << "No Media Item found with title \"" << t << "\"" << endl;
		}
	}

	// 9. Function to make wishlist
	void wishlist(vector <Movie*> movieItems, vector <TVSeries*> tvseriesItems)
	{
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
			wishlist.push_back(title);
			ofstream outputFile("cinebliss_wishlist.txt", ios::app);
			if (!outputFile.is_open())
			{
				cerr << "Error: Unable to open file for writing." << endl;
				return;
			}
			outputFile << title << endl;
			outputFile.close();
			cout << "Added \"" << title << "\" to wishlist." << endl;
		}
		else if (ch5 == 2)
		{
			bool found = false;
			for (auto it = wishlist.begin(); it != wishlist.end(); ++it)
			{
				if (*it == title)
				{
					wishlist.erase(it);
					found = true;
					break;
				}
			}
			if (!found)
			{
				cout << "Item \"" << title << "\" not found in wishlist." << endl;
				return;
			}
		}
		else if (ch5 == 3)
		{
			ofstream outputFile("cinebliss_wishlist.txt");
			if (!outputFile.is_open())
			{
				cerr << "Error: Unable to open file for writing." << endl;
				return;
			}
			for (const auto& item : wishlist)
			{
				outputFile << item << endl;
			}
			outputFile.close();
		}
		else if (ch5 == 4)
		{
			cout << "Details of Wishlist Items:" << endl;
			for (const auto& item : wishlist)
			{
				for (int i = 0; i < movieItems.size(); i++)
				{
					if (item == movieItems.at(i)->get_t())
					{
						movieItems.at(i)->displayDetails();
					}
				}
			}
		}
		else
		{
			throw 'a';
		}
	}

	//3. Function to filter the media items
	//FOR DAKSH ---> yahan pe input ke liye cin >> ka use kia hai instead of getline... agar input error hoga to check kar liyo... mera to nhi aa rha tha
	void filter(vector <Movie*> movieItems, vector <TVSeries*> tvseriesItems)
	{
		cout << "FILTER MENU" << endl;
		cout << "1. By Genre" << endl;
		cout << "2. By Streaming Platform" << endl;
		cout << "3. By Language" << endl;
		cout << "4. By IMDb Rating" << endl;
		int ch6;
		cout << "Enter your choice: ";
		cin >> ch6;
		if (ch6 == 1)
		{
			cout << "GENRE MENU" << endl;
			cout << "1. Action" << endl;
			cout << "2. Comedy" << endl;
			cout << "3. Drama" << endl;
			cout << "4. Romance" << endl;
			cout << "5. Thriller" << endl;
			cout << "6. Horror" << endl;
			cout << "7. Historical" << endl;
			string g, s;
			cout << "Enter your choice for Genre: ";
			cin >> g;
			cout << "Enter your choice for Subgenre: ";
			cin >> s;
			cout << "MOVIES:" << endl;
			for (const auto& movie : movieItems)
			{
				if (movie->get_g() == g && movie->get_s() == s)
				{
					movie->displayDetails();
					cout << endl;
				}
			}
			cout << "TV SERIES:" << endl;
			for (const auto& tv : tvseriesItems)
			{
				if (tv->get_g() == g && tv->get_s() == s)
				{
					tv->displayDetails();
					cout << endl;
				}
			}
		}
		else if (ch6 == 2)
		{
			cout << "STREAMING PLATFORM MENU" << endl;
			cout << "1. Netflix" << endl;
			cout << "2. Amazon Prime" << endl;
			cout << "3. Disney+ Hotstar" << endl;
			cout << "4. SonyLiv" << endl;
			cout << "5. ZEE5" << endl;
			string sp;
			cout << "Enter your choice for Streaming Platform: ";
			cin >> sp;
			cout << "MOVIES:" << endl;
			for (const auto& movie : movieItems)
			{
				if (movie->getStreamingPlatform() == sp)
				{
					movie->displayDetails();
					cout << endl;
				}
			}
			cout << "TV SERIES:" << endl;
			for (const auto& tv : tvseriesItems)
			{
				if (tv->getStreamingPlatform() == sp)
				{
					tv->displayDetails();
					cout << endl;
				}
			}
		}
		else if (ch6 == 3)
		{
			cout << "LANGUAGE MENU" << endl;
			cout << "1. English" << endl;
			cout << "2. Hindi" << endl;
			cout << "3. Korean" << endl;
			cout << "4. Chinese" << endl;
			string l;
			cout << "Enter your choice for Language: ";
			cin >> l;
			cout << "MOVIES:" << endl;
			for (const auto& movie : movieItems)
			{
				if (movie->getLanguage() == l)
				{
					movie->displayDetails();
					cout << endl;
				}
			}
			cout << "TV SERIES:" << endl;
			for (const auto& tv : tvseriesItems)
			{
				if (tv->getLanguage() == l)
				{
					tv->displayDetails();
					cout << endl;
				}
			}
		}
		else if (ch6 == 4)
		{
			cout << "IMDB RATING MENU" << endl;
			cout << "1. Greater than 9" << endl;
			cout << "2. Greater than 8" << endl;
			cout << "3. Greater than 7" << endl;
			cout << "4. Greater than 6" << endl;
			cout << "5. Greater than 5" << endl;
			double i;
			cout << "Enter your choice for IMDb Rating (9, 8, 7, 6, 5): ";
			cin >> i;
			cout << "MOVIES:" << endl;
			for (const auto& movie : movieItems)
			{
				if (movie->getimdbRating() >= i)
				{
					movie->displayDetails();
					cout << endl;
				}
			}
			cout << "TV SERIES:" << endl;
			for (const auto& tv : tvseriesItems)
			{
				if (tv->getimdbRating() >= i)
				{
					tv->displayDetails();
					cout << endl;
				}
			}
		}
	}

	// 6. Function to recommend a random movie
	void recommendRandom(vector <Movie*> movieItems, vector <TVSeries*> tvseriesItems)
	{
		cout << "MOVIE OF THE DAY: " << endl;
		if (movieItems.empty())
		{
			cout << "No media items available for recommendation." << endl;
			return;
		}

		random_device rd;
		mt19937 gen(rd());
		uniform_int_distribution<int> dist(0, movieItems.size() - 1);

		int randomIndex = dist(gen);
		movieItems[randomIndex]->displayDetails();
	}

	// 7. Function to recommend a random tv series
	void recommendRandomtv(vector <Movie*> movieItems, vector <TVSeries*> tvseriesItems)
	{
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
		movieItems[randomIndex]->displayDetails();
	}
};

class CafeRecommendationSystem {
private:
	MYSQL* conn;

public:
	CafeRecommendationSystem() {
		conn = mysql_init(0);
		conn = mysql_real_connect(conn, "localhost", "root", "Daksh@2705", "booking", 3306, NULL, 0);
	}

	~CafeRecommendationSystem() {
		if (conn) {
			mysql_close(conn);
		}
	}

	void recommendCafes(int budget) {
		string query = "SELECT sno, cafe_name, price_per_head FROM cafes WHERE price_per_head <= " + to_string(budget);

		if (mysql_query(conn, query.c_str()) == 0) {
			MYSQL_RES* res = mysql_store_result(conn);
			if (res) {
				cout << "Cafe Recommendations within your budget:" << endl;
				cout << "+-----+---------------------------+-------------------+" << endl;
				cout << "| SNo | Cafe Name                 | Price Per Head (₹) |" << endl;
				cout << "+-----+---------------------------+-------------------+" << endl;

				MYSQL_ROW row;
				while ((row = mysql_fetch_row(res))) {
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

	void bookCafe(int sno, int numPeople) {
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
				cout << "Total Price: ₹" << totalPrice << endl;

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
		Login login(move(db_log));
		BookingManager bookingManager(move(db));
		string username, password;
		int choice;
	b:
		cout << "WELCOME TO CINEBLISS, AN EXPERT LOCATION TO HANG OUT WITH FRIENDS OR FAMILY." << endl
			<< endl;
		do
		{
			cout << "\nMain Menu\n";
			cout << "\t1. Sign Up\n";
			cout << "\t2. Login\n";
			cout << "\t3. Exit\n";
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
				a:
					cout << "\n\t\tENTER YOUR DETAILS BEFORE WE CONTINUE!!" << endl;
					bookingManager.getDetails();
					// code for choosing between cafe booking & movie booking.
					cout << "\n\nWhat would you like to do?" << endl
						<< endl;
					cout << "\t1. MOVIE / TV SERIES " << endl;
					cout << "\t2. CAFE " << endl;
					cout << "\t3. LOG OUT " << endl;
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
						cout << "\n WHAT WOULD YOU LIKE TO DO?" << endl;
						cout << "\t1. BOOK A MOVIE." << endl;
						cout << "\t2. GET MOVIE / TV RECOMMENDATIONS." << endl;
						cout << "\t3. CHECK YOUR TICKETS." << endl;
						cout << "\t4. Exit" << endl;
						int choice2;
						while (true)
						{
							cout << "\nENTER CHOICE:";
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
						if (choice2 == 1)
						{
							bookingManager.bookTickets();
							goto c;
						}
						else if (choice2 == 2)
						{
							/* // Writing data into the file
	 ofstream outFile;
	 outFile.open("mediaitem_data.txt", ios::out);
	 if (outFile.is_open())
	 {
		 outFile << "1, Movie, Inception, Action, Sci - Fi, Netflix, English, 8.8, Leonardo DiCaprio | Joseph Gordon - Levitt, 2010 - 07 - 16, 148, A thief who steals corporate secrets through the use of dream - sharing technology is given the inverse task of planting an idea into the mind of a C.E.O. but his tragic past may doom the project and his team to disaster., It focuses on the emotional journey of its lead character Cobb but at the same time thrusts the audience into multiple levels of action packed story-telling very distinct from one another but all finely connected." << endl;
		 outFile << "2, Movie, The Godfather,Thriller, Drama, Amazon Prime, English, 9.2, Marlon Brando | Al Pacino, 1972-03-24, 175, The film follows Vito Corleone the head of an Italian-American mafioso family between 1945 and 1955 a peroid of change after the end of WWII. The Godfather is the ultimate crime film about loyalty revenge and masculunity in the post-war era., The Godfather is one of Hollywood's greatest critical and commercial successes that gets everything right; a gangster flick that is overflowing with life rich with emotion and subtle acting and further blessed with amazing direction." << endl;
		 outFile << "3, TVSeries, Breaking Bad, Drama, Thriller, Netflix, English, 9.5, Bryan Cranston, Aaron Paul, 2008 - 01 - 20, 62, A chemistry teacher diagnosed with inoperable lung cancer turns to manufacturing and selling methamphetamine with a former student in order to secure his family's future., Breaking Bad is every bit as good as everyone says it is. It's easily one of my favorites shows of all-time and one of the best shows in the history of television." << endl;
		 outFile << "4, TVSeries, Friends, Comedy, Romance, Netflix, English, 8.9, Jennifer Aniston | Courteney Cox, 1994 - 09 - 22, 236, Comedy series about a tight-knit group of friends living in Manhattan. It also covers their interactions with their families their lovers and various colleagues over a period of several years., I absolutely love it; it always gets me super emotional when I rewatch Friends. The reason why I like Friends is that it honestly perfected the sitcom style." << endl;
		 outFile << "5, Movie, Extraction, Action, Thriller, Netflix, English, 6.7, Chris Hemsworth | Rudhraksh Jaiswal | Randeep Hooda, 2020-04-24, 117, A black ops mercenary embarks on a dangerous mission to rescue a kidnapped boy in Dhaka Bangladesh., Extraction delivers adrenaline-pumping action sequences with Chris Hemsworth's compelling performance making it an engaging watch despite its predictable plot." << endl;
	 }
	 else
	 {
		 cerr << "Unable to open file." << endl;
	 }

	 // Assigning data to the objects
	 ifstream inputFile("mediaitem_data.txt");
	 if (!inputFile.is_open())
	 {
		 cerr << "Error: Unable to open file." << endl;
		 return 1;
	 }

	 vector <Movie*> movieItems;
	 vector <TVSeries*> tvseriesItems;

	 string line;
	 while (getline(inputFile, line))
	 {
		 istringstream iss(line);
		 int code;
		 string type;
		 string title;
		 string genre;
		 string subgenre;
		 string language;
		 double imdbRating;
		 string streamingPlatform;
		 vector<string> cast;
		 string releaseDate;
		 int durationOrEpisodes;
		 string synopsis;
		 string review;

		 iss >> code;
		 iss.ignore();
		 getline(iss, type, ',');
		 getline(iss, title, ',');
		 getline(iss, genre, ',');
		 getline(iss, subgenre, ',');
		 getline(iss, streamingPlatform, ',');
		 getline(iss, language, ',');
		 iss >> imdbRating;
		 iss.ignore();
		 string castList;
		 getline(iss, castList, ',');
		 stringstream castStream(castList);
		 string actor;
		 while (getline(castStream, actor, '|'))
		 {
			 cast.push_back(actor);
		 }
		 getline(iss, releaseDate, ',');
		 iss >> durationOrEpisodes;
		 iss.ignore();
		 getline(iss, synopsis, ',');
		 getline(iss, review, ',');
		 iss.ignore();

		 // Create a media item based on the type
		 if (type == "Movie")
		 {
			 Movie* movie = new Movie (code, title, genre, subgenre, streamingPlatform, language, imdbRating,
				 type, cast, releaseDate, durationOrEpisodes, synopsis, review);
			 movieItems.push_back(movie);
		 }
		 else if (type == "TVSeries")
		 {
			 TVSeries* series = new TVSeries (code, title, genre, subgenre, streamingPlatform, language, imdbRating,
				 type, cast, releaseDate, durationOrEpisodes, synopsis, review);
			 tvseriesItems.push_back(series);
		 }
	 }

	 inputFile.close();

	 for (int i = 0; i < movieItems.size(); i++)
	 {
		 movieItems.at(i)->displayDetails();
	 }*/

	 //Data Initialization
							vector <Movie*> movieItems;
							vector <TVSeries*> tvseriesItems;

							Movie* movie1 = new Movie(1, "INCEPTION", "Action", "Sci-Fi", "Netflix", "English", 8.8,
								"Movie", { "Leonardo DiCaprio", "Joseph Gordon-Levitt" }, "2010-07-16", 148,
								"A thief who steals corporate secrets through the use of dream-sharing technology is given the inverse task of planting an idea into the mind of a CEO. But his tragic past may doom the project and his team to disaster.",
								"It focuses on the emotional journey of its lead character Cobb but at the same time thrusts the audience into multiple levels of action-packed storytelling very distinct from one another but all finely connected.");
							movieItems.push_back(movie1);

							Movie* movie2 = new Movie(2, "EXTRACTION", "Action", "Thriller", "Netflix", "English", 6.7,
								"Movie", { "Chris Hemsworth", "Rudhraksh Jaiswal", "Randeep Hooda" }, "2020-04-24", 117,
								"A black ops mercenary embarks on a dangerous mission to rescue a kidnapped boy in Dhaka, Bangladesh.",
								"Extraction delivers adrenaline-pumping action sequences with Chris Hemsworth''s compelling performance, making it an engaging watch despite its predictable plot.");
							movieItems.push_back(movie2);

							TVSeries* series1 = new TVSeries(3, "BREAKING BAD", "Drama", "Thriller", "Netflix", "English", 9.5,
								"TV Series", { "Bryan Cranston", "Aaron Paul" }, "2008-01-20", 62,
								"A chemistry teacher diagnosed with inoperable lung cancer turns to manufacturing and selling methamphetamine with a former student in order to secure his family's future.",
								"Breaking Bad is every bit as good as everyone says it is. It's easily one of my favorite shows of all time and one of the best shows in the history of television.");
							tvseriesItems.push_back(series1);

							//Object of Recommender Class
							Recommender rec;

							try
							{
								// Output Format
								cout << "WELCOME TO THE CINEBLISS RECOMMENDATION SYSTEM!!!" << endl;
								cout << "We are glad to assist you to find your PERFECT WATCH!!!" << endl;
								int time; // yeh woh waala time hai jo starting me manga tha ekdam budget ke saath
								cout << "Enter Time: "; //remove this statement during compilation
								cin >> time;
								if (time < 4 && time >= 1) // starting time should be in hours
								{
									cout << "According to the time constraints we suggest you should go for a movie!" << endl;
									cout << "Please select a suitable option from the MAIN MENU" << endl;
								}
								else if (time >= 4)
								{
									cout << "You have a lot of free time in your hands." << endl;
									cout << "We assure you a pleasant watching experince!!!" << endl;
									cout << "Please select a suitable option from the MAIN MENU" << endl;
								}

								//Main Menu
							mainmenu:
								cout << "MAIN MENU" << endl;
								cout << "1. Display a list of all Media Items" << endl; //DONE
								cout << "2. Display a list of either MOVIES or TV SERIES (as per choice)" << endl; //DONE
								cout << "3. Filter Media Items" << endl; //
								cout << "4. Search Media Items" << endl; //DONE
								cout << "5. MOVIE of the Day" << endl;
								cout << "6. TV SERIES of the Day" << endl;
								cout << "7. Wishlist" << endl; // WRITTEN BUT NO OUTPUT
								cout << "8. Exit" << endl; //DONE
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
									throw 1.0;
								}
							}

							//Exception Handling ---> Catch Blocks
							catch (int x)
							{
								cout << "INVALID INPUT!!!" << endl;
								cout << "Please enter either '1' or '2'" << endl;
							}
							catch (double y)
							{
								cout << "INVALID INPUT!!!" << endl;
								cout << "Please enter a number between '1' and '10'" << endl;
							}
							catch (char z)
							{
								cout << "INVALID INPUT!!!" << endl;
								cout << "Please enter a number between '1' and '4'" << endl;
							}

							/* // Freeing allocated memory
							for (Movie* movie : movieItems)
							{
								delete movie;
							}
							for (TVSeries* series : tvseriesItems)
							{
								delete series;
							}

							// Clearing vectors
							movieItems.clear();
							tvseriesItems.clear();*/

							goto c;
						}
						else if (choice2 == 3)
						{
							goto c;
						}
						else if (choice2 == 4)
						{
							goto a; // start of cinebliss
						}
					}

					else if (choice1 == 2)
					{
						CafeRecommendationSystem system;

						int budget;
						cout << "Enter your budget (in ₹): ";
						cin >> budget;

						system.recommendCafes(budget);

						int choice, numPeople;
						cout << "Enter the serial number of the cafe you want to book: ";
						cin >> choice;
						cout << "Enter the number of people coming: ";
						cin >> numPeople;

						system.bookCafe(choice, numPeople);
					}
					else if (choice1 == 3)
					{
						goto a; // start of page
					}
					else
					{
						cout << "Enter valid option.";
						goto b;
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

SQL QUERIES

CREATE DATABASE booking;
USE booking;
CREATE TABLE users(
	id INT AUTO_INCREMENT PRIMARY KEY,
	username VARCHAR(50) UNIQUE NOT NULL,
	password VARCHAR(50) NOT NULL
);

CREATE TABLE movies(
	sno INT PRIMARY KEY,
	movie_name VARCHAR(100),
	language VARCHAR(50),
	cbfc_rating VARCHAR(10),
	price integer,
	length integer
);

INSERT INTO movies(sno, movie_name, language, cbfc_rating, price, length)
VALUES
(1, 'Dangal', 'Hindi', 'U', 429, 172),
(2, '23 Idiots', 'Hindi', 'U', 313, 70),
(3, 'DDLJ', 'Hindi', 'U', 293, 94),
(4, 'PK', '', 'U/A', 444, 68),
(5, 'Lagaan', 'Hindi', 'U/A', 352, 140),
(6, 'Sholay', 'Hindi', 'U/A', 473, 99),
(7, 'Inception', 'English', 'PG-13', 475, 131),
(8, 'The Dark Knight', 'English', 'PG-13', 265, 97),
(9, 'Pulp Fiction', 'English', 'R', 465, 118),
(10, 'Interstellar', 'English', 'PG-13', 436, 116),
(11, 'Avengers: Endgame', 'English', 'PG-13', 497, 124),
(12, 'Jatt & Juliet', '', 'U/A', 218, 74),
(13, 'Sardar Ji', '', 'U', 325, 146),
(14, 'Manje Bistre', 'Punjabi', 'U/A', 415, 177),
(15, 'Carry On Jatta', 'Punjabi', 'U/A', 408, 128),
(16, 'Shadaa', 'Punjabi', 'U/A', 218, 74),
(17, 'Jab We Met', 'Hindi', 'U/A', 302, 127),
(18, 'Bhaag Milkha Bhaag', 'Hindi', 'U', 426, 71),
(19, 'Kesari', 'Hindi', 'U/A', 268, 162),
(20, 'Queen', 'Hindi', 'U', 362, 80);



UPDATE movies
SET
price = FLOOR(RAND() * (500 - 200 + 1)) + 200, --Random price between 200 and 500
length = FLOOR(RAND() * (180 - 60 + 1)) + 60 --Random length between 60 and 180 minutes
WHERE
sno <= 20;


CREATE TABLE Bookings(
	username VARCHAR(255),
	state VARCHAR(255),
	city VARCHAR(255),
	film VARCHAR(255),
	price VARCHAR(255),
	time VARCHAR(255),
	hall VARCHAR(255),
	quantity INT,
	seats VARCHAR(255), --Changed to VARCHAR
	language VARCHAR(255),
	date DATE,
	startTime TIME,
	endTime TIME,
	budget DECIMAL(10, 2)
);

--Create the theaters table
CREATE TABLE theaters(
	sno INT NOT NULL PRIMARY KEY,
	theater_name VARCHAR(100),
	place VARCHAR(100),
	landmark CHAR(1)
);

--Insert sample data into the theaters table
INSERT INTO theaters(sno, theater_name, place, landmark) VALUES
(1, 'PVR Cinemas', 'DLF Promenade, Vasant Kunj', 'A'),
(2, 'INOX Cinemas', 'Ambience Mall, Vasant Kunj', 'A'),
(3, 'Carnival Cinemas', 'DLF City Centre, Gurgaon', 'B'),
(4, 'Wave Cinemas', 'Wave Mall, Noida', 'C'),
(5, 'DT Cinemas', 'DLF Mall of India, Noida', 'C'),
(6, 'PVR Cinemas', 'Pacific Mall, Tagore Garden', 'D'),
(7, 'Cinepolis Cinemas', 'DLF Cyber Hub, Gurgaon', 'B'),
(8, 'INOX Cinemas', 'Shipra Mall, Indirapuram', 'E'),
(9, 'Wave Cinemas', 'Wave Mall, Kaushambi', 'F'),
(10, 'PVR Cinemas', 'DLF Emporio, Vasant Kunj', 'A'),
(11, 'Carnival Cinemas', 'Ansal Plaza, Khel Gaon Marg', 'G'),
(12, 'INOX Cinemas', 'Logix City Center Mall, Noida', 'C'),
(13, 'Cinepolis Cinemas', 'The Grand Venice Mall, Greater Noida', 'H'),
(14, 'PVR Cinemas', 'Ambience Mall, Gurgaon', 'B'),
(15, 'Wave Cinemas', 'Great India Place Mall, Noida', 'C');

--Create the cafes table
CREATE TABLE cafes(
	sno INT NOT NULL PRIMARY KEY,
	cafe_name VARCHAR(100),
	price_per_head INT
);

--Insert sample data into the cafes table
INSERT INTO cafes(sno, cafe_name, price_per_head) VALUES
(1, 'The Brew Room', 500),
(2, 'Blue Tokai Coffee Roasters', 400),
(3, 'Chaayos', 300);
