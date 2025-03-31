#ifndef DB_H
#define DB_H

#include <iostream>
#include <sqlite3.h>
#include <vector>
#include <string>

using namespace std;

int fetch_books(void *data, int argc, char **argv, char **azColName);
std::vector<std::string> get_all_books(const string &db_path);
int get_chapter_count(const string &db_name, const string &book_name);
void display_verses(const string &db_name, const string &book_name, int chapter_number);

#endif // MENU_H