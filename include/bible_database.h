#ifndef BIBLE_DATABASE_H
#define BIBLE_DATABASE_H

#include <sqlite3.h>
#include <string>
#include <vector>

class BibleDatabase
{
private:
    sqlite3 *db; // Databse connection pointer

public:
    // Constructor: Open the database
    BibleDatabase(const std::string &db_name);

    // Get all books from the database
    std::vector<std::string> get_all_books();

    std::vector<std::string> get_old_testament_books();

    int get_book_id_by_name(std::string &bookName);

    // Destructor: Close the database connection
    ~BibleDatabase();
};

#endif