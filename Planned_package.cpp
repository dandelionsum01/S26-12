#include "header.h"
using namespace std;

PlannedPackage::PlannedPackage()
{
    basePrice = 0;
    expiryDate = "";
    category = Category::NORTHERN_AREAS;
}

void PlannedPackage::setExpiryDate(const string &expiryDate)
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

string PlannedPackage::getExpiryDate() const
{
    return expiryDate;
}
int PlannedPackage::getBasePrice() const
{
    return basePrice;
}
Category PlannedPackage::getCategory() const
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

namespace
{
    bool isValidDate(const string &d)
    {
        if (d.length() != 10)
            return false;
        if (d[4] != '-' || d[7] != '-')
            return false;
        for (int i = 0; i < 10; ++i)
        {
            if (i == 4 || i == 7)
                continue;
            if (d[i] < '0' || d[i] > '9')
                return false;
        }
        int year, month, day;
        try
        {
            year = stoi(d.substr(0, 4));
            month = stoi(d.substr(5, 2));
            day = stoi(d.substr(8, 2));
        }
        catch (...)
        {
            return false;
        }

        if (year < 2024 || year > 2100)
            return false;
        if (month < 1 || month > 12)
            return false;
        if (day < 1 || day > 31)
            return false;

        int monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        if (month == 2 && leap)
            monthDays[1] = 29;
        if (day > monthDays[month - 1])
            return false;
        return true;
    }

    string readValidDate(const string &prompt)
    {
        for (int i = 0; i < 3; ++i)
        {
            cout << prompt;
            string d;
            cin >> d;
            if (isValidDate(d))
                return d;
            cout << "Invalid date. Use YYYY-MM-DD (e.g. 2026-08-20).\n";
        }
        cout << "Too many invalid attempts. Aborting.\n";
        return "";
    }

    string cityCode(const string &s, int n = 3)
    {
        string out;
        for (char c : s)
        {
            if (isalpha(static_cast<unsigned char>(c)))
            {
                out += static_cast<char>(toupper(static_cast<unsigned char>(c)));
                if (static_cast<int>(out.size()) == n)
                    break;
            }
        }
        if (out.empty())
            out = "XXX";
        return out;
    }

    vector<string> readExistingPackageIDs()
    {
        vector<string> ids;
        ifstream f("packages.csv");
        if (!f.is_open())
            return ids;
        string line;
        getline(f, line); // header
        while (getline(f, line))
        {
            if (line.empty())
                continue;
            stringstream ss(line);
            string id;
            getline(ss, id, ',');
            while (!id.empty() && (id.back() == '\r' || id.back() == '\n'))
                id.pop_back();
            if (!id.empty())
                ids.push_back(id);
        }
        return ids;
    }

    string generateUniquePackageID(const string &from, const string &to)
    {
        string prefix = cityCode(from) + "-" + cityCode(to) + "-";
        vector<string> existing = readExistingPackageIDs();
        int n = 1;
        while (true)
        {
            string candidate = prefix + to_string(n);
            bool clash = false;
            for (const string &e : existing)
                if (e == candidate)
                {
                    clash = true;
                    break;
                }
            if (!clash)
                return candidate;
            n++;
        }
    }
} // namespace

void PlannedPackage::addPackage()
{
    string from, to, hotel;
    int capacity, priceChoice, categoryChoice;

    cout << "\n--- Add New Package ---\n";

    cout << "Enter Start City: ";
    getline(cin, from);

    cout << "Enter Destination: ";
    getline(cin, to);

    string fromNorm = normalizeCityName(from);
    string toNorm = normalizeCityName(to);

    if (fromNorm.empty() || toNorm.empty())
    {
        cout << "Invalid city name. Use letters only (e.g. Lahore, New York).\n";
        return;
    }

    int suggestedFuel = displayRouteInfo(fromNorm, toNorm);
    if (suggestedFuel < 0)
    {
        cout << "Cannot add package -- one or both cities are missing.\n";
        cout << "Add the city first via 'Add city', then retry.\n";
        return;
    }

    cout << "Suggested Price: Rs. " << suggestedFuel << "\n";
    cout << "Enter your price (0 to use suggested): ";
    cin >> priceChoice;
    if (cin.fail() || priceChoice < 0)
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid price. Aborting.\n";
        return;
    }
    if (priceChoice == 0)
        priceChoice = suggestedFuel;

    cout << "Enter Hotel: ";
    cin.ignore();
    getline(cin, hotel);
    if (hotel.empty())
    {
        cout << "Hotel name cannot be empty. Aborting.\n";
        return;
    }

    cout << "Enter Capacity: ";
    cin >> capacity;
    if (cin.fail() || capacity <= 0)
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Capacity must be a positive integer. Aborting.\n";
        return;
    }

    cout << "Enter Category (1.Northern Areas, 2.South, 3.Central, 4.Coastal): ";
    cin >> categoryChoice;
    if (cin.fail() || categoryChoice < 1 || categoryChoice > 4)
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Category must be 1-4. Aborting.\n";
        return;
    }
    Category categoryInput = intToCategory(categoryChoice);
    string categoryStr = categoryToString(categoryInput);

    string date = readValidDate("Enter Departure Date (YYYY-MM-DD): ");
    if (date.empty())
        return;

    string expiry = readValidDate("Enter Expiry Date   (YYYY-MM-DD): ");
    if (expiry.empty())
        return;

    if (expiry < date)
    {
        cout << "Expiry date cannot be before departure date. Aborting.\n";
        return;
    }

    string id = generateUniquePackageID(fromNorm, toNorm);
    cout << "Auto-generated Package ID: " << id << "\n";

    setPackageID(id);
    setStart(fromNorm);
    setDestination(toNorm);
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
            isEmpty = true;
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

    packageOut << id << "," << fromNorm << "," << toNorm << ","
               << priceChoice << "," << hotel << "," << capacity << ","
               << date << "," << expiry << "," << categoryStr << "\n";
    packageOut.close();
    cout << "Package added successfully!\n";

    broadcastNotification("New package available: " + id +
                          " (" + fromNorm + " -> " + toNorm + ")");
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

    bool found = false;
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
         << setw(18) << "Seats"
         << setw(12) << "Departure"
         << setw(12) << "Expiry"
         << setw(10) << "Category"
         << "\n";
    cout << string(103, '-') << "\n";

    while (getline(file, line))
    {
        if (line.empty())
            continue;
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
        try
        {
            total = stoi(cap);
        }
        catch (...)
        {
            total = 0;
        }

        int booked = 0;
        ifstream bf("bookings.csv");
        if (bf.is_open())
        {
            string bline;
            getline(bf, bline); // header
            while (getline(bf, bline))
            {
                if (bline.empty())
                    continue;
                stringstream bs(bline);
                string bid, uname, pkgID;
                getline(bs, bid, ',');
                getline(bs, uname, ',');
                getline(bs, pkgID, ',');
                while (!pkgID.empty() && (pkgID.back() == '\r' || pkgID.back() == '\n'))
                    pkgID.pop_back();
                if (pkgID == id)
                    booked++;
            }
            bf.close();
        }

        string bar;
        for (int i = 0; i < 10; i++)
            bar += (total > 0 && i < (booked * 10 / total)) ? "#" : "-";

        string seatStr = "[" + bar + "]" + to_string(booked) + "/" + to_string(total);

        cout << left
             << setw(12) << id
             << setw(12) << from
             << setw(12) << to
             << setw(10) << ("Rs." + price)
             << setw(17) << hotel
             << setw(18) << seatStr
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
    tm *now = localtime(&t);
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
    if (!fileOut.is_open())
    {
        cout << "Cannot open temp_packages.csv for writing!\n";
        fileIn.close();
        return;
    }

    string line;
    int removedCount = 0;

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
    return true;
}
