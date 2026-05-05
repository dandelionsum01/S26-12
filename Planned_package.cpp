#include <iostream>
#include <string>
#include <fstream>
#include "header.h"
#include <iomanip>
#include <ctime>
using namespace std;

PlannedPackage::PlannedPackage()
{
    basePrice = 0;
    expiryDate = "";
    category = Category::NORTHERN_AREAS;
}

void PlannedPackage::setExpiryDate(string expiryDate)
{
    this->expiryDate = expiryDate;
}
void PlannedPackage::setBasePrice(int price)
{
    this->basePrice = price;
}
void PlannedPackage::setCategory(Category category)
{
    this->category = category;
}
string PlannedPackage::getExpiryDate()
{
    return expiryDate;
}
int PlannedPackage::getBasePrice()
{
    return basePrice;
}
Category PlannedPackage::getCategory()
{
    return category;
}

Category PlannedPackage::intToCategory(int choice)
{
    switch (choice)
    {
    case 1:
        return Category::NORTHERN_AREAS;
    case 2:
        return Category::SOUTH;
    case 3:
        return Category::CENTRAL;
    case 4:
        return Category::COASTAL;
    default:
        return Category::NORTHERN_AREAS;
    }
}
string PlannedPackage::categoryToString(Category cat)
{
    switch (cat)
    {
    case Category::NORTHERN_AREAS:
        return "N.Areas";
    case Category::SOUTH:
        return "South";
    case Category::CENTRAL:
        return "Central";
    case Category::COASTAL:
        return "Coastal";
    default:
        return "Unknown";
    }
}

void PlannedPackage::addPackage()
{
    string id, from, to, hotel, date, expiry;
    int capacity, priceChoice, categoryChoice;

    Category categoryInput;

    cout << "\n--- Add New Package ---\n";
    cout << "Enter Package ID: ";
    cin >> id;
    cout << "Enter Start City: ";
    cin.ignore();
    getline(cin, from);
    cout << "Enter Destination: ";
    cin.ignore();
    getline(cin, to);

    int suggestedFuel = displayRouteInfo(from, to);
    cout << "Suggested Price: Rs. " << suggestedFuel << "\n";
    cout << "Enter your price (0 to use suggested): ";
    cin >> priceChoice;
    if (priceChoice == 0)
        priceChoice = suggestedFuel;

    cout << "Enter Hotel: ";
    cin.ignore();
    getline(cin, hotel);
    cout << "Enter Capacity: ";
    cin >> capacity;
    cout << "Enter Category (1.Northern Areas:\n2.South:\n3.Central:\n4.Coastal: ";
    cin >> categoryChoice;
    categoryInput = intToCategory(categoryChoice);        // int  → Category
    string categoryStr = categoryToString(categoryInput); // Category → string  ✅
    cout << "Enter Departure Date: (YYYY-MM-DD) ";
    cin >> date;
    cout << "Enter Expiry Date: (YYYY-MM-DD) ";
    cin >> expiry;

    setPackageID(id);
    setStart(from);
    setDestination(to);
    setPackageFee(priceChoice);
    setHotel(hotel);
    setCapacity(capacity);
    setDepartureDate(date);
    setExpiryDate(expiry);
    setCategory(categoryInput);

    bool isEmpty = 0;
    ifstream packageIn;
    packageIn.open("packages.csv");
    if (packageIn.is_open())
    {
        packageIn.seekg(0, ios::end);
        isEmpty = (packageIn.tellg() == 0);
        packageIn.close();
    }
    else
    {
        cout << "Cannot open packageIn file!\n";
        isEmpty = true; // assume empty if file can't be opened
    }

    ofstream packageOut;
    packageOut.open("packages.csv", ios::app);

    if (packageOut.is_open())
    {
        if (isEmpty)
        {
            packageOut << "PackageID,Start,Destination,Price,Hotel,Capacity,DepartureDate,ExpiryDate,Category\n"; // header once
        }

        packageOut << id << "," << from << "," << to << "," << priceChoice << ","
            << hotel << "," << capacity << "," << date << "," << expiry << "," << categoryStr << "\n";

        packageOut.close();
        cout << "Package added successfully!\n";
    }
    else
        cout << "Cannot open packageOut file!\n";
}

void PlannedPackage::deletePackage()
{
    string targetID;
    cout << "Enter the Package ID : ";
    cin >> targetID;

    ifstream checkInFile;
    checkInFile.open("packages.csv");
    if (!checkInFile.is_open())
    {
        cout << "Can not open packages.csv or it does not exists!\n";
        return;
    }
    else
    {
        bool found = false;
        ofstream checkOutFile;
        checkOutFile.open("temp_packages.csv");
        string line;
        if (checkOutFile.is_open())
        {
            getline(checkInFile, line);
            checkOutFile << line << "\n"; // copy header

            while (getline(checkInFile, line))
            {
                stringstream ss(line);
                string id;
                getline(ss, id, ','); // read from start of each line until comma and store in id
                if (id != targetID)
                {
                    checkOutFile << line << "\n"; // copy line if id does not match
                }
                else
                {
                    found = true; // skip the line if id matches
                }
            }
            checkOutFile.close();
        }

        checkInFile.close();
        if (found)
        {
            remove("packages.csv");
            rename("temp_packages.csv", "packages.csv");
            cout << "Package with Package Id:" << targetID << " successfully removed!\n";
        }
        else
        {
            remove("temp_packages.csv");
            cout << "Package with Package ID: " << targetID << " not found!\n";
        }
    }
}
void PlannedPackage::displayPackage()
{
    ifstream file("packages.csv");
    if (!file.is_open())
    {
        cout << "Cannot open packages.csv!\n";
        return;
    }

    string line;
    getline(file, line); // skip header

    cout << "\n--- All Planned Packages ---\n";
    cout << left
        << setw(12) << "ID"
        << setw(12) << "From"
        << setw(12) << "To"
        << setw(10) << "Fee"
        << setw(17) << "Hotel"
        << setw(12) << "Seats"
        << setw(12) << "Departure"
        << setw(12) << "Expiry"
        << setw(10) << "Category"

        << "\n";
    cout << string(85, '-') << "\n";

    while (getline(file, line))
    {
        stringstream ss(line);
        string id, from, to, price, hotel, cap, depDate, expiry, cat;
        getline(ss, id, ',');
        getline(ss, from, ',');
        getline(ss, to, ',');
        getline(ss, price, ',');
        getline(ss, hotel, ',');
        getline(ss, cap, ',');
        getline(ss, depDate, ',');
        getline(ss, expiry, ',');
        getline(ss, cat, ',');

        // capacity bar
        // int total = stoi(cap);
        // int booked = total - checkCapacity(); // allign with member 3
        int total = stoi(cap);
        int booked = 0;
        string bar = "";
        for (int i = 0; i < 10; i++)
            bar += (i < (booked * 10 / total)) ? "#" : "-";

        cout << left
            << setw(12) << id
            << setw(12) << from
            << setw(12) << to
            << setw(10) << ("Rs." + price)
            << setw(17) << hotel
            << setw(12) << ("[" + bar + "]")
            << setw(12) << depDate
            << setw(12) << expiry
            << setw(10) << cat

            << "\n";
    }

    file.close();
}

string toLower(string s)
{
    for (int i = 0; i < s.length(); i++)
        s[i] = tolower(s[i]);
    return s;
}

string PlannedPackage::findPackage()
{
    string dest;
    cout << "Enter destination to search: ";
    cin >> dest;

    ifstream file("packages.csv");
    if (!file.is_open())
    {
        cout << "Cannot open packages.csv!\n";
        return "";
    }
    string line;
    getline(file, line); // skip header
    bool found = false;

    while (getline(file, line))
    {
        stringstream ss(line);
        string id, from, to;
        getline(ss, id, ',');
        getline(ss, from, ',');
        getline(ss, to, ',');

        if (toLower(to) == toLower(dest)) // to compare the lowercase/
        {
            cout << id << " | " << from << " → " << to << "\n";
            found = true;
        }
    }
    if (!found)
        cout << "No packages found for " << dest << "\n";
    file.close();
    return dest;
}

void PlannedPackage::removeExpiredPackages()
{
    // get today's date automatically
    time_t t = time(0); // from system clock
    tm* now = localtime(&t);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", now);
    string today = buffer;

    ifstream fileIn("packages.csv");
    if (!fileIn.is_open())
    {
        cout << "Cannot open packages.csv!\n";
        return;
    }

    ofstream fileOut("temp_packages.csv");
    string line;
    int removedCount = 0;

    getline(fileIn, line);
    fileOut << line << "\n"; // copy header

    while (getline(fileIn, line))
    {
        stringstream ss(line);
        string id, from, to, price, hotel, cap, depDate, expiry, cat;
        getline(ss, id, ',');
        getline(ss, from, ',');
        getline(ss, to, ',');
        getline(ss, price, ',');
        getline(ss, hotel, ',');
        getline(ss, cap, ',');
        getline(ss, depDate, ',');
        getline(ss, expiry, ',');
        getline(ss, cat, ',');

        if (expiry >= today)
            fileOut << line << "\n";
        else
            removedCount++;
    }

    fileIn.close();
    fileOut.close();

    if (removedCount > 0)
    {
        remove("packages.csv");
        rename("temp_packages.csv", "packages.csv");
        cout << removedCount << " expired packages removed.\n";
    }
    else
    {
        remove("temp_packages.csv");
        cout << "No expired packages found.\n";
    }
    cout << removedCount << " expired packages removed.\n";
}

bool PlannedPackage::checkCapacity()
{
    // will read bookings.csv, count bookings for this packageID and departure date
    //  for now return true (seats available)////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    return true;
}