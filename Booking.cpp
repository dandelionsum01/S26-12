// ============================================================
//  Booking.cpp
//
//  Reads:    packages.csv
//  Writes:   bookings.csv
// ============================================================
#include "header.h"
#include <cctype>
using namespace std;

// ----------------------------------------------------------------
// Local helpers
// ----------------------------------------------------------------
namespace
{
    string trimWS(string s)
    {
        while (!s.empty() && (s.back() == '\r' || s.back() == '\n' ||
            s.back() == ' ' || s.back() == '\t'))
            s.pop_back();
        return s;
    }

    string todayString()
    {
        time_t t = time(0);
        tm* now = localtime(&t);
        char   buf[20];
        strftime(buf, sizeof(buf), "%Y-%m-%d", now);
        return string(buf);
    }

    struct PkgRow
    {
        string id, from, to, hotel, depDate, expiry, category;
        int    price = 0;
        int    capacity = 0;
    };

    PkgRow* loadPackages(int& count)
    {
        count = 0;
        ifstream f("packages.csv");
        if (!f.is_open()) return nullptr;

        string line;
        getline(f, line); // header
        while (getline(f, line)) if (!line.empty()) count++;
        f.close();

        if (count == 0) return nullptr;

        PkgRow* rows = new PkgRow[count];
        ifstream f2("packages.csv");
        getline(f2, line); // header
        int i = 0;
        while (getline(f2, line) && i < count)
        {
            if (line.empty()) continue;
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

            try
            {
                rows[i].price = stoi(price);
                rows[i].capacity = stoi(cap);
                i++;
            }
            catch (...)
            {
                // Skip malformed row
            }
        }
        f2.close();
        count = i;
        return rows;
    }

    int countBookings(const string& packageID)
    {
        int booked = 0;
        ifstream f("bookings.csv");
        if (!f.is_open()) return 0;

        string line;
        getline(f, line); // header
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

    string generateBookingID(const string& packageID)
    {
        int n = countBookings(packageID) + 1;
        return "BK-" + packageID + "-" + to_string(n);
    }
}

// ----------------------------------------------------------------
// ctor / setters / getters
// ----------------------------------------------------------------
Booking::Booking()
{
    payment = 0;
    packageID = "";
    customerUsername = "";
}

void   Booking::setPayment(int p) { payment = p; }
void   Booking::setPackageID(const string& id) { packageID = id; }
void   Booking::setCustomerUsername(const string& u) { customerUsername = u; }

int    Booking::getPayment()         const { return payment; }
string Booking::getPackageID()       const { return packageID; }
string Booking::getCustomerUsername()const { return customerUsername; }

// ----------------------------------------------------------------
// Luhn check
// ----------------------------------------------------------------
bool Booking::checkCreditNum(const string& cardNum)
{
    string digits;
    for (char c : cardNum)
        if (isdigit(static_cast<unsigned char>(c))) digits += c;

    if (digits.length() < 13 || digits.length() > 19) return false;

    int  sum = 0;
    bool alt = false;
    for (int i = static_cast<int>(digits.length()) - 1; i >= 0; --i)
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

// ----------------------------------------------------------------
// updateCapacity: decrement Capacity in packages.csv for given id.
// ----------------------------------------------------------------
void Booking::updateCapacity(const string& pkgID)
{
    ifstream fileIn("packages.csv");
    if (!fileIn.is_open())
    {
        cout << "Cannot open packages.csv!\n";
        return;
    }

    ofstream fileOut("temp_packages_cap.csv");
    if (!fileOut.is_open())
    {
        cout << "Cannot open temp file!\n";
        fileIn.close();
        return;
    }

    string line;
    bool   updated = false;

    getline(fileIn, line); // header
    fileOut << line << "\n";

    while (getline(fileIn, line))
    {
        stringstream ss(line);
        string id, from, to, price, hotel, cap, depDate, expiry, category;
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
            int remaining = 0;
            try { remaining = stoi(cap) - 1; }
            catch (...) { remaining = 0; }
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

// ----------------------------------------------------------------
// Total amount paid by a single customer across bookings.csv.
// ----------------------------------------------------------------
int Booking::returnPayment(const string& customerUsername)
{
    ifstream f("bookings.csv");
    if (!f.is_open()) return 0;

    int total = 0;
    string line;
    getline(f, line); // header

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
        {
            try { total += stoi(pay); }
            catch (...) {}
        }
    }
    f.close();
    return total;
}

// ----------------------------------------------------------------
// makeBooking
// ----------------------------------------------------------------
void Booking::makeBooking(const string& customerUsername)
{
    int     pkgCount = 0;
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
    for (int i = 0; i < pkgCount; ++i)
    {
        if (packages[i].expiry < today) continue;

        int booked = countBookings(packages[i].id);
        int seatsLeft = packages[i].capacity - booked;
        if (seatsLeft <= 0) continue;

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

    // -------- pick a package --------
    string chosenID;
    PkgRow chosen;
    bool   found = false;

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

        for (int i = 0; i < pkgCount; ++i)
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

    int booked = countBookings(chosen.id);
    int seatsLeft = chosen.capacity - booked;

    if (seatsLeft <= 0)
    {
        cout << "Sorry, this package is fully booked!\n";
        return;
    }

    // -------- already booked? --------
    {
        ifstream bf("bookings.csv");
        if (bf.is_open())
        {
            string line;
            getline(bf, line); // header
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
                    return;
                }
            }
        }
    }

    // -------- dynamic pricing --------
    double fillRate = (chosen.capacity > 0)
        ? static_cast<double>(booked) / chosen.capacity
        : 0.0;
    int    finalPrice = chosen.price;
    string priceNote;
    if (fillRate > 0.8)
    {
        finalPrice = static_cast<int>(chosen.price * 1.2);
        priceNote = " (High Demand +20%)";
    }
    else if (fillRate < 0.5)
    {
        finalPrice = static_cast<int>(chosen.price * 0.9);
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

    // -------- credit card check --------
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

    // -------- write to bookings.csv --------
    string bookingID = generateBookingID(chosen.id);

    bool isEmpty = true;
    {
        ifstream chk("bookings.csv");
        if (chk.is_open())
        {
            chk.seekg(0, ios::end);
            isEmpty = (chk.tellg() == 0);
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

    // NOTE: updateCapacity() intentionally NOT called.
    // countBookings(bookings.csv) is the single source of truth
    // for how many seats have been taken. Calling updateCapacity
    // here would also decrement the Capacity field in packages.csv,
    // and the viewer (which subtracts countBookings from capacity)
    // would then double-count -- making seatsLeft drop by 2 per
    // booking instead of 1. The function is kept in the class for
    // backward compatibility but should be considered deprecated.

    setPackageID(chosen.id);
    setCustomerUsername(customerUsername);
    setPayment(finalPrice);

    cout << "\nBooking confirmed!\n";
    cout << "Booking ID:  " << bookingID << "\n";
    cout << "Amount Paid: Rs. " << finalPrice << "\n";
    cout << "Thank you for booking with us!\n";
}
