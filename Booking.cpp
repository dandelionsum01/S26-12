// ============================================================
// MEMBER C  —  Booking.cpp
// Depends on:  packages.csv        (written by Member B)
// Produces:    bookings.csv        (read by Member B for checkCapacity / getDynamicPrice)
//
// bookings.csv columns:
//   BookingID, CustomerUsername, PackageID, DepartureDate, Payment, BookingDate
// ============================================================

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include "header.h"
using namespace std;

// ─── tiny helpers (local to this translation unit) ───────────────────────────

static string trimWS(string s)
{
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' ||
        s.back() == ' ' || s.back() == '\t'))
        s.pop_back();
    return s;
}

static string todayString()
{
    time_t t = time(0);
    tm* now = localtime(&t);
    char   buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d", now);
    return string(buf);
}

// ─── struct that holds one package row read from packages.csv ────────────────
struct PkgRow
{
    string id, from, to, hotel, depDate, expiry, category;
    int    price = 0;
    int    capacity = 0;
};

// Read all rows from packages.csv into a heap array; caller must delete[].
static PkgRow* loadPackages(int& count)
{
    count = 0;
    ifstream f("packages.csv");
    if (!f.is_open()) return nullptr;

    string line;
    getline(f, line);               // skip header
    while (getline(f, line)) count++;
    f.close();

    if (count == 0) return nullptr;

    PkgRow* rows = new PkgRow[count];
    ifstream f2("packages.csv");
    getline(f2, line);              // skip header
    int i = 0;
    while (getline(f2, line) && i < count)
    {
        stringstream ss(line);
        string price, cap;
        getline(ss, rows[i].id, ',');
        getline(ss, rows[i].from, ',');
        getline(ss, rows[i].to, ',');
        getline(ss, price, ',');
        getline(ss, rows[i].hotel, ',');
        getline(ss, cap, ',');
        getline(ss, rows[i].depDate, ',');
        getline(ss, rows[i].expiry, ',');
        getline(ss, rows[i].category, ',');
        rows[i].category = trimWS(rows[i].category);
        rows[i].price = stoi(price);
        rows[i].capacity = stoi(cap);
        i++;
    }
    f2.close();
    return rows;
}

// Count how many confirmed bookings exist for a given packageID in bookings.csv
static int countBookings(const string& packageID)
{
    int booked = 0;
    ifstream f("bookings.csv");
    if (!f.is_open()) return 0;

    string line;
    getline(f, line);               // skip header
    while (getline(f, line))
    {
        stringstream ss(line);
        string bid, uname, pkgID;
        getline(ss, bid, ',');
        getline(ss, uname, ',');
        getline(ss, pkgID, ',');
        if (trimWS(pkgID) == packageID) booked++;
    }
    f.close();
    return booked;
}

// Generate a unique BookingID: "BK-" + packageID + "-" + running number
static string generateBookingID(const string& packageID)
{
    int n = countBookings(packageID) + 1;
    return "BK-" + packageID + "-" + to_string(n);
}

// ─── Booking constructor ──────────────────────────────────────────────────────

Booking::Booking()
{
    payment = 0;
    packageID = "";
    customerUsername = "";
}

// ─── Setters / Getters ────────────────────────────────────────────────────────

void Booking::setPayment(int p) { payment = p; }
void Booking::setPackageID(string id) { packageID = id; }
void Booking::setCustomerUsername(string u) { customerUsername = u; }

int    Booking::getPayment() { return payment; }
string Booking::getPackageID() { return packageID; }
string Booking::getCustomerUsername() { return customerUsername; }

// ─── checkCreditNum  (Luhn algorithm) ────────────────────────────────────────
// Returns true if the card number passes the Luhn check.
bool Booking::checkCreditNum(string cardNum)
{
    // strip spaces / dashes for convenience
    string digits = "";
    for (char c : cardNum)
        if (isdigit(c)) digits += c;

    if (digits.length() < 13 || digits.length() > 19)
        return false;

    int sum = 0;
    bool alt = false;
    for (int i = (int)digits.length() - 1; i >= 0; i--)
    {
        int d = digits[i] - '0';
        if (alt)
        {
            d *= 2;
            if (d > 9) d -= 9;
        }
        sum += d;
        alt = !alt;
    }
    return (sum % 10 == 0);
}

// ─── updateCapacity ──────────────────────────────────────────────────────────
// Decrements the Capacity field in packages.csv for the given packageID.
// Call this after a booking is successfully written to bookings.csv.
void Booking::updateCapacity(string pkgID)
{
    ifstream fileIn("packages.csv");
    if (!fileIn.is_open())
    {
        cout << "Cannot open packages.csv!\n";
        return;
    }

    ofstream fileOut("temp_packages_cap.csv");
    string line;
    bool updated = false;

    getline(fileIn, line);
    fileOut << line << "\n";        // copy header

    while (getline(fileIn, line))
    {
        stringstream ss(line);
        string id, from, to, price, hotel, cap,
            depDate, expiry, category;
        getline(ss, id, ',');
        getline(ss, from, ',');
        getline(ss, to, ',');
        getline(ss, price, ',');
        getline(ss, hotel, ',');
        getline(ss, cap, ',');
        getline(ss, depDate, ',');
        getline(ss, expiry, ',');
        getline(ss, category, ',');
        category = trimWS(category);

        if (trimWS(id) == pkgID)
        {
            int remaining = stoi(cap) - 1;
            if (remaining < 0) remaining = 0;
            fileOut << id << "," << from << "," << to << ","
                << price << "," << hotel << "," << remaining << ","
                << depDate << "," << expiry << "," << category << "\n";
            updated = true;
        }
        else
        {
            fileOut << line << "\n";
        }
    }

    fileIn.close();
    fileOut.close();

    if (updated)
    {
        remove("packages.csv");
        rename("temp_packages_cap.csv", "packages.csv");
    }
    else
    {
        remove("temp_packages_cap.csv");
        cout << "Package ID not found while updating capacity.\n";
    }
}

// ─── returnPayment ───────────────────────────────────────────────────────────
// Returns the TOTAL amount paid by a customer across all their bookings.
// Used by Accounts::readPayments() (Member A).
int Booking::returnPayment(string customerUsername)
{
    ifstream f("bookings.csv");
    if (!f.is_open()) return 0;

    int total = 0;
    string line;
    getline(f, line);               // skip header

    while (getline(f, line))
    {
        stringstream ss(line);
        string bid, uname, pkgID, depDate, pay, bookDate;
        getline(ss, bid, ',');
        getline(ss, uname, ',');
        getline(ss, pkgID, ',');
        getline(ss, depDate, ',');
        getline(ss, pay, ',');
        getline(ss, bookDate, ',');

        if (trimWS(uname) == customerUsername)
            total += stoi(pay);
    }
    f.close();
    return total;
}

// ─── makeBooking ─────────────────────────────────────────────────────────────
void Booking::makeBooking(string customerUsername)
{
    // ── 1. Load and display available packages ───────────────────────────────
    int pkgCount = 0;
    PkgRow* packages = loadPackages(pkgCount);

    if (packages == nullptr || pkgCount == 0)
    {
        cout << "No packages available at the moment.\n";
        delete[] packages;
        return;
    }

    string today = todayString();

    cout << "\n========== Available Packages ==========\n";
    cout << left
        << setw(14) << "ID"
        << setw(12) << "From"
        << setw(12) << "To"
        << setw(10) << "Price"
        << setw(17) << "Hotel"
        << setw(10) << "Seats Left"
        << setw(12) << "Departure"
        << setw(10) << "Category"
        << "\n";
    cout << string(97, '-') << "\n";

    bool anyAvailable = false;
    for (int i = 0; i < pkgCount; i++)
    {
        // Skip expired packages
        if (packages[i].expiry < today) continue;

        int booked = countBookings(packages[i].id);
        int seatsLeft = packages[i].capacity - booked;
        if (seatsLeft <= 0) continue;   // fully booked

        anyAvailable = true;
        cout << left
            << setw(14) << packages[i].id
            << setw(12) << packages[i].from
            << setw(12) << packages[i].to
            << setw(10) << ("Rs." + to_string(packages[i].price))
            << setw(17) << packages[i].hotel
            << setw(10) << seatsLeft
            << setw(12) << packages[i].depDate
            << setw(10) << packages[i].category
            << "\n";
    }

    if (!anyAvailable)
    {
        cout << "No packages are available for booking right now.\n";
        delete[] packages;
        return;
    }
    cout << "=========================================\n";

    // ── 2. Ask customer to pick a package ───────────────────────────────────
    string chosenID;
    PkgRow chosen;
    bool found = false;

    while (!found)
    {
        cout << "Enter Package ID to book (or 0 to cancel): ";
        cin >> chosenID;
        if (chosenID == "0")
        {
            cout << "Booking cancelled.\n";
            delete[] packages;
            return;
        }

        for (int i = 0; i < pkgCount; i++)
        {
            if (trimWS(packages[i].id) == chosenID)
            {
                chosen = packages[i];
                found = true;
                break;
            }
        }
        if (!found)
            cout << "Package ID not found. Try again.\n";
    }
    delete[] packages;

    // ── 3. Check capacity ────────────────────────────────────────────────────
    int booked = countBookings(chosen.id);
    int seatsLeft = chosen.capacity - booked;

    if (seatsLeft <= 0)
    {
        cout << "Sorry, this package is fully booked!\n";
        return;
    }

    // ── 4. Check if customer already booked this package ─────────────────────
    {
        ifstream bf("bookings.csv");
        if (bf.is_open())
        {
            string line;
            getline(bf, line);
            while (getline(bf, line))
            {
                stringstream ss(line);
                string bid, uname, pkgID;
                getline(ss, bid, ',');
                getline(ss, uname, ',');
                getline(ss, pkgID, ',');
                if (trimWS(uname) == customerUsername &&
                    trimWS(pkgID) == chosen.id)
                {
                    cout << "You have already booked this package!\n";
                    bf.close();
                    return;
                }
            }
            bf.close();
        }
    }

    // ── 5. Show booking summary ──────────────────────────────────────────────
    // Apply dynamic pricing logic
    double fillRate = (double)booked / chosen.capacity;
    int    finalPrice = chosen.price;
    string priceNote = "";
    if (fillRate > 0.8)
    {
        finalPrice = (int)(chosen.price * 1.2);
        priceNote = " (High Demand +20%)";
    }
    else if (fillRate < 0.5)
    {
        finalPrice = (int)(chosen.price * 0.9);
        priceNote = " (Early Bird -10%)";
    }

    cout << "\n--- Booking Summary ---\n";
    cout << "Package ID:    " << chosen.id << "\n";
    cout << "From:          " << chosen.from << "\n";
    cout << "To:            " << chosen.to << "\n";
    cout << "Hotel:         " << chosen.hotel << "\n";
    cout << "Departure:     " << chosen.depDate << "\n";
    cout << "Seats Left:    " << seatsLeft << "\n";
    cout << "Price:         Rs. " << finalPrice << priceNote << "\n";
    cout << "-----------------------\n";

    char confirm;
    cout << "Confirm booking? (y/n): ";
    do {
        cin >> confirm;
        if (confirm != 'y' && confirm != 'Y' &&
            confirm != 'n' && confirm != 'N')
            cout << "Please enter y or n: ";
    } while (confirm != 'y' && confirm != 'Y' &&
        confirm != 'n' && confirm != 'N');

    if (confirm == 'n' || confirm == 'N')
    {
        cout << "Booking cancelled.\n";
        return;
    }

    // ── 6. Payment via credit card ───────────────────────────────────────────
    string cardNum;
    bool   validCard = false;
    int    attempts = 0;

    while (!validCard && attempts < 3)
    {
        cout << "Enter Credit Card Number: ";
        cin >> cardNum;
        validCard = checkCreditNum(cardNum);
        if (!validCard)
        {
            attempts++;
            cout << "Invalid card number! "
                << (3 - attempts) << " attempt(s) remaining.\n";
        }
    }

    if (!validCard)
    {
        cout << "Too many failed attempts. Booking cancelled.\n";
        return;
    }
    cout << "Card verified successfully.\n";

    // ── 7. Write to bookings.csv ─────────────────────────────────────────────
    string bookingID = generateBookingID(chosen.id);

    bool isEmpty = true;
    {
        ifstream chk("bookings.csv");
        if (chk.is_open())
        {
            chk.seekg(0, ios::end);
            isEmpty = (chk.tellg() == 0);
            chk.close();
        }
    }

    ofstream out("bookings.csv", ios::app);
    if (!out.is_open())
    {
        cout << "Cannot open bookings.csv!\n";
        return;
    }
    if (isEmpty)
        out << "BookingID,CustomerUsername,PackageID,DepartureDate,Payment,BookingDate\n";

    out << bookingID << ","
        << customerUsername << ","
        << chosen.id << ","
        << chosen.depDate << ","
        << finalPrice << ","
        << todayString() << "\n";
    out.close();

    // ── 8. Decrement capacity in packages.csv ────────────────────────────────
    updateCapacity(chosen.id);

    setPackageID(chosen.id);
    setCustomerUsername(customerUsername);
    setPayment(finalPrice);

    cout << "\n✔ Booking confirmed!\n";
    cout << "Booking ID: " << bookingID << "\n";
    cout << "Amount Paid: Rs. " << finalPrice << "\n";
    cout << "Thank you for booking with us!\n";
}
