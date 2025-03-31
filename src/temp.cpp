// Bible Terminal Viewer with SQLite Database
// Filename: bible_viewer.cpp

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <sqlite3.h>
#include <cstring>
#include <ncurses.h>

// Structure to hold Bible verses
struct Verse
{
    int id;
    std::string book;
    int chapter;
    int verse;
    std::string text;
};

// Structure to hold Bible books
struct Book
{
    std::string name;
    int chapters;
};

class BibleViewer
{
private:
    sqlite3 *db;
    std::vector<Book> books;
    int currentBook = 0;
    int currentChapter = 1;
    int currentVerse = 1;
    int screenRows = 0;
    int screenCols = 0;

    // Initialize ncurses
    void initNcurses()
    {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLUE);   // Title
        init_pair(2, COLOR_YELLOW, COLOR_BLACK); // Highlights
        init_pair(3, COLOR_GREEN, COLOR_BLACK);  // Verse numbers
        curs_set(0);                             // Hide cursor
    }

    // Load books from the database
    void loadBooks()
    {
        books.clear();

        const char *query = "SELECT DISTINCT book, MAX(chapter) as chapters FROM bible GROUP BY book ORDER BY id";
        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
            return;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            Book book;
            book.name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            book.chapters = sqlite3_column_int(stmt, 1);
            books.push_back(book);
        }

        sqlite3_finalize(stmt);
    }

    // Get verses for a specific chapter
    std::vector<Verse> getChapterVerses(const std::string &book, int chapter)
    {
        std::vector<Verse> verses;

        std::string query = "SELECT id, book, chapter, verse, text FROM bible WHERE book = ? AND chapter = ? ORDER BY verse";
        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
            return verses;
        }

        sqlite3_bind_text(stmt, 1, book.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, chapter);

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            Verse verse;
            verse.id = sqlite3_column_int(stmt, 0);
            verse.book = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            verse.chapter = sqlite3_column_int(stmt, 2);
            verse.verse = sqlite3_column_int(stmt, 3);
            verse.text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
            verses.push_back(verse);
        }

        sqlite3_finalize(stmt);
        return verses;
    }

    // Search for verses containing a specific term
    std::vector<Verse> searchVerses(const std::string &term)
    {
        std::vector<Verse> results;

        std::string query = "SELECT id, book, chapter, verse, text FROM bible WHERE text LIKE ? ORDER BY id LIMIT 100";
        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
            return results;
        }

        std::string searchTerm = "%" + term + "%";
        sqlite3_bind_text(stmt, 1, searchTerm.c_str(), -1, SQLITE_STATIC);

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            Verse verse;
            verse.id = sqlite3_column_int(stmt, 0);
            verse.book = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            verse.chapter = sqlite3_column_int(stmt, 2);
            verse.verse = sqlite3_column_int(stmt, 3);
            verse.text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
            results.push_back(verse);
        }

        sqlite3_finalize(stmt);
        return results;
    }

    // Display the current chapter
    void displayChapter()
    {
        clear();
        getmaxyx(stdscr, screenRows, screenCols);

        // Display title
        attron(COLOR_PAIR(1));
        std::string title = books[currentBook].name + " Chapter " + std::to_string(currentChapter);
        mvprintw(0, (screenCols - title.length()) / 2, "%s", title.c_str());
        mvhline(1, 0, ACS_HLINE, screenCols);
        attroff(COLOR_PAIR(1));

        // Get and display verses
        std::vector<Verse> verses = getChapterVerses(books[currentBook].name, currentChapter);

        int row = 3;
        int scrollOffset = 0;

        // Calculate scroll offset to keep current verse visible
        if (currentVerse > 1)
        {
            for (size_t i = 0; i < verses.size() && i < static_cast<size_t>(currentVerse - 1); i++)
            {
                scrollOffset += (verses[i].text.length() / (screenCols - 10) + 1);
            }
            scrollOffset = std::max(0, scrollOffset - screenRows + 10);
        }

        // Display verses with word wrapping
        for (const auto &verse : verses)
        {
            std::string verseText = verse.text;

            // Highlight the current verse
            if (verse.verse == currentVerse)
            {
                attron(COLOR_PAIR(2));
            }

            // Display verse number
            attron(COLOR_PAIR(3));
            mvprintw(row - scrollOffset, 2, "%d", verse.verse);
            attroff(COLOR_PAIR(3));

            // Word wrap verse text
            int col = 6;
            for (size_t i = 0; i < verseText.length(); i++)
            {
                if (col >= screenCols - 2)
                {
                    row++;
                    col = 6;
                }

                if (row - scrollOffset >= 2 && row - scrollOffset < screenRows - 2)
                {
                    mvaddch(row - scrollOffset, col, verseText[i]);
                }
                col++;
            }

            if (verse.verse == currentVerse)
            {
                attroff(COLOR_PAIR(2));
            }

            row += 2;
        }

        // Display navigation help
        attron(COLOR_PAIR(1));
        mvhline(screenRows - 2, 0, ACS_HLINE, screenCols);
        mvprintw(screenRows - 1, 0, "↑/↓: Navigate verses | ←/→: Chapters | b: Book list | s: Search | q: Quit");
        attroff(COLOR_PAIR(1));

        refresh();
    }

    // Display the book selection menu
    void displayBookMenu()
    {
        clear();
        getmaxyx(stdscr, screenRows, screenCols);

        attron(COLOR_PAIR(1));
        std::string title = "Bible Book Selection";
        mvprintw(0, (screenCols - title.length()) / 2, "%s", title.c_str());
        mvhline(1, 0, ACS_HLINE, screenCols);
        attroff(COLOR_PAIR(1));

        int startRow = 3;
        int booksPerRow = 3;
        int columnWidth = screenCols / booksPerRow;

        for (size_t i = 0; i < books.size(); i++)
        {
            int row = startRow + (i / booksPerRow) * 2;
            int col = (i % booksPerRow) * columnWidth;

            if (i == static_cast<size_t>(currentBook))
            {
                attron(COLOR_PAIR(2));
            }

            mvprintw(row, col + 2, "%s (%d chapters)",
                     books[i].name.c_str(), books[i].chapters);

            if (i == static_cast<size_t>(currentBook))
            {
                attroff(COLOR_PAIR(2));
            }
        }

        attron(COLOR_PAIR(1));
        mvhline(screenRows - 2, 0, ACS_HLINE, screenCols);
        mvprintw(screenRows - 1, 0, "↑/↓/←/→: Navigate | Enter: Select | q: Quit");
        attroff(COLOR_PAIR(1));

        refresh();
    }

    // Display search interface
    void displaySearchInterface()
    {
        clear();
        getmaxyx(stdscr, screenRows, screenCols);

        attron(COLOR_PAIR(1));
        std::string title = "Bible Search";
        mvprintw(0, (screenCols - title.length()) / 2, "%s", title.c_str());
        mvhline(1, 0, ACS_HLINE, screenCols);
        attroff(COLOR_PAIR(1));

        mvprintw(3, 2, "Enter search term: ");
        echo();
        curs_set(1);

        char searchTerm[100];
        getstr(searchTerm);

        noecho();
        curs_set(0);

        // If search term is empty, return to chapter view
        if (strlen(searchTerm) == 0)
        {
            displayChapter();
            return;
        }

        // Search for verses
        std::vector<Verse> results = searchVerses(searchTerm);

        clear();
        attron(COLOR_PAIR(1));
        std::string resultTitle = "Search Results for: " + std::string(searchTerm);
        mvprintw(0, (screenCols - resultTitle.length()) / 2, "%s", resultTitle.c_str());
        mvhline(1, 0, ACS_HLINE, screenCols);
        attroff(COLOR_PAIR(1));

        if (results.empty())
        {
            mvprintw(3, 2, "No results found.");
        }
        else
        {
            mvprintw(2, 2, "Found %zu results:", results.size());

            int row = 4;
            for (const auto &verse : results)
            {
                if (row >= screenRows - 3)
                    break;

                attron(COLOR_PAIR(3));
                mvprintw(row, 2, "%s %d:%d", verse.book.c_str(), verse.chapter, verse.verse);
                attroff(COLOR_PAIR(3));

                // Truncate verse text if too long for display
                std::string displayText = verse.text;
                if (displayText.length() > static_cast<size_t>(screenCols - 4))
                {
                    displayText = displayText.substr(0, screenCols - 7) + "...";
                }

                mvprintw(row + 1, 4, "%s", displayText.c_str());
                row += 3;
            }
        }

        attron(COLOR_PAIR(1));
        mvhline(screenRows - 2, 0, ACS_HLINE, screenCols);
        mvprintw(screenRows - 1, 0, "Press any key to return");
        attroff(COLOR_PAIR(1));

        refresh();
        getch(); // Wait for key press
        displayChapter();
    }

public:
    BibleViewer() : db(nullptr) {}

    ~BibleViewer()
    {
        if (db)
        {
            sqlite3_close(db);
        }
        endwin(); // Clean up ncurses
    }

    // Initialize the database
    bool initDatabase(const std::string &dbPath)
    {
        if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK)
        {
            std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }

        // Check if the database has the required table
        sqlite3_stmt *stmt;
        const char *checkTableSQL = "SELECT name FROM sqlite_master WHERE type='table' AND name='bible'";

        if (sqlite3_prepare_v2(db, checkTableSQL, -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }

        bool tableExists = (sqlite3_step(stmt) == SQLITE_ROW);
        sqlite3_finalize(stmt);

        if (!tableExists)
        {
            std::cerr << "Database does not have a 'bible' table. Please use a properly formatted database." << std::endl;
            return false;
        }

        loadBooks();

        if (books.empty())
        {
            std::cerr << "No books found in the database." << std::endl;
            return false;
        }

        return true;
    }

    // Run the main interface
    void run()
    {
        if (!db || books.empty())
        {
            std::cerr << "Database not initialized correctly." << std::endl;
            return;
        }

        initNcurses();
        displayChapter();

        int ch;
        bool quitRequested = false;

        while (!quitRequested && (ch = getch()))
        {
            switch (ch)
            {
            case 'q':
            case 'Q':
                quitRequested = true;
                break;

            case KEY_UP:
                if (currentVerse > 1)
                {
                    currentVerse--;
                    displayChapter();
                }
                break;

            case KEY_DOWN:
            {
                std::vector<Verse> verses = getChapterVerses(books[currentBook].name, currentChapter);
                if (currentVerse < static_cast<int>(verses.size()))
                {
                    currentVerse++;
                    displayChapter();
                }
                break;
            }

            case KEY_LEFT:
                if (currentChapter > 1)
                {
                    currentChapter--;
                    currentVerse = 1;
                    displayChapter();
                }
                break;

            case KEY_RIGHT:
                if (currentChapter < books[currentBook].chapters)
                {
                    currentChapter++;
                    currentVerse = 1;
                    displayChapter();
                }
                break;

            case 'b':
            case 'B':
            {
                bool bookMenuActive = true;
                displayBookMenu();

                while (bookMenuActive)
                {
                    int bookCh = getch();

                    switch (bookCh)
                    {
                    case 'q':
                    case 'Q':
                        bookMenuActive = false;
                        quitRequested = true;
                        break;

                    case KEY_UP:
                        if (currentBook >= 3)
                        {
                            currentBook -= 3;
                            displayBookMenu();
                        }
                        break;

                    case KEY_DOWN:
                        if (currentBook + 3 < static_cast<int>(books.size()))
                        {
                            currentBook += 3;
                            displayBookMenu();
                        }
                        break;

                    case KEY_LEFT:
                        if (currentBook > 0)
                        {
                            currentBook--;
                            displayBookMenu();
                        }
                        break;

                    case KEY_RIGHT:
                        if (currentBook < static_cast<int>(books.size()) - 1)
                        {
                            currentBook++;
                            displayBookMenu();
                        }
                        break;

                    case '\n':
                    case KEY_ENTER:
                        bookMenuActive = false;
                        currentChapter = 1;
                        currentVerse = 1;
                        displayChapter();
                        break;
                    }
                }
                break;
            }

            case 's':
            case 'S':
                displaySearchInterface();
                break;
            }
        }
    }

    // Create database schema and import data (simplified example)
    static bool createDatabase(const std::string &dbPath)
    {
        sqlite3 *newDb;
        if (sqlite3_open(dbPath.c_str(), &newDb) != SQLITE_OK)
        {
            std::cerr << "Error creating database: " << sqlite3_errmsg(newDb) << std::endl;
            return false;
        }

        // Create table
        const char *createTableSQL =
            "CREATE TABLE IF NOT EXISTS bible ("
            "    id INTEGER PRIMARY KEY,"
            "    book TEXT NOT NULL,"
            "    chapter INTEGER NOT NULL,"
            "    verse INTEGER NOT NULL,"
            "    text TEXT NOT NULL"
            ");";

        char *errMsg = nullptr;
        if (sqlite3_exec(newDb, createTableSQL, nullptr, nullptr, &errMsg) != SQLITE_OK)
        {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            sqlite3_close(newDb);
            return false;
        }

        std::cout << "Bible database schema created successfully." << std::endl;
        std::cout << "You should now import Bible text data into this database." << std::endl;

        sqlite3_close(newDb);
        return true;
    }
};

// Utility to import Bible text from a CSV file
bool importBibleFromCSV(const std::string &dbPath, const std::string &csvPath)
{
    sqlite3 *db;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK)
    {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // Begin transaction for faster import
    char *errMsg = nullptr;
    if (sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }

    // Prepare insert statement
    const char *insertSQL = "INSERT INTO bible (book, chapter, verse, text) VALUES (?, ?, ?, ?)";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }

    // Open and read CSV file
    FILE *file = fopen(csvPath.c_str(), "r");
    if (!file)
    {
        std::cerr << "Error opening CSV file: " << csvPath << std::endl;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    // Simple CSV parser
    char line[10000]; // Assuming no line is longer than 10000 chars
    int count = 0;

    // Skip header line if present
    if (fgets(line, sizeof(line), file) == nullptr)
    {
        std::cerr << "Error reading CSV file or file is empty." << std::endl;
        fclose(file);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    // Process data lines
    while (fgets(line, sizeof(line), file))
    {
        char book[100];
        int chapter, verse;
        char text[9000];

        // Parse CSV line (very simplified - assumes proper formatting)
        if (sscanf(line, "\"%[^\"]\", %d, %d, \"%[^\"]\"", book, &chapter, &verse, text) == 4)
        {
            sqlite3_bind_text(stmt, 1, book, -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 2, chapter);
            sqlite3_bind_int(stmt, 3, verse);
            sqlite3_bind_text(stmt, 4, text, -1, SQLITE_STATIC);

            if (sqlite3_step(stmt) != SQLITE_DONE)
            {
                std::cerr << "Error inserting data: " << sqlite3_errmsg(db) << std::endl;
            }
            else
            {
                count++;
            }

            sqlite3_reset(stmt);
        }
    }

    fclose(file);
    sqlite3_finalize(stmt);

    // Commit transaction
    if (sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }

    std::cout << "Imported " << count << " verses successfully." << std::endl;
    sqlite3_close(db);
    return true;
}

void printUsage()
{
    std::cout << "Bible Terminal Viewer" << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "  bible_viewer view <database.db>" << std::endl;
    std::cout << "  bible_viewer create <database.db>" << std::endl;
    std::cout << "  bible_viewer import <database.db> <bible.csv>" << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printUsage();
        return 1;
    }

    std::string command = argv[1];
    std::string dbPath = argv[2];

    if (command == "view")
    {
        BibleViewer viewer;
        if (viewer.initDatabase(dbPath))
        {
            viewer.run();
        }
    }
    else if (command == "create")
    {
        if (BibleViewer::createDatabase(dbPath))
        {
            std::cout << "Database created successfully: " << dbPath << std::endl;
        }
    }
    else if (command == "import")
    {
        if (argc < 4)
        {
            std::cout << "Error: Missing CSV file path." << std::endl;
            printUsage();
            return 1;
        }

        std::string csvPath = argv[3];
        if (importBibleFromCSV(dbPath, csvPath))
        {
            std::cout << "Bible data imported successfully into: " << dbPath << std::endl;
        }
    }
    else
    {
        std::cout << "Unknown command: " << command << std::endl;
        printUsage();
        return 1;
    }

    return 0;
}