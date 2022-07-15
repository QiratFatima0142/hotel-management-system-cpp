# Hotel Management System (C++)

[![Build](https://github.com/QiratFatima0142/hotel-management-system-cpp/actions/workflows/build.yml/badge.svg)](https://github.com/QiratFatima0142/hotel-management-system-cpp/actions/workflows/build.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Language: C++11](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)]()
[![Style: single-file](https://img.shields.io/badge/style-single--file-informational.svg)]()

A **menu-driven console application** built in C++ that simulates the daily
operations of a small hotel — adding customers with family details, booking
rooms by type and date, tracking availability, managing check-in /
check-out, and generating a formatted invoice.

> Built as an **Introduction to Computers / C++** course project after
> ~4 months of study. The entire program lives inside `main()` (no
> user-defined functions) so every beginner can read it top-to-bottom.

---

## Table of Contents

1. [Features](#features)
2. [Folder Structure](#folder-structure)
3. [How to Build & Run](#how-to-build--run)
4. [How the System Works](#how-the-system-works)
5. [Sample Output](#sample-output)
6. [Design Choices](#design-choices)
7. [Limitations](#limitations)
8. [Future Improvements](#future-improvements)
9. [Author](#author)

---

## Features

| #   | Feature             | Description                                                                                                           |
| --- | ------------------- | --------------------------------------------------------------------------------------------------------------------- |
| 1   | Add Customer        | Stores full name, age, phone, address, up to **15 family members** + their **relationship** (Wife, Son, Father, …)   |
| 2   | Book Room           | Asks **room type** + **check-in date (D M Y)** + **days**; runs a date-overlap check so future bookings don't collide |
| 3   | Booked Room Count   | Summary of active bookings + breakdown by type (Single/Double/Suite)                                                  |
| 4   | Availability Check  | Ask for type + date range, system lists which specific rooms of that type are free                                    |
| 5   | Check-In            | Customer arrives — mark booking as checked-in                                                                         |
| 6   | Check-Out + Billing | Generates a professional invoice (Subtotal + 5% service tax) and frees the booking slot                               |
| 7   | Customer List       | Shows every customer added in this session                                                                            |
| 8   | Room Status Board   | Every room with all its bookings (multiple future reservations shown per room)                                        |
| 0   | Exit                | Ends the program (records are cleared from memory)                                                                    |

### Validation & Smart Checks

- Cannot book if **no customer exists yet**.
- **Room-type aware** — if the customer wants a Suite and both Suites
  clash with the requested dates, the system prints:
  > `Sorry! All 2 Suite room(s) are fully booked for 1/1/2026 to 3/1/2026.`
  > `Room 109 -> free from 4/1/2026`
  > `Room 110 -> free from 4/1/2026`
- **Date-overlap aware** — if Suite 109 is booked Jan 1–3 and another
  customer wants it Jan 5–7, the booking is accepted (future
  reservations coexist on the same room).
- Each room has up to **5 booking slots**, so multiple non-overlapping
  reservations per room are supported.
- Cannot **check-in** a booking that doesn't exist for that room + customer.
- Cannot **check-out** without first checking in.
- **Real-calendar date validation** — every month uses its actual
  number of days and **leap years are handled correctly**:
  - `31 1 2026` ✅ (Jan has 31 days)
  - `30 2 2026` ❌ `Month 2/2026 has 28 days.`
  - `29 2 2024` ✅ (2024 is a leap year)
  - `29 2 2026` ❌ `Month 2/2026 has 28 days.`
  - `10 13 2026` ❌ `Month must be between 1 and 12.`
  - Year must be between `2020` and `2100`, days > 0.
- Family count is clamped between `0` and `MAX_FAMILY` (**15**).
- Each family member is stored with a **name + relationship**.
- Invalid menu choices are handled gracefully.

### Date Model (real calendar, no extra libs)

Each date `(D, M, Y)` is validated against a `daysInMonth[12]` table
and a leap-year rule:

```text
leap(Y) = (Y % 4 == 0 && Y % 100 != 0) || (Y % 400 == 0)
```

Internally a date is encoded into a **serial-day** integer counting days
since **1 Jan 2020**, so the check-out date of a booking is simply
`startKey + days - 1`. Two bookings overlap when

```text
A.startKey <= B.endKey  AND  B.startKey <= A.endKey
```

The end-date *display* (e.g. "Check-out: 2/2/2027") is computed by
walking one day at a time with proper month-rollover and leap-year
logic — so booking **Jan 31 for 3 days** correctly yields **Feb 2**.

---

## Folder Structure

```
hotel-management-system-cpp/
│
├── src/
│   └── main.cpp              # Entire program (single-file, all in main)
│
├── docs/                     # Screenshots, reports, extra docs
│
├── .github/
│   └── workflows/
│       └── build.yml         # CI: compile + smoke test on Ubuntu + macOS
│
├── Makefile                  # make / make run / make clean
├── .editorconfig             # Consistent formatting across editors
├── .gitignore                # Ignores the compiled binary, OS junk, etc.
├── LICENSE                   # MIT License
└── README.md                 # This file
```

Just **one** source file — easy to submit, easy to grade, easy to read.

---

## How to Build & Run

### Option A — Using the provided Makefile (recommended)

```bash
git clone https://github.com/QiratFatima0142/hotel-management-system-cpp.git
cd hotel-management-system-cpp
make run           # compile and launch
make clean         # remove the compiled binary
```

### Option B — Plain `g++` (Linux / macOS / MinGW on Windows)

```bash
g++ -std=c++11 -Wall -Wextra -Wpedantic -O2 -o hotel src/main.cpp
./hotel
```

### Option C — Visual Studio (Windows)

1. Create a new **Empty C++ Project**.
2. Right-click **Source Files → Add → Existing Item…** and pick `src/main.cpp`.
3. Press `Ctrl + F5` to build and run.

### Option D — Code::Blocks / Dev-C++

1. Open `src/main.cpp`.
2. Press `F9` (Build & Run).

> Requires any compiler that supports **C++11** or newer
> (all modern compilers do).

### Continuous Integration

Every push to `main` is automatically compiled on **Ubuntu** and **macOS**
with the strictest warning flags (`-Wall -Wextra -Wpedantic`) via GitHub
Actions — see the **Build** badge at the top of this README.

---

## How the System Works

### 1. Data Model

The program uses **two `struct`s** to group data:

- `Room` — number, type (Single / Double / Suite), price per day, booking
  flags, current customer id, days booked.
- `Customer` — id, name, age, phone, address, family count, family names.

All data is kept in simple **arrays** (`Room rooms[10]`,
`Customer customers[50]`) — no dynamic allocation, no vectors, no files.
This mirrors what a student typically knows after a first C++ course.

### 2. Program Flow

```
┌──────────────────────────────────────────────────────────┐
│                      PROGRAM START                       │
│  • 10 rooms are auto-created (4 Single / 4 Double /      │
│    2 Suite) with prices.                                 │
└──────────────────────────────────────────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────────┐
│                     MAIN MENU LOOP                       │
│  Reads user choice and runs the matching block using     │
│  if / else-if chain — loops until user picks 0 (Exit).   │
└──────────────────────────────────────────────────────────┘
                         │
     ┌───────────────────┼────────────────────┐
     ▼                   ▼                    ▼
  Add Customer       Book Room           Check-In
  (assigns ID)       (validates          (must be
                     availability)       booked first)
     │                   │                    │
     └───────────────────┴────────────────────┘
                         │
                         ▼
┌──────────────────────────────────────────────────────────┐
│                      CHECK-OUT                           │
│  • Calculates Subtotal = price/day × days                │
│  • Adds 5% service tax                                   │
│  • Prints a formatted invoice                            │
│  • Frees the room (sets all booking flags to false)      │
└──────────────────────────────────────────────────────────┘
```

### 3. Key Logic Blocks (all inside `main`)

- **Booking** — asks for room **type**, **check-in date** and **days**.
  The system encodes both the requested range and every existing
  booking into integer keys and walks `rooms[]`. A room is picked only
  if it's of the right type **AND** no existing slot's date range
  overlaps the new one. If all rooms of that type clash, the system
  prints when each will be free.
- **Availability check (option 4)** — same overlap test but read-only.
  Useful for asking "can I get a Suite on those dates?" **before**
  creating the booking.
- **Multi-booking per room** — every room holds an array
  `BookingSlot slots[5]`, so one room can have several future
  reservations as long as their date ranges don't touch each other.
- **Billing** — `subtotal = pricePerDay * daysToStay`, then 5% service
  tax is added. All numbers are displayed with `fixed << setprecision(2)`
  for a clean, currency-style output.

### 4. Persistence

As required, records are **in-memory only**. Closing the program clears
everything — there is _no_ file I/O.

---

## Sample Output

**Main menu**

```
==============================================
     WELCOME TO GRAND PALACE HOTEL SYSTEM
==============================================
  Rooms : 4 Single (3500/day)
          4 Double (6000/day)
          2 Suite  (12000/day)
==============================================

----------------- MAIN MENU -----------------
 1. Add New Customer (with family details)
 2. Book a Room (by type + dates)
 3. Show Number of Rooms Currently Booked
 4. Check Availability by Type & Dates
 5. Check-In Customer
 6. Check-Out Customer (with Bill)
 7. Display All Customers
 8. Display All Rooms (Status Board)
 0. Exit
---------------------------------------------
```

**Adding a customer (now up to 15 family members, with relationships)**

```
--- ADD NEW CUSTOMER ---
Enter full name      : Ahmed Khan
Enter age            : 34
Enter phone number   : 0300-1234567
Enter home address   : House 22, F-8, Islamabad
How many family members? (0 - 15) : 3

  (Common relationships: Wife, Husband, Son,
   Daughter, Father, Mother, Brother, Sister,
   Uncle, Aunt, Cousin, Grandfather, Grandmother)
  Family member 1 name         : Fatima Khan
  Family member 1 relationship : Wife
  Family member 2 name         : Ali Khan
  Family member 2 relationship : Son
  Family member 3 name         : Sara Khan
  Family member 3 relationship : Daughter

[+] Customer added successfully!
    Customer ID : 1001
```

**Invalid date examples (real-calendar checks)**

```
Check-in date (D M Y) : 32 1 2026
[!] Invalid date: Day is invalid for that month. Month 1/2026 has 31 days.

Check-in date (D M Y) : 10 13 2026
[!] Invalid date: Month must be between 1 and 12.

Check-in date (D M Y) : 30 2 2026
[!] Invalid date: Day is invalid for that month. Month 2/2026 has 28 days.

Check-in date (D M Y) : 29 2 2024   <- leap year, accepted
Number of days        : 3
[+] Room booked successfully!
    Check-in  : 29/2/2024
    Check-out : 2/3/2024

Check-in date (D M Y) : 31 1 2027   <- 31-day month + 3 days
Number of days        : 3
[+] Room booked successfully!
    Check-in  : 31/1/2027
    Check-out : 2/2/2027           <- month rolls correctly
```

**Customer list now shows relationships**

```
--- ALL CUSTOMERS (1) ---

  ID      : 1001
  Name    : Ahmed Khan
  Age     : 34
  Phone   : 0300-1234567
  Address : House 22, F-8
  Family  : 7 members
            - Fatima Khan (Wife)
            - Ali Khan (Son)
            - Sara Khan (Daughter)
            - Bilal Khan (Son)
            - Rehman Khan (Father)
            - Ayesha Khan (Mother)
            - Asad Khan (Brother)
  ----------------------------------
```

**Booking by type + dates** (customer 1 books a Suite for Jan 1–3)

```
--- BOOK A ROOM ---
Enter Customer ID : 1001

Select room type:
   1. Single  (PKR 3500 / day)
   2. Double  (PKR 6000 / day)
   3. Suite   (PKR 12000 / day)
Enter choice (1-3) : 3
Check-in date (D M Y)   : 1 1 2026
Number of days to stay  : 3

[+] Room booked successfully!
    Room Number : 109 (Suite)
    Customer    : Ahmed Khan
    Check-in    : 1/1/2026
    Check-out   : 3/1/2026
    Days        : 3
    Estimated   : PKR 36000.00
```

**Both suites now taken — 3rd customer tries the same dates**

```
Enter choice (1-3) : 3
Check-in date (D M Y)   : 1 1 2026
Number of days to stay  : 3

[!] Sorry! All 2 Suite room(s) are fully booked for 1/1/2026 to 3/1/2026.
    Here is when each Suite room will be free:
      Room 109 -> free from 4/1/2026
      Room 110 -> free from 4/1/2026
    Please come back with different dates.
```

**Same customer tries Jan 5–7 — accepted (room frees up in time)**

```
Enter choice (1-3) : 3
Check-in date (D M Y)   : 5 1 2026
Number of days to stay  : 3

[+] Room booked successfully!
    Room Number : 109 (Suite)
    Customer    : Bilal Ahmed
    Check-in    : 5/1/2026
    Check-out   : 7/1/2026
    Days        : 3
    Estimated   : PKR 36000.00
```

**Room status board** (shows Room 109 holding two non-overlapping bookings)

```
 Room 109  (Suite, PKR 12000.00/day)
    * Cust 1001 | 1/1/2026 -> 3/1/2026 | days=3 | booked
    * Cust 1003 | 5/1/2026 -> 7/1/2026 | days=3 | booked

 Room 110  (Suite, PKR 12000.00/day)
    * Cust 1002 | 1/1/2026 -> 3/1/2026 | days=3 | booked
```

**Availability check (option 4)**

```
--- CHECK AVAILABILITY ---
Select room type:
   1. Single
   2. Double
   3. Suite
Enter choice (1-3) : 3
Check-in date (D M Y) : 5 1 2026
Number of days        : 3

Checking Suite rooms from 5/1/2026 to 7/1/2026 ...

  [OK]  Room 109 is AVAILABLE for those dates.
  [OK]  Room 110 is AVAILABLE for those dates.

Result : 2 of 2 Suite room(s) available.
```

**Check-Out & Bill**

```
Enter your choice : 6

--- CHECK-OUT & BILLING ---
Enter room number : 109

==========================================
            GRAND PALACE HOTEL
              FINAL INVOICE
==========================================
 Customer ID   : 1001
 Customer Name : Ahmed Khan
 Phone         : 0300-1234567
 Family Size   : 3 (incl. customer)
 Room Number   : 109
 Room Type     : Suite
 Rate / Day    : PKR 12000.00
 Days Stayed   : 3
------------------------------------------
 Subtotal      : PKR 36000.00
 Service Tax   : PKR 1800.00 (5%)
 GRAND TOTAL   : PKR 37800.00
==========================================
   Thank you for staying with us!
==========================================
```

**Room status board**

```
--- ROOM STATUS BOARD ---
Room    Type      Price/Day   Booked?     CheckedIn?  CustID    Days
--------------------------------------------------------------------
101     Single    3500        NO          NO          -         0
102     Single    3500        NO          NO          -         0
...
109     Suite     12000       YES         YES         1001      3
110     Suite     12000       NO          NO          -         0
```

---

## Design Choices

- **Single file, no functions** — requested for the course. Makes it easy
  to read top-to-bottom without jumping around.
- **Structs (`Room`, `Customer`)** — structs are basic C++ data grouping
  (not functions), perfectly fine for a first-course project and make
  the arrays very readable.
- **`if / else if` chain instead of `switch`** — `switch` doesn't allow
  variable declarations without braces; the `if` chain keeps the code
  flatter and easier to follow for a beginner.
- **Fixed sizes (10 rooms, 50 customers)** — avoids dynamic memory. A
  student learns vectors later; arrays are fine here.
- **5% service tax** — a realistic touch that makes the invoice feel
  complete.

---

## Limitations

- No file saving — all data is lost on exit (this was **required**).
- Only one room per customer can be tracked at a time via the
  booking-id link.
- No date support; "days to stay" is just an integer entered by the user.
- No authentication / admin login (not needed for this scope).

---

## Future Improvements

If you want to extend this for a bigger project:

- Save / load customers & rooms from a text file (`fstream`).
- Separate the code into functions, then into classes (`Hotel`, `Room`,
  `Customer`).
- Let one customer book multiple rooms.
- Add search by customer name / phone.
- Store actual check-in / check-out **dates** using `<ctime>`.
- Build a simple GUI using Qt or raylib.

---

## Author

**Qirat Fatima**
Computer Science student — Introduction to Computers / C++

- GitHub: [@QiratFatima0142](https://github.com/QiratFatima0142)
- Project: [hotel-management-system-cpp](https://github.com/QiratFatima0142/hotel-management-system-cpp)

If this project helped you, a star on the repo is very appreciated.

---

## License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE)
file for details. You are free to fork, study, and extend it.

---

## Contributing

This is primarily a personal / course project, but suggestions and pull
requests are welcome. If you'd like to contribute:

1. Fork the repository
2. Create your feature branch (`git checkout -b feat/my-feature`)
3. Commit your changes (`git commit -m "feat: add my feature"`)
4. Push to the branch (`git push origin feat/my-feature`)
5. Open a Pull Request

Please run the build locally (`make`) before submitting a PR so the CI
stays green.
