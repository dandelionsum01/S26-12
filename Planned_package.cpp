#include "header.h"
using namespace std;

PlannedPackage::PlannedPackage()
{
    basePrice = 0;
    expiryDate = "";
    category = Category::NORTHERN_AREAS;
}

void PlannedPackage::setExpiryDate(const string& expiryDate) { this->expiryDate = expiryDate; }
void PlannedPackage::setBasePrice(int price) { this->basePrice = price; }
void PlannedPackage::setCategory(Category category) { this->category = category; }

string   PlannedPackage::getExpiryDate() const { return expiryDate; }
int      PlannedPackage::getBasePrice()  const { return basePrice; }
Category PlannedPackage::getCategory()   const { return category; }

Category PlannedPackage::intToCategory(int choice)
{
    switch (choice)
    {
    case 1: return Category::NORTHERN_AREAS;
    case 2: return Category::SOUTH;
    case 3: return Category::CENTRAL;
    case 4: return Category::COASTAL;
    default: return Category::NORTHERN_AREAS;
    }
}

string PlannedPackage::categoryToString(Category cat)
{
    switch (cat)
    {
    case Category::NORTHERN_AREAS: return "N.Areas";
    case Category::SOUTH:          return "South";
    case Category::CENTRAL:        return "Central";
    case Category::COASTAL:        return "Coastal";
    default:                       return "Unknown";
    }
}

void PlannedPackage::addPackage()
{
    string id, from, to, hotel, date, expiry;
    int    capacity, priceChoice, categoryChoice;

    cout << "\n--- Add New Package ---\n";
    cout << "Enter Package ID: ";
    cin >> id;
    cout << "Enter Start City: ";
    cin.ignore();
    getline(cin, from);
    cout << "Enter Destination: ";
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
    if (capacity <= 0)
    {
        cout << "Capacity must be positive. Aborting.\n";
        return;
    }

    cout << "Enter Category (1.Northern Areas, 2.South, 3.Central, 4.Coastal): ";
    cin >> categoryChoice;
    Category categoryInput = intToCategory(categoryChoice);
    string   categoryStr = categoryToString(categoryInput);

    cout << "Enter Departure Date (YYYY-MM-DD): ";
    cin >> date;
    cout << "Enter Expiry Date   (YYYY-MM-DD): ";
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

    bool isEmpty = false;
    {
        ifstream packageIn("packages.csv");
        if (packageIn.is_open())
        {
            packageIn.seekg(0, ios::end);
            isEmpty = (packageIn.tellg() == 0);
        }
        else
        {
            isEmpty = true; // not yet created
        }
    }

    ofstream packageOut("packages.csv", ios::app);
    if (!packageOut.is_open())
    {
        cout << "Cannot open packages.csv for writing!\n";
        return;
    }

    if (isEmpty)
        packageOut << "PackageID,Start,Destination,Price,Hotel,Capacity,"
        "DepartureDate,ExpiryDate,Category\n";

    packageOut << id << "," << from << "," << to << ","
        << priceChoice << "," << hotel << "," << capacity << ","
        << date << "," << expiry << "," << categoryStr << "\n";
    packageOut.close();
    cout << "Package added successfully!\n";

    // Notify all subscribers about the new package
    broadcastNotification("New package available: " + id +
        " (" + from + " -> " + to + ")");
}

void PlannedPackage::deletePackage()
{
    string targetID;
    cout << "Enter the Package ID: ";
    cin >> targetID;

    ifstream checkInFile("packages.csv");
    if (!checkInFile.is_open())
    {
        cout << "Cannot open packages.csv or it does not exist!\n";
        return;
    }

    ofstream checkOutFile("temp_packages.csv");
    if (!checkOutFile.is_open())
    {
        cout << "Cannot open temp_packages.csv for writing!\n";
        checkInFile.close();
        return;
    }

    bool   found = false;
    string line;

    getline(checkInFile, line); // header
    checkOutFile << line << "\n";

    while (getline(checkInFile, line))
    {
        stringstream ss(line);
        string id;
        getline(ss, id, ',');

        if (id != targetID)
            checkOutFile << line << "\n";
        else
            found = true;
    }
    checkInFile.close();
    checkOutFile.close();

    if (found)
    {
        remove("packages.csv");
        rename("temp_packages.csv", "packages.csv");
        cout << "Package with Package ID: " << targetID << " successfully removed!\n";
    }
    else
    {
        remove("temp_packages.csv");
        cout << "Package with Package ID: " << targetID << " not found!\n";
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
    getline(file, line); // header

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
        if (line.empty()) continue;
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

        int total = 0;
        try { total = stoi(cap); }
        catch (...) { total = 0; }

        // Booked count is unknown to PlannedPackage (Booking owns it).
        // Show an empty bar to avoid divide-by-zero / misleading data.
        int booked = 0;
        string bar;
        for (int i = 0; i < 10; i++)
            bar += (total > 0 && i < (booked * 10 / total)) ? "#" : "-";

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

static string toLowerStr(string s)
{
    for (size_t i = 0; i < s.length(); ++i)
        s[i] = static_cast<char>(tolower(static_cast<unsigned char>(s[i])));
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
    getline(file, line); // header
    bool found = false;

    while (getline(file, line))
    {
        stringstream ss(line);
        string id, from, to;
        getline(ss, id, ',');
        getline(ss, from, ',');
        getline(ss, to, ',');

        if (toLowerStr(to) == toLowerStr(dest))
        {
            cout << id << " | " << from << " -> " << to << "\n";
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
    time_t t = time(0);
    tm* now = localtime(&t);
    char   buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", now);
    string today = buffer;

    ifstream fileIn("packages.csv");
    if (!fileIn.is_open())
    {
        cout << "Cannot open packages.csv!\n";
        return;
    }

    ofstream fileOut("temp_packages.csv");
    if (!fileOut.is_open())
    {
        cout << "Cannot open temp_packages.csv for writing!\n";
        fileIn.close();
        return;
    }

    string line;
    int    removedCount = 0;

    getline(fileIn, line); // header
    fileOut << line << "\n";

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
}

bool PlannedPackage::checkCapacity()
{
    // Capacity tracking is owned by Booking which writes bookings.csv.
    // This method exists only to satisfy the abstract base contract.
    return true;
}
