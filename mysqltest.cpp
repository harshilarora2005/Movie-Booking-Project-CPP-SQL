#include<mysql.h>
#include<iostream>
using namespace std;
int main() {
	MYSQL* conn;
	MYSQL_ROW row;
	MYSQL_RES* res;
	conn = mysql_init(0);
	conn = mysql_real_connect(conn, "localhost", "root", "", "testdb", 3306, NULL, 0);
	if (conn) {
		puts("SUCCESSFUL connection to database!");
	}
}