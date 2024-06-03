# CineBliss

CineBliss is a C++ application designed to assist users in finding suitable entertainment options, either movies or cafes, based on their available time and budget constraints. With the ever-growing choices in both cinema and café culture, it can be overwhelming to decide where to spend time with friends. This system aims to simplify the decision-making process by providing personalized recommendations tailored to individual preferences.

## Features

### 1. User Registration and Authentication
- Users can create accounts securely and log in to access the features of the project.
- Passwords are hashed for security, and user authentication is ensured for every login attempt.

### 2. Main Menu
- Users can input their available time slot and budget for the outing.
- Additionally, users may specify preferences such as genre for movies or ambiance for cafes.

### 3. Data Retrieval
- The system fetches data from a pre-existing database containing information about movies currently playing in nearby theaters and cafes in the vicinity.
- Information includes movie titles, showtimes, genres, café names, locations, menus, and pricing.

### 4. Recommendation Algorithm
- Utilizes a recommendation algorithm to analyze user preferences and available options.
- Considers factors such as movie genres, café ambiance, distance from the user's location, and budget constraints.

### 5. Filtered Results
- Presents users with a filtered list of recommended movies and cafes that align with their specified criteria.
- Allows users to view additional details such as movie trailers, café reviews, and directions.

### 6. Online Streaming
- For users who prefer to watch films at home, the recommendation algorithm offers the ideal film or television show based on the user's preferences.

## Concepts Used

- Basics of C++
- File handling
- SQL
- Classes and objects
- Polymorphism
- Inheritance
- STL

## Included Header Files

```cpp
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
