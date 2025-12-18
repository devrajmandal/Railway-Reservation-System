#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

class Database
{
private:
    sqlite3 *db;
    std::string dbPath;
    char *errMsg;

public:
    Database(const std::string &path) : db(nullptr), dbPath(path), errMsg(nullptr) {}

    ~Database()
    {
        disconnect();
    }

    bool connect()
    {
        int rc = sqlite3_open(dbPath.c_str(), &db);
        if (rc != SQLITE_OK)
        {
            std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
        std::cout << "Database connected successfully!" << std::endl;
        return true;
    }

    void disconnect()
    {
        if (db)
        {
            sqlite3_close(db);
            db = nullptr;
        }
    }

    bool executeQuery(const std::string &query)
    {
        int rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK)
        {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            return false;
        }
        return true;
    }

    bool createTables()
    {
        std::string createUsers = R"(
            CREATE TABLE IF NOT EXISTS users (
                user_id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT UNIQUE NOT NULL,
                password TEXT NOT NULL,
                email TEXT,
                phone TEXT,
                is_admin INTEGER DEFAULT 0
            );
        )";

        std::string createVehicles = R"(
            CREATE TABLE IF NOT EXISTS vehicles (
                vehicle_id INTEGER PRIMARY KEY AUTOINCREMENT,
                vehicle_type TEXT NOT NULL,
                vehicle_number TEXT UNIQUE NOT NULL,
                source TEXT NOT NULL,
                destination TEXT NOT NULL,
                departure_time TEXT NOT NULL,
                arrival_time TEXT NOT NULL,
                total_seats INTEGER NOT NULL,
                available_seats INTEGER NOT NULL,
                fare REAL NOT NULL
            );
        )";

        std::string createBookings = R"(
            CREATE TABLE IF NOT EXISTS bookings (
                booking_id INTEGER PRIMARY KEY AUTOINCREMENT,
                user_id INTEGER NOT NULL,
                vehicle_id INTEGER NOT NULL,
                booking_date TEXT NOT NULL,
                journey_date TEXT NOT NULL,
                num_seats INTEGER NOT NULL,
                total_amount REAL NOT NULL,
                status TEXT DEFAULT 'confirmed',
                FOREIGN KEY (user_id) REFERENCES users(user_id),
                FOREIGN KEY (vehicle_id) REFERENCES vehicles(vehicle_id)
            );
        )";

        return executeQuery(createUsers) &&
               executeQuery(createVehicles) &&
               executeQuery(createBookings);
    }

    bool insertUser(const std::string &username, const std::string &password,
                    const std::string &email, const std::string &phone, int isAdmin = 0)
    {
        std::string query = "INSERT INTO users (username, password, email, phone, is_admin) VALUES ('" +
                            username + "', '" + password + "', '" + email + "', '" + phone + "', " +
                            std::to_string(isAdmin) + ");";
        return executeQuery(query);
    }

    bool verifyLogin(const std::string &username, const std::string &password)
    {
        std::string query = "SELECT * FROM users WHERE username='" + username +
                            "' AND password='" + password + "';";

        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);

        if (rc != SQLITE_OK)
        {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }

        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        return (rc == SQLITE_ROW);
    }

    void displayVehicles()
    {
        std::string query = "SELECT * FROM vehicles WHERE available_seats > 0;";
        sqlite3_stmt *stmt;

        int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK)
        {
            std::cerr << "Failed to fetch vehicles" << std::endl;
            return;
        }

        std::cout << "\n=== Available Vehicles ===" << std::endl;
        std::cout << std::left; // Left align text
        std::cout << std::setw(5) << "ID"
                  << std::setw(10) << "Type"
                  << std::setw(12) << "Number"
                  << std::setw(15) << "Source"
                  << std::setw(15) << "Destination"
                  << std::setw(12) << "Departure"
                  << std::setw(10) << "Fare"
                  << std::setw(8) << "Seats" << std::endl;
        std::cout << std::string(87, '-') << std::endl;

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            std::cout << std::setw(5) << sqlite3_column_int(stmt, 0)
                      << std::setw(10) << sqlite3_column_text(stmt, 1)
                      << std::setw(12) << sqlite3_column_text(stmt, 2)
                      << std::setw(15) << sqlite3_column_text(stmt, 3)
                      << std::setw(15) << sqlite3_column_text(stmt, 4)
                      << std::setw(12) << sqlite3_column_text(stmt, 5)
                      << std::setw(10) << sqlite3_column_double(stmt, 9)
                      << std::setw(8) << sqlite3_column_int(stmt, 8) << std::endl;
        }

        sqlite3_finalize(stmt);
    }

    void insertSampleData()
    {
        executeQuery("INSERT OR IGNORE INTO vehicles VALUES (1, 'Train', 'TR001', 'Delhi', 'Mumbai', '08:00', '20:00', 100, 100, 1500.0);");
        executeQuery("INSERT OR IGNORE INTO vehicles VALUES (2, 'Bus', 'BS001', 'Delhi', 'Jaipur', '09:00', '14:00', 40, 40, 500.0);");
        executeQuery("INSERT OR IGNORE INTO vehicles VALUES (3, 'Train', 'TR002', 'Mumbai', 'Bangalore', '10:00', '22:00', 80, 80, 2000.0);");
        executeQuery("INSERT OR IGNORE INTO vehicles VALUES (4, 'Bus', 'BS002', 'Chennai', 'Hyderabad', '07:00', '15:00', 35, 35, 800.0);");

        insertUser("admin", "admin123", "admin@railway.com", "9999999999", 1);
    }

    sqlite3 *getDB() { return db; }
};

#endif
