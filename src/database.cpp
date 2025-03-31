#include "../include/database.h"
#include <iostream>

using namespace std;

// Callback function for sqlite3_exec to store the books in a vector
static int callback(void *data, int argc, char **argv, char **azColName)
{
    vector<string> *books = static_cast<vector<string> *>(data); // Casting data to vector<string>*

    if (argc > 0 && argv[0] != nullptr)
    {
        books->push_back(argv[0]); // Add the book name (first column) to the vector
    }

    return 0; // Return 0 to indicate success
}

std::vector<std::string> get_all_books(const std::string &db_path)
{
    sqlite3 *db;
    char *err_message = 0;
    vector<string> books;

    // Open the SQLite database
    if (sqlite3_open(db_path.c_str(), &db))
    {
        cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        return books; // Return empty vector on error
    }

    // SQL query to get all books (assuming the books are stored in a "books" table with a "name" column)
    const char *sql_query = "SELECT name FROM books;"; // Replace with your actual table and column names

    // Execute the query and store the results in the 'books' vector
    int rc = sqlite3_exec(db, sql_query, callback, &books, &err_message);
    if (rc != SQLITE_OK)
    {
        cerr << "SQL error: " << err_message << endl;
        sqlite3_free(err_message);
    }

    // Close the database
    sqlite3_close(db);

    return books; // Return the vector with all book names
}
