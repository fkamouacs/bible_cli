#include "../include/bible_database.h"
#include <iostream>

// Constructor
BibleDatabase::BibleDatabase(const std::string &db_name)
{
    if (sqlite3_open(db_name.c_str(), &db) != SQLITE_OK)
    {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    }
}

int BibleDatabase::get_book_id_by_name(std::string &bookName)
{
    int bookId = -1;

    if (!db)
        return bookId;

    std::string sql = "SELECT id FROM books WHERE name = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        // ✅ Bind book name as a string
        sqlite3_bind_text(stmt, 1, bookName.c_str(), -1, SQLITE_STATIC);

        // ✅ Execute query
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            bookId = sqlite3_column_int(stmt, 0); // Get the book ID
        }
    }
    else
    {
        std::cerr << "Error executing SQL: " << sqlite3_errmsg(db) << std::endl;
    }

    // ✅ Cleanup
    sqlite3_finalize(stmt);
    return bookId;
}

std::vector<std::string> BibleDatabase::get_all_books()
{
    std::vector<std::string> books;

    if (!db)
        return books;

    std::string sql = "SELECT name FROM books ORDER BY id";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            books.push_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)));
        }
    }
    else
    {
        std::cerr << "Error executing SQL: " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
    return books;
}

std::vector<std::string> BibleDatabase::get_old_testament_books()
{
    // std::vector<std::string> books;

    // if (!db)
    //     return books;

    std::string firstBookNewTestament = "Matthew";
    int bookOfMatthewId = BibleDatabase::get_book_id_by_name(firstBookNewTestament);

      std::vector<std::string> books;

    if (!db)
        return books;

    std::string sql = "SELECT name FROM books WHERE id < ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {

        sqlite3_bind_int(stmt, 1, bookOfMatthewId);
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            books.push_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)));
        }
    }
    else
    {
        std::cerr << "Error executing SQL: " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
    return books;
}

// Destructor: Close database connection
BibleDatabase::~BibleDatabase()
{
    if (db)
    {
        sqlite3_close(db);
        sqlite3_shutdown();
    }
}