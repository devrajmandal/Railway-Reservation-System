#include <iostream>
#include <string>
#include <ctime>
#include "Database.h"

using namespace std;

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

string getCurrentDate() {
    time_t now = time(0);
    char* dt = ctime(&now);
    string date(dt);
    return date.substr(0, date.length() - 1);
}

class ReservationSystem {
private:
    Database db;
    string currentUser;
    int currentUserId;
    bool isAdmin;

public:
    ReservationSystem() : db("database/reservation.db"), currentUserId(-1), isAdmin(false) {
        if (db.connect()) {
            db.createTables();
            db.insertSampleData();
        }
    }

    void run() {
        while (true) {
            clearScreen();
            cout << "\n========================================" << endl;
            cout << "  RAILWAY/BUS RESERVATION SYSTEM" << endl;
            cout << "========================================" << endl;
            cout << "\n1. Register" << endl;
            cout << "2. Login" << endl;
            cout << "3. Exit" << endl;
            cout << "\nEnter your choice: ";

            int choice;
            cin >> choice;
            cin.ignore();

            switch (choice) {
                case 1:
                    registerUser();
                    break;
                case 2:
                    login();
                    break;
                case 3:
                    cout << "\nThank you for using our system!" << endl;
                    return;
                default:
                    cout << "\nInvalid choice!" << endl;
                    cin.get();
            }
        }
    }

private:
    void registerUser() {
        clearScreen();
        cout << "\n=== USER REGISTRATION ===" << endl;
        
        string username, password, email, phone;
        
        cout << "Enter username: ";
        getline(cin, username);
        
        cout << "Enter password: ";
        getline(cin, password);
        
        cout << "Enter email: ";
        getline(cin, email);
        
        cout << "Enter phone: ";
        getline(cin, phone);

        if (db.insertUser(username, password, email, phone)) {
            cout << "\nRegistration successful! You can now login." << endl;
        } else {
            cout << "\nRegistration failed! Username might already exist." << endl;
        }
        
        cout << "\nPress Enter to continue...";
        cin.get();
    }

    void login() {
        clearScreen();
        cout << "\n=== LOGIN ===" << endl;
        
        string username, password;
        
        cout << "Enter username: ";
        getline(cin, username);
        
        cout << "Enter password: ";
        getline(cin, password);

        if (db.verifyLogin(username, password)) {
            currentUser = username;
            
            // Check if admin
            string query = "SELECT user_id, is_admin FROM users WHERE username='" + username + "';";
            sqlite3_stmt* stmt;
            sqlite3_prepare_v2(db.getDB(), query.c_str(), -1, &stmt, nullptr);
            
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                currentUserId = sqlite3_column_int(stmt, 0);
                isAdmin = sqlite3_column_int(stmt, 1);
            }
            sqlite3_finalize(stmt);

            cout << "\nLogin successful! Welcome, " << username << "!" << endl;
            cout << "Press Enter to continue...";
            cin.get();
            
            if (isAdmin) {
                adminMenu();
            } else {
                userMenu();
            }
        } else {
            cout << "\nInvalid username or password!" << endl;
            cout << "Press Enter to continue...";
            cin.get();
        }
    }

    void userMenu() {
        while (true) {
            clearScreen();
            cout << "\n========================================" << endl;
            cout << "  USER MENU - Welcome " << currentUser << "!" << endl;
            cout << "========================================" << endl;
            cout << "\n1. View Available Vehicles" << endl;
            cout << "2. Book Ticket" << endl;
            cout << "3. View My Bookings" << endl;
            cout << "4. Cancel Booking" << endl;
            cout << "5. Logout" << endl;
            cout << "\nEnter your choice: ";

            int choice;
            cin >> choice;
            cin.ignore();

            switch (choice) {
                case 1:
                    viewVehicles();
                    break;
                case 2:
                    bookTicket();
                    break;
                case 3:
                    viewMyBookings();
                    break;
                case 4:
                    cancelBooking();
                    break;
                case 5:
                    currentUser = "";
                    currentUserId = -1;
                    return;
                default:
                    cout << "\nInvalid choice!" << endl;
                    cin.get();
            }
        }
    }

    void adminMenu() {
        while (true) {
            clearScreen();
            cout << "\n========================================" << endl;
            cout << "  ADMIN MENU - Welcome " << currentUser << "!" << endl;
            cout << "========================================" << endl;
            cout << "\n1. View All Vehicles" << endl;
            cout << "2. Add New Vehicle" << endl;
            cout << "3. View All Bookings" << endl;
            cout << "4. View Statistics" << endl;
            cout << "5. Logout" << endl;
            cout << "\nEnter your choice: ";

            int choice;
            cin >> choice;
            cin.ignore();

            switch (choice) {
                case 1:
                    viewVehicles();
                    break;
                case 2:
                    addVehicle();
                    break;
                case 3:
                    viewAllBookings();
                    break;
                case 4:
                    viewStatistics();
                    break;
                case 5:
                    currentUser = "";
                    currentUserId = -1;
                    isAdmin = false;
                    return;
                default:
                    cout << "\nInvalid choice!" << endl;
                    cin.get();
            }
        }
    }

    void viewVehicles() {
        clearScreen();
        db.displayVehicles();
        cout << "\nPress Enter to continue...";
        cin.get();
    }

    void bookTicket() {
        clearScreen();
        cout << "\n=== BOOK TICKET ===" << endl;
        db.displayVehicles();
        
        int vehicleId, numSeats;
        cout << "\nEnter Vehicle ID: ";
        cin >> vehicleId;
        cout << "Enter number of seats: ";
        cin >> numSeats;
        cin.ignore();

        // Check available seats
        string query = "SELECT available_seats, fare FROM vehicles WHERE vehicle_id=" + to_string(vehicleId) + ";";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db.getDB(), query.c_str(), -1, &stmt, nullptr);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int availableSeats = sqlite3_column_int(stmt, 0);
            double fare = sqlite3_column_double(stmt, 1);
            
            if (availableSeats >= numSeats) {
                double totalAmount = fare * numSeats;
                string bookingDate = getCurrentDate();
                
                // Insert booking
                string insertQuery = "INSERT INTO bookings (user_id, vehicle_id, booking_date, journey_date, num_seats, total_amount) VALUES (" +
                                    to_string(currentUserId) + ", " + to_string(vehicleId) + ", '" + bookingDate + 
                                    "', '" + bookingDate + "', " + to_string(numSeats) + ", " + to_string(totalAmount) + ");";
                
                if (db.executeQuery(insertQuery)) {
                    // Update available seats
                    string updateQuery = "UPDATE vehicles SET available_seats = available_seats - " + 
                                       to_string(numSeats) + " WHERE vehicle_id=" + to_string(vehicleId) + ";";
                    db.executeQuery(updateQuery);
                    
                    cout << "\n[SUCCESS] Booking successful!" << endl;
                    cout << "Total Amount: Rs. " << totalAmount << endl;
                    cout << "Booking Date: " << bookingDate << endl;
                } else {
                    cout << "\nBooking failed!" << endl;
                }
            } else {
                cout << "\nInsufficient seats available! Only " << availableSeats << " seats left." << endl;
            }
        } else {
            cout << "\nVehicle not found!" << endl;
        }
        
        sqlite3_finalize(stmt);
        cout << "\nPress Enter to continue...";
        cin.get();
    }

    void viewMyBookings() {
        clearScreen();
        cout << "\n=== MY BOOKINGS ===" << endl;
        
        string query = "SELECT b.booking_id, v.vehicle_type, v.vehicle_number, v.source, v.destination, "
                      "b.journey_date, b.num_seats, b.total_amount, b.status FROM bookings b "
                      "JOIN vehicles v ON b.vehicle_id = v.vehicle_id WHERE b.user_id=" + to_string(currentUserId) + ";";
        
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db.getDB(), query.c_str(), -1, &stmt, nullptr);

        cout << "ID\tType\tNumber\t\tRoute\t\t\tDate\t\tSeats\tAmount\tStatus" << endl;
        cout << "-------------------------------------------------------------------------" << endl;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            cout << sqlite3_column_int(stmt, 0) << "\t"
                 << sqlite3_column_text(stmt, 1) << "\t"
                 << sqlite3_column_text(stmt, 2) << "\t"
                 << sqlite3_column_text(stmt, 3) << "->" << sqlite3_column_text(stmt, 4) << "\t"
                 << sqlite3_column_text(stmt, 5) << "\t"
                 << sqlite3_column_int(stmt, 6) << "\t"
                 << sqlite3_column_double(stmt, 7) << "\t"
                 << sqlite3_column_text(stmt, 8) << endl;
        }

        sqlite3_finalize(stmt);
        cout << "\nPress Enter to continue...";
        cin.get();
    }

    void cancelBooking() {
        clearScreen();
        viewMyBookings();
        
        int bookingId;
        cout << "\nEnter Booking ID to cancel: ";
        cin >> bookingId;
        cin.ignore();

        // Get booking details
        string query = "SELECT vehicle_id, num_seats, status FROM bookings WHERE booking_id=" + 
                      to_string(bookingId) + " AND user_id=" + to_string(currentUserId) + ";";
        
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db.getDB(), query.c_str(), -1, &stmt, nullptr);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int vehicleId = sqlite3_column_int(stmt, 0);
            int numSeats = sqlite3_column_int(stmt, 1);
            string status = (const char*)sqlite3_column_text(stmt, 2);

            if (status == "cancelled") {
                cout << "\nThis booking is already cancelled!" << endl;
            } else {
                // Update booking status
                string updateBooking = "UPDATE bookings SET status='cancelled' WHERE booking_id=" + to_string(bookingId) + ";";
                
                // Return seats
                string updateSeats = "UPDATE vehicles SET available_seats = available_seats + " + 
                                   to_string(numSeats) + " WHERE vehicle_id=" + to_string(vehicleId) + ";";

                if (db.executeQuery(updateBooking) && db.executeQuery(updateSeats)) {
                    cout << "\n✓ Booking cancelled successfully!" << endl;
                } else {
                    cout << "\nCancellation failed!" << endl;
                }
            }
        } else {
            cout << "\nBooking not found!" << endl;
        }

        sqlite3_finalize(stmt);
        cout << "\nPress Enter to continue...";
        cin.get();
    }

    void addVehicle() {
        clearScreen();
        cout << "\n=== ADD NEW VEHICLE ===" << endl;
        
        string type, number, source, dest, depTime, arrTime;
        int totalSeats;
        double fare;

        cout << "Vehicle Type (Train/Bus): ";
        getline(cin, type);
        cout << "Vehicle Number: ";
        getline(cin, number);
        cout << "Source: ";
        getline(cin, source);
        cout << "Destination: ";
        getline(cin, dest);
        cout << "Departure Time (HH:MM): ";
        getline(cin, depTime);
        cout << "Arrival Time (HH:MM): ";
        getline(cin, arrTime);
        cout << "Total Seats: ";
        cin >> totalSeats;
        cout << "Fare: ";
        cin >> fare;
        cin.ignore();

        string query = "INSERT INTO vehicles (vehicle_type, vehicle_number, source, destination, "
                      "departure_time, arrival_time, total_seats, available_seats, fare) VALUES ('" +
                      type + "', '" + number + "', '" + source + "', '" + dest + "', '" + 
                      depTime + "', '" + arrTime + "', " + to_string(totalSeats) + ", " + 
                      to_string(totalSeats) + ", " + to_string(fare) + ");";

        if (db.executeQuery(query)) {
            cout << "\n✓ Vehicle added successfully!" << endl;
        } else {
            cout << "\nFailed to add vehicle!" << endl;
        }

        cout << "\nPress Enter to continue...";
        cin.get();
    }

    void viewAllBookings() {
        clearScreen();
        cout << "\n=== ALL BOOKINGS ===" << endl;
        
        string query = "SELECT b.booking_id, u.username, v.vehicle_number, v.source, v.destination, "
                      "b.journey_date, b.num_seats, b.total_amount, b.status FROM bookings b "
                      "JOIN users u ON b.user_id = u.user_id "
                      "JOIN vehicles v ON b.vehicle_id = v.vehicle_id;";
        
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db.getDB(), query.c_str(), -1, &stmt, nullptr);

        cout << "ID\tUser\t\tVehicle\tRoute\t\t\tDate\t\tSeats\tAmount\tStatus" << endl;
        cout << "-------------------------------------------------------------------------" << endl;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            cout << sqlite3_column_int(stmt, 0) << "\t"
                 << sqlite3_column_text(stmt, 1) << "\t"
                 << sqlite3_column_text(stmt, 2) << "\t"
                 << sqlite3_column_text(stmt, 3) << "->" << sqlite3_column_text(stmt, 4) << "\t"
                 << sqlite3_column_text(stmt, 5) << "\t"
                 << sqlite3_column_int(stmt, 6) << "\t"
                 << sqlite3_column_double(stmt, 7) << "\t"
                 << sqlite3_column_text(stmt, 8) << endl;
        }

        sqlite3_finalize(stmt);
        cout << "\nPress Enter to continue...";
        cin.get();
    }

    void viewStatistics() {
        clearScreen();
        cout << "\n=== SYSTEM STATISTICS ===" << endl;
        
        // Total users
        string q1 = "SELECT COUNT(*) FROM users;";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db.getDB(), q1.c_str(), -1, &stmt, nullptr);
        sqlite3_step(stmt);
        cout << "Total Users: " << sqlite3_column_int(stmt, 0) << endl;
        sqlite3_finalize(stmt);

        // Total vehicles
        string q2 = "SELECT COUNT(*) FROM vehicles;";
        sqlite3_prepare_v2(db.getDB(), q2.c_str(), -1, &stmt, nullptr);
        sqlite3_step(stmt);
        cout << "Total Vehicles: " << sqlite3_column_int(stmt, 0) << endl;
        sqlite3_finalize(stmt);

        // Total bookings
        string q3 = "SELECT COUNT(*), SUM(total_amount) FROM bookings WHERE status='confirmed';";
        sqlite3_prepare_v2(db.getDB(), q3.c_str(), -1, &stmt, nullptr);
        sqlite3_step(stmt);
        cout << "Total Bookings: " << sqlite3_column_int(stmt, 0) << endl;
        cout << "Total Revenue: Rs. " << sqlite3_column_double(stmt, 1) << endl;
        sqlite3_finalize(stmt);

        cout << "\nPress Enter to continue...";
        cin.get();
    }
};

int main() {
    ReservationSystem system;
    system.run();
    return 0;
}