/* =====================================================================
 *                 HOTEL MANAGEMENT SYSTEM (C++)
 * ---------------------------------------------------------------------
 *  Course      : Introduction to Computers / C++ Programming
 *  Language    : C++ (Standard C++11)
 *  Style       : Single-file, no user-defined functions (all inside main)
 *  Persistence : In-memory only (records are lost when program closes)
 *
 *  Features:
 *    1) Add new customer (with family details)
 *    2) Book a room  -> asks room TYPE + check-in date + days
 *                       (checks date-overlap, supports future bookings,
 *                        tells the customer when rooms free up if the
 *                        requested type is fully booked for those dates)
 *    3) Show number of rooms currently booked
 *    4) Check availability by type & dates (before booking)
 *    5) Check-in customer
 *    6) Check-out customer (auto-generates the invoice)
 *    7) Display all customers
 *    8) Display all rooms (status board with every booking)
 *    0) Exit program
 *
 *  Dates are stored as simple (Day, Month, Year) and also encoded into a
 *  "serial key" = (Year-2020)*365 + (Month-1)*30 + Day  so overlap checks
 *  can be done with plain integer comparisons. A 30-day month is used
 *  for simplicity (typical for a first C++ course).
 * =====================================================================
 */

#include <iostream>
#include <string>
#include <iomanip>   // setw / setprecision for nice tables

using namespace std;

/* ---------------------------------------------------------------------
 *  CONSTANTS
 * ------------------------------------------------------------------ */
const int TOTAL_ROOMS            = 10;   // total rooms in the hotel
const int MAX_CUSTOMERS          = 50;   // max customers per run
const int MAX_FAMILY             = 15;   // max family members per customer
const int MAX_BOOKINGS_PER_ROOM  = 5;    // allow up to 5 future bookings per room

/* ---------------------------------------------------------------------
 *  STRUCT : Customer
 * ------------------------------------------------------------------ */
struct Customer {
    int    id;                              // unique id (auto)
    string name;                            // full name
    int    age;                             // age in years
    string phone;                           // contact number
    string address;                         // home address
    int    familyCount;                     // number of family members
    string familyNames[MAX_FAMILY];         // family member names
    string familyRelations[MAX_FAMILY];     // relationship (Wife/Son/Father/...)
};

/* ---------------------------------------------------------------------
 *  STRUCT : BookingSlot
 *  Each room keeps an array of these slots so multiple customers can
 *  reserve the same room for non-overlapping date ranges.
 * ------------------------------------------------------------------ */
struct BookingSlot {
    bool   occupied;       // true if this slot is in use
    int    customerId;     // which customer reserved it
    int    startD, startM, startY;   // check-in date (human-readable)
    int    endD,   endM,   endY;     // check-out date (human-readable)
    int    startKey;       // check-in  encoded as integer for compare
    int    endKey;         // check-out encoded as integer for compare
    int    daysToStay;     // number of days booked
    bool   isCheckedIn;    // customer has physically checked in
};

/* ---------------------------------------------------------------------
 *  STRUCT : Room
 * ------------------------------------------------------------------ */
struct Room {
    int          number;                           // 101, 102, ...
    string       type;                             // "Single"/"Double"/"Suite"
    double       pricePerDay;                      // rent per day in PKR
    BookingSlot  slots[MAX_BOOKINGS_PER_ROOM];     // reservation calendar
};

int main() {

    /* -----------------------------------------------------------------
     *  CALENDAR DATA
     *  Real days-per-month (Feb auto-bumps to 29 in leap years).
     *  Used for validation, serial-key date encoding, and end-date
     *  calculation throughout the program.
     * -------------------------------------------------------------- */
    const int daysInMonth[12] = { 31, 28, 31, 30, 31, 30,
                                  31, 31, 30, 31, 30, 31 };

    /* -----------------------------------------------------------------
     *  INITIALISE ROOMS
     *  4 Singles (101-104), 4 Doubles (105-108), 2 Suites (109-110)
     * -------------------------------------------------------------- */
    Room rooms[TOTAL_ROOMS];

    for (int i = 0; i < TOTAL_ROOMS; i++) {
        rooms[i].number = 101 + i;

        if (i < 4) {
            rooms[i].type        = "Single";
            rooms[i].pricePerDay = 3500.0;
        }
        else if (i < 8) {
            rooms[i].type        = "Double";
            rooms[i].pricePerDay = 6000.0;
        }
        else {
            rooms[i].type        = "Suite";
            rooms[i].pricePerDay = 12000.0;
        }

        for (int s = 0; s < MAX_BOOKINGS_PER_ROOM; s++) {
            rooms[i].slots[s].occupied    = false;
            rooms[i].slots[s].customerId  = -1;
            rooms[i].slots[s].startD      = 0;
            rooms[i].slots[s].startM      = 0;
            rooms[i].slots[s].startY      = 0;
            rooms[i].slots[s].endD        = 0;
            rooms[i].slots[s].endM        = 0;
            rooms[i].slots[s].endY        = 0;
            rooms[i].slots[s].startKey    = 0;
            rooms[i].slots[s].endKey      = 0;
            rooms[i].slots[s].daysToStay  = 0;
            rooms[i].slots[s].isCheckedIn = false;
        }
    }

    /* -----------------------------------------------------------------
     *  CUSTOMER STORAGE
     * -------------------------------------------------------------- */
    Customer customers[MAX_CUSTOMERS];
    int customerCount  = 0;
    int nextCustomerId = 1001;

    int choice = -1;

    // Welcome banner
    cout << "\n==============================================\n";
    cout << "     WELCOME TO GRAND PALACE HOTEL SYSTEM\n";
    cout << "==============================================\n";
    cout << "  Rooms : 4 Single (3500/day)\n";
    cout << "          4 Double (6000/day)\n";
    cout << "          2 Suite  (12000/day)\n";
    cout << "==============================================\n";

    /* =================================================================
     *                         MAIN MENU LOOP
     * ============================================================== */
    while (choice != 0) {

        cout << "\n----------------- MAIN MENU -----------------\n";
        cout << " 1. Add New Customer (with family details)\n";
        cout << " 2. Book a Room (by type + dates)\n";
        cout << " 3. Show Number of Rooms Currently Booked\n";
        cout << " 4. Check Availability by Type & Dates\n";
        cout << " 5. Check-In Customer\n";
        cout << " 6. Check-Out Customer (with Bill)\n";
        cout << " 7. Display All Customers\n";
        cout << " 8. Display All Rooms (Status Board)\n";
        cout << " 0. Exit\n";
        cout << "---------------------------------------------\n";
        cout << "Enter your choice : ";
        cin  >> choice;
        cin.ignore();

        /* ================= 1. ADD NEW CUSTOMER ================== */
        if (choice == 1) {
            if (customerCount >= MAX_CUSTOMERS) {
                cout << "\n[!] Customer list is full. Cannot add more.\n";
            }
            else {
                Customer c;
                c.id = nextCustomerId;
                nextCustomerId++;

                cout << "\n--- ADD NEW CUSTOMER ---\n";
                cout << "Enter full name      : ";
                getline(cin, c.name);

                cout << "Enter age            : ";
                cin  >> c.age;
                cin.ignore();

                cout << "Enter phone number   : ";
                getline(cin, c.phone);

                cout << "Enter home address   : ";
                getline(cin, c.address);

                cout << "How many family members? (0 - " << MAX_FAMILY << ") : ";
                cin  >> c.familyCount;
                cin.ignore();

                if (c.familyCount < 0)          c.familyCount = 0;
                if (c.familyCount > MAX_FAMILY) c.familyCount = MAX_FAMILY;

                cout << "\n  (Common relationships: Wife, Husband, Son,\n";
                cout <<   "   Daughter, Father, Mother, Brother, Sister,\n";
                cout <<   "   Uncle, Aunt, Cousin, Grandfather, Grandmother)\n";

                for (int i = 0; i < c.familyCount; i++) {
                    cout << "  Family member " << (i + 1) << " name         : ";
                    getline(cin, c.familyNames[i]);
                    cout << "  Family member " << (i + 1) << " relationship : ";
                    getline(cin, c.familyRelations[i]);
                }

                customers[customerCount] = c;
                customerCount++;

                cout << "\n[+] Customer added successfully!\n";
                cout << "    Customer ID : " << c.id << "\n";
                cout << "    Please remember this ID for booking.\n";
            }
        }

        /* ====================== 2. BOOK A ROOM ================== */
        else if (choice == 2) {
            if (customerCount == 0) {
                cout << "\n[!] No customer exists yet. Please add a customer first.\n";
            }
            else {
                int cid;
                cout << "\n--- BOOK A ROOM ---\n";
                cout << "Enter Customer ID : ";
                cin  >> cid;

                // find customer
                int custIdx = -1;
                for (int i = 0; i < customerCount; i++) {
                    if (customers[i].id == cid) { custIdx = i; break; }
                }

                if (custIdx == -1) {
                    cout << "[!] Customer ID not found.\n";
                }
                else {
                    // -------- ask for room TYPE --------
                    cout << "\nSelect room type:\n";
                    cout << "   1. Single  (PKR 3500 / day)\n";
                    cout << "   2. Double  (PKR 6000 / day)\n";
                    cout << "   3. Suite   (PKR 12000 / day)\n";
                    cout << "Enter choice (1-3) : ";
                    int tChoice;
                    cin  >> tChoice;

                    string wantType = "";
                    if      (tChoice == 1) wantType = "Single";
                    else if (tChoice == 2) wantType = "Double";
                    else if (tChoice == 3) wantType = "Suite";

                    if (wantType == "") {
                        cout << "[!] Invalid room type choice.\n";
                    }
                    else {
                        // -------- ask for dates --------
                        int sD, sM, sY, days;
                        cout << "Check-in date (D M Y) : ";
                        cin  >> sD >> sM >> sY;
                        cout << "Number of days to stay : ";
                        cin  >> days;

                        // -------- proper calendar validation --------
                        // 1) days > 0
                        // 2) month in 1..12
                        // 3) year in 2020..2100
                        // 4) day in 1..(real days in that month, leap-aware)
                        bool validDate = true;
                        string dateError = "";

                        if (days <= 0)                { validDate = false; dateError = "Days must be at least 1."; }
                        else if (sM < 1 || sM > 12)   { validDate = false; dateError = "Month must be between 1 and 12."; }
                        else if (sY < 2020 || sY > 2100) { validDate = false; dateError = "Year must be between 2020 and 2100."; }
                        else {
                            bool isLeap = (sY % 4 == 0 && sY % 100 != 0) || (sY % 400 == 0);
                            int maxDay = daysInMonth[sM - 1];
                            if (sM == 2 && isLeap) maxDay = 29;

                            if (sD < 1 || sD > maxDay) {
                                validDate = false;
                                dateError = "Day is invalid for that month. Month ";
                                dateError += to_string(sM);
                                dateError += "/" + to_string(sY) + " has ";
                                dateError += to_string(maxDay);
                                dateError += " days.";
                            }
                        }

                        if (!validDate) {
                            cout << "[!] Invalid date: " << dateError << "\n";
                        }
                        else {
                            // -------- encode date as serial-day (real calendar) --------
                            // serial = days elapsed since 1/1/2020
                            int newStart = 0;
                            for (int y = 2020; y < sY; y++) {
                                bool leapY = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
                                newStart += leapY ? 366 : 365;
                            }
                            bool leapCurr = (sY % 4 == 0 && sY % 100 != 0) || (sY % 400 == 0);
                            for (int m = 1; m < sM; m++) {
                                int md = daysInMonth[m - 1];
                                if (m == 2 && leapCurr) md = 29;
                                newStart += md;
                            }
                            newStart += sD;
                            int newEnd = newStart + days - 1;

                            // -------- compute check-out date (real months) --------
                            int eD = sD, eM = sM, eY = sY;
                            for (int step = 0; step < days - 1; step++) {
                                eD++;
                                bool leapE = (eY % 4 == 0 && eY % 100 != 0) || (eY % 400 == 0);
                                int mdE = daysInMonth[eM - 1];
                                if (eM == 2 && leapE) mdE = 29;
                                if (eD > mdE) {
                                    eD = 1; eM++;
                                    if (eM > 12) { eM = 1; eY++; }
                                }
                            }

                            // count total rooms of this type
                            int typeTotal = 0;
                            for (int i = 0; i < TOTAL_ROOMS; i++) {
                                if (rooms[i].type == wantType) typeTotal++;
                            }

                            // try to find a room of wanted type with
                            // (a) a free slot AND (b) no date overlap
                            int assignedRoom = -1;
                            int assignedSlot = -1;
                            int clashingRooms = 0;  // rooms of type that clash

                            for (int i = 0; i < TOTAL_ROOMS; i++) {
                                if (rooms[i].type != wantType) continue;

                                bool overlap  = false;
                                int  freeSlot = -1;

                                for (int s = 0; s < MAX_BOOKINGS_PER_ROOM; s++) {
                                    if (!rooms[i].slots[s].occupied) {
                                        if (freeSlot == -1) freeSlot = s;
                                        continue;
                                    }
                                    // overlap test: [a,b] and [c,d] overlap iff a<=d && c<=b
                                    if (rooms[i].slots[s].startKey <= newEnd &&
                                        newStart <= rooms[i].slots[s].endKey) {
                                        overlap = true;
                                    }
                                }

                                if (overlap) {
                                    clashingRooms++;
                                }
                                else if (freeSlot != -1 && assignedRoom == -1) {
                                    assignedRoom = i;
                                    assignedSlot = freeSlot;
                                }
                            }

                            if (typeTotal == 0) {
                                cout << "[!] No rooms of type " << wantType << " exist.\n";
                            }
                            else if (assignedRoom == -1) {
                                // ALL rooms of this type clash with requested dates
                                cout << "\n[!] Sorry! All " << typeTotal
                                     << " " << wantType
                                     << " room(s) are fully booked for "
                                     << sD << "/" << sM << "/" << sY
                                     << " to " << eD << "/" << eM << "/" << eY << ".\n";

                                cout << "    Here is when each " << wantType
                                     << " room will be free:\n";

                                for (int i = 0; i < TOTAL_ROOMS; i++) {
                                    if (rooms[i].type != wantType) continue;

                                    // find the LAST (max endKey) clashing booking
                                    int maxEndKey = -1;
                                    int maxIdx    = -1;

                                    for (int s = 0; s < MAX_BOOKINGS_PER_ROOM; s++) {
                                        if (rooms[i].slots[s].occupied &&
                                            rooms[i].slots[s].startKey <= newEnd &&
                                            newStart <= rooms[i].slots[s].endKey &&
                                            rooms[i].slots[s].endKey > maxEndKey) {
                                            maxEndKey = rooms[i].slots[s].endKey;
                                            maxIdx    = s;
                                        }
                                    }

                                    if (maxIdx != -1) {
                                        int fD = rooms[i].slots[maxIdx].endD + 1;
                                        int fM = rooms[i].slots[maxIdx].endM;
                                        int fY = rooms[i].slots[maxIdx].endY;

                                        bool leapF = (fY % 4 == 0 && fY % 100 != 0) || (fY % 400 == 0);
                                        int mdF = daysInMonth[fM - 1];
                                        if (fM == 2 && leapF) mdF = 29;
                                        if (fD > mdF) {
                                            fD = 1; fM++;
                                            if (fM > 12) { fM = 1; fY++; }
                                        }

                                        cout << "      Room " << rooms[i].number
                                             << " -> free from "
                                             << fD << "/" << fM << "/" << fY << "\n";
                                    }
                                }
                                cout << "    Please come back with different dates.\n";
                            }
                            else {
                                // assign the booking slot
                                rooms[assignedRoom].slots[assignedSlot].occupied    = true;
                                rooms[assignedRoom].slots[assignedSlot].customerId  = cid;
                                rooms[assignedRoom].slots[assignedSlot].startD      = sD;
                                rooms[assignedRoom].slots[assignedSlot].startM      = sM;
                                rooms[assignedRoom].slots[assignedSlot].startY      = sY;
                                rooms[assignedRoom].slots[assignedSlot].endD        = eD;
                                rooms[assignedRoom].slots[assignedSlot].endM        = eM;
                                rooms[assignedRoom].slots[assignedSlot].endY        = eY;
                                rooms[assignedRoom].slots[assignedSlot].startKey    = newStart;
                                rooms[assignedRoom].slots[assignedSlot].endKey      = newEnd;
                                rooms[assignedRoom].slots[assignedSlot].daysToStay  = days;
                                rooms[assignedRoom].slots[assignedSlot].isCheckedIn = false;

                                double total = rooms[assignedRoom].pricePerDay * days;

                                cout << "\n[+] Room booked successfully!\n";
                                cout << "    Room Number : " << rooms[assignedRoom].number
                                     << " (" << wantType << ")\n";
                                cout << "    Customer    : " << customers[custIdx].name << "\n";
                                cout << "    Check-in    : " << sD << "/" << sM << "/" << sY << "\n";
                                cout << "    Check-out   : " << eD << "/" << eM << "/" << eY << "\n";
                                cout << "    Days        : " << days << "\n";
                                cout << "    Estimated   : PKR " << fixed << setprecision(2) << total << "\n";

                                if (clashingRooms > 0) {
                                    cout << "    (Note: " << clashingRooms
                                         << " other " << wantType
                                         << " room(s) were unavailable for your dates.)\n";
                                }
                            }
                        }
                    }
                }
            }
        }

        /* ============ 3. COUNT ROOMS CURRENTLY BOOKED =========== */
        else if (choice == 3) {
            int totalBookings    = 0;
            int roomsWithBooking = 0;

            for (int i = 0; i < TOTAL_ROOMS; i++) {
                bool hasAny = false;
                for (int s = 0; s < MAX_BOOKINGS_PER_ROOM; s++) {
                    if (rooms[i].slots[s].occupied) {
                        totalBookings++;
                        hasAny = true;
                    }
                }
                if (hasAny) roomsWithBooking++;
            }

            cout << "\n--- BOOKING SUMMARY ---\n";
            cout << "Total rooms in hotel          : " << TOTAL_ROOMS << "\n";
            cout << "Rooms with at least 1 booking : " << roomsWithBooking << "\n";
            cout << "Rooms fully unreserved        : " << (TOTAL_ROOMS - roomsWithBooking) << "\n";
            cout << "Total active bookings         : " << totalBookings << "\n";

            // breakdown by type
            cout << "\nBy type:\n";
            for (int pass = 0; pass < 3; pass++) {
                string t = (pass == 0) ? "Single" : (pass == 1) ? "Double" : "Suite";
                int tTot = 0, tBooked = 0;
                for (int i = 0; i < TOTAL_ROOMS; i++) {
                    if (rooms[i].type != t) continue;
                    tTot++;
                    for (int s = 0; s < MAX_BOOKINGS_PER_ROOM; s++) {
                        if (rooms[i].slots[s].occupied) { tBooked++; break; }
                    }
                }
                cout << "  " << left << setw(8) << t
                     << " : " << tBooked << " / " << tTot
                     << " rooms have bookings\n";
            }
        }

        /* ============ 4. CHECK AVAILABILITY BY TYPE & DATE ====== */
        else if (choice == 4) {
            cout << "\n--- CHECK AVAILABILITY ---\n";
            cout << "Select room type:\n";
            cout << "   1. Single\n   2. Double\n   3. Suite\n";
            cout << "Enter choice (1-3) : ";
            int tChoice;
            cin  >> tChoice;

            string wantType = "";
            if      (tChoice == 1) wantType = "Single";
            else if (tChoice == 2) wantType = "Double";
            else if (tChoice == 3) wantType = "Suite";

            if (wantType == "") {
                cout << "[!] Invalid type.\n";
            }
            else {
                int sD, sM, sY, days;
                cout << "Check-in date (D M Y) : ";
                cin  >> sD >> sM >> sY;
                cout << "Number of days        : ";
                cin  >> days;

                // -------- real-calendar date validation --------
                bool validDate = true;
                string dateError = "";

                if (days <= 0)                { validDate = false; dateError = "Days must be at least 1."; }
                else if (sM < 1 || sM > 12)   { validDate = false; dateError = "Month must be between 1 and 12."; }
                else if (sY < 2020 || sY > 2100) { validDate = false; dateError = "Year must be between 2020 and 2100."; }
                else {
                    bool isLeap = (sY % 4 == 0 && sY % 100 != 0) || (sY % 400 == 0);
                    int maxDay = daysInMonth[sM - 1];
                    if (sM == 2 && isLeap) maxDay = 29;
                    if (sD < 1 || sD > maxDay) {
                        validDate = false;
                        dateError = "Day is invalid for that month. Month ";
                        dateError += to_string(sM);
                        dateError += "/" + to_string(sY) + " has ";
                        dateError += to_string(maxDay);
                        dateError += " days.";
                    }
                }

                if (!validDate) {
                    cout << "[!] Invalid date: " << dateError << "\n";
                }
                else {
                    // real serial-day encoding
                    int newStart = 0;
                    for (int y = 2020; y < sY; y++) {
                        bool leapY = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
                        newStart += leapY ? 366 : 365;
                    }
                    bool leapCurr = (sY % 4 == 0 && sY % 100 != 0) || (sY % 400 == 0);
                    for (int m = 1; m < sM; m++) {
                        int md = daysInMonth[m - 1];
                        if (m == 2 && leapCurr) md = 29;
                        newStart += md;
                    }
                    newStart += sD;
                    int newEnd = newStart + days - 1;

                    // compute check-out date using real month lengths
                    int eD = sD, eM = sM, eY = sY;
                    for (int step = 0; step < days - 1; step++) {
                        eD++;
                        bool leapE = (eY % 4 == 0 && eY % 100 != 0) || (eY % 400 == 0);
                        int mdE = daysInMonth[eM - 1];
                        if (eM == 2 && leapE) mdE = 29;
                        if (eD > mdE) {
                            eD = 1; eM++;
                            if (eM > 12) { eM = 1; eY++; }
                        }
                    }

                    cout << "\nChecking " << wantType << " rooms from "
                         << sD << "/" << sM << "/" << sY << " to "
                         << eD << "/" << eM << "/" << eY << " ...\n\n";

                    int freeCount = 0;
                    int tTot = 0;

                    for (int i = 0; i < TOTAL_ROOMS; i++) {
                        if (rooms[i].type != wantType) continue;
                        tTot++;

                        bool overlap = false;
                        for (int s = 0; s < MAX_BOOKINGS_PER_ROOM; s++) {
                            if (rooms[i].slots[s].occupied &&
                                rooms[i].slots[s].startKey <= newEnd &&
                                newStart <= rooms[i].slots[s].endKey) {
                                overlap = true;
                                break;
                            }
                        }

                        if (!overlap) {
                            cout << "  [OK]  Room " << rooms[i].number
                                 << " is AVAILABLE for those dates.\n";
                            freeCount++;
                        }
                        else {
                            cout << "  [X]   Room " << rooms[i].number
                                 << " is BUSY for those dates.\n";
                        }
                    }

                    cout << "\nResult : " << freeCount << " of " << tTot
                         << " " << wantType << " room(s) available.\n";
                }
            }
        }

        /* ====================== 5. CHECK-IN ===================== */
        else if (choice == 5) {
            int rno, cid;
            cout << "\n--- CHECK-IN ---\n";
            cout << "Enter room number : ";
            cin  >> rno;
            cout << "Enter customer ID : ";
            cin  >> cid;

            int ridx = -1;
            for (int i = 0; i < TOTAL_ROOMS; i++) {
                if (rooms[i].number == rno) { ridx = i; break; }
            }

            if (ridx == -1) {
                cout << "[!] Invalid room number.\n";
            }
            else {
                int sidx = -1;
                for (int s = 0; s < MAX_BOOKINGS_PER_ROOM; s++) {
                    if (rooms[ridx].slots[s].occupied &&
                        rooms[ridx].slots[s].customerId == cid) {
                        sidx = s; break;
                    }
                }

                if (sidx == -1) {
                    cout << "[!] No booking found for that customer in that room.\n";
                }
                else if (rooms[ridx].slots[sidx].isCheckedIn) {
                    cout << "[!] Customer is already checked-in.\n";
                }
                else {
                    rooms[ridx].slots[sidx].isCheckedIn = true;

                    string cname = "Unknown";
                    for (int i = 0; i < customerCount; i++) {
                        if (customers[i].id == cid) { cname = customers[i].name; break; }
                    }

                    cout << "\n[+] Check-in successful!\n";
                    cout << "    Room     : " << rooms[ridx].number << "\n";
                    cout << "    Customer : " << cname << "\n";
                    cout << "    Stay     : "
                         << rooms[ridx].slots[sidx].startD << "/"
                         << rooms[ridx].slots[sidx].startM << "/"
                         << rooms[ridx].slots[sidx].startY << " to "
                         << rooms[ridx].slots[sidx].endD << "/"
                         << rooms[ridx].slots[sidx].endM << "/"
                         << rooms[ridx].slots[sidx].endY << "\n";
                    cout << "    Enjoy your stay!\n";
                }
            }
        }

        /* =============== 6. CHECK-OUT + BILLING ================= */
        else if (choice == 6) {
            int rno, cid;
            cout << "\n--- CHECK-OUT & BILLING ---\n";
            cout << "Enter room number : ";
            cin  >> rno;
            cout << "Enter customer ID : ";
            cin  >> cid;

            int ridx = -1;
            for (int i = 0; i < TOTAL_ROOMS; i++) {
                if (rooms[i].number == rno) { ridx = i; break; }
            }

            if (ridx == -1) {
                cout << "[!] Invalid room number.\n";
            }
            else {
                int sidx = -1;
                for (int s = 0; s < MAX_BOOKINGS_PER_ROOM; s++) {
                    if (rooms[ridx].slots[s].occupied &&
                        rooms[ridx].slots[s].customerId == cid) {
                        sidx = s; break;
                    }
                }

                if (sidx == -1) {
                    cout << "[!] No booking found for that customer in that room.\n";
                }
                else if (!rooms[ridx].slots[sidx].isCheckedIn) {
                    cout << "[!] Customer has not checked-in yet. Cannot check-out.\n";
                }
                else {
                    int cidx = -1;
                    for (int i = 0; i < customerCount; i++) {
                        if (customers[i].id == cid) { cidx = i; break; }
                    }

                    double subtotal = rooms[ridx].pricePerDay *
                                      rooms[ridx].slots[sidx].daysToStay;
                    double tax      = subtotal * 0.05;
                    double grand    = subtotal + tax;

                    cout << "\n==========================================\n";
                    cout <<   "            GRAND PALACE HOTEL\n";
                    cout <<   "              FINAL INVOICE\n";
                    cout <<   "==========================================\n";
                    if (cidx != -1) {
                        cout << " Customer ID   : " << customers[cidx].id      << "\n";
                        cout << " Customer Name : " << customers[cidx].name    << "\n";
                        cout << " Phone         : " << customers[cidx].phone   << "\n";
                        cout << " Family Size   : "
                             << (1 + customers[cidx].familyCount)
                             << " (incl. customer)\n";
                    }
                    cout << " Room Number   : " << rooms[ridx].number        << "\n";
                    cout << " Room Type     : " << rooms[ridx].type          << "\n";
                    cout << " Check-in      : "
                         << rooms[ridx].slots[sidx].startD << "/"
                         << rooms[ridx].slots[sidx].startM << "/"
                         << rooms[ridx].slots[sidx].startY << "\n";
                    cout << " Check-out     : "
                         << rooms[ridx].slots[sidx].endD << "/"
                         << rooms[ridx].slots[sidx].endM << "/"
                         << rooms[ridx].slots[sidx].endY << "\n";
                    cout << " Rate / Day    : PKR " << fixed << setprecision(2)
                                                    << rooms[ridx].pricePerDay << "\n";
                    cout << " Days Stayed   : " << rooms[ridx].slots[sidx].daysToStay << "\n";
                    cout << "------------------------------------------\n";
                    cout << " Subtotal      : PKR " << subtotal              << "\n";
                    cout << " Service Tax   : PKR " << tax << " (5%)\n";
                    cout << " GRAND TOTAL   : PKR " << grand                 << "\n";
                    cout << "==========================================\n";
                    cout << "   Thank you for staying with us!\n";
                    cout << "==========================================\n";

                    // free this slot
                    rooms[ridx].slots[sidx].occupied    = false;
                    rooms[ridx].slots[sidx].customerId  = -1;
                    rooms[ridx].slots[sidx].isCheckedIn = false;
                    rooms[ridx].slots[sidx].daysToStay  = 0;
                    rooms[ridx].slots[sidx].startKey    = 0;
                    rooms[ridx].slots[sidx].endKey      = 0;
                }
            }
        }

        /* ================ 7. DISPLAY ALL CUSTOMERS ============== */
        else if (choice == 7) {
            if (customerCount == 0) {
                cout << "\n[!] No customers have been added yet.\n";
            }
            else {
                cout << "\n--- ALL CUSTOMERS (" << customerCount << ") ---\n";
                for (int i = 0; i < customerCount; i++) {
                    cout << "\n  ID      : " << customers[i].id      << "\n";
                    cout << "  Name    : "   << customers[i].name    << "\n";
                    cout << "  Age     : "   << customers[i].age     << "\n";
                    cout << "  Phone   : "   << customers[i].phone   << "\n";
                    cout << "  Address : "   << customers[i].address << "\n";
                    cout << "  Family  : "   << customers[i].familyCount << " members";
                    if (customers[i].familyCount > 0) {
                        cout << "\n";
                        for (int j = 0; j < customers[i].familyCount; j++) {
                            cout << "            - " << customers[i].familyNames[j]
                                 << " ("            << customers[i].familyRelations[j]
                                 << ")\n";
                        }
                    } else {
                        cout << "\n";
                    }
                    cout << "  ----------------------------------";
                }
                cout << "\n";
            }
        }

        /* ============ 8. DISPLAY ALL ROOMS (STATUS BOARD) ======= */
        else if (choice == 8) {
            cout << "\n--- ROOM STATUS BOARD ---\n";
            for (int i = 0; i < TOTAL_ROOMS; i++) {
                cout << "\n Room " << rooms[i].number
                     << "  (" << rooms[i].type
                     << ", PKR " << fixed << setprecision(2) << rooms[i].pricePerDay << "/day)\n";

                int shown = 0;
                for (int s = 0; s < MAX_BOOKINGS_PER_ROOM; s++) {
                    if (!rooms[i].slots[s].occupied) continue;
                    cout << "    * Cust " << rooms[i].slots[s].customerId
                         << " | " << rooms[i].slots[s].startD << "/"
                                  << rooms[i].slots[s].startM << "/"
                                  << rooms[i].slots[s].startY
                         << " -> "  << rooms[i].slots[s].endD << "/"
                                    << rooms[i].slots[s].endM << "/"
                                    << rooms[i].slots[s].endY
                         << " | days=" << rooms[i].slots[s].daysToStay
                         << " | " << (rooms[i].slots[s].isCheckedIn ? "CHECKED-IN" : "booked")
                         << "\n";
                    shown++;
                }

                if (shown == 0) {
                    cout << "    (no bookings)\n";
                }
            }
        }

        /* ======================= 0. EXIT ======================== */
        else if (choice == 0) {
            cout << "\n==============================================\n";
            cout << "  Thank you for using Grand Palace Hotel System\n";
            cout << "  NOTE: All records are cleared on exit.\n";
            cout << "==============================================\n";
        }

        /* =================== INVALID CHOICE ===================== */
        else {
            cout << "\n[!] Invalid choice. Please pick from the menu.\n";
        }
    }

    return 0;
}
