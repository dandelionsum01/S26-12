#include "header.h"
using namespace std;

CustomPackage::CustomPackage()
{
    packageApproval = false;
    customerUsername = "";
    nights = 0;
    carRental = false;
    status = PENDING;
    hotelCategory = HotelCategory::STANDARD;
}

void CustomPackage::setNights(int nights) { this->nights = nights; }
void CustomPackage::setCarRental(bool carRental) { this->carRental = carRental; }
void CustomPackage::setCustomerUsername(const string& u) { this->customerUsername = u; }
void CustomPackage::setStatus(PackageStatus status) { this->status = status; }
void CustomPackage::setHotelCategory(HotelCategory hc) { this->hotelCategory = hc; }

int           CustomPackage::getNights()           const { return nights; }
bool          CustomPackage::getCarRental()        const { return carRental; }
string        CustomPackage::getCustomerUsername() const { return customerUsername; }
PackageStatus CustomPackage::getStatus()           const { return status; }
HotelCategory CustomPackage::getHotelCategory()    const { return hotelCategory; }

bool CustomPackage::checkCapacity()
{
    return true; // No capacity model defined for custom packages.
}

HotelCategory CustomPackage::intToHotelCategory(int choice)
{
    switch (choice)
    {
    case 1: return HotelCategory::BUDGET;
    case 2: return HotelCategory::STANDARD;
    case 3: return HotelCategory::LUXURY;
    default: return HotelCategory::STANDARD;
    }
}

string CustomPackage::CategoryToString(HotelCategory choice)
{
    switch (choice)
    {
    case HotelCategory::BUDGET:   return "BUDGET";
    case HotelCategory::STANDARD: return "STANDARD";
    case HotelCategory::LUXURY:   return "LUXURY";
    default:                      return "STANDARD";
    }
}

int CustomPackage::getHotelPrice() const
{
    switch (hotelCategory)
    {
    case HotelCategory::BUDGET:   return 1500;
    case HotelCategory::STANDARD: return 3000;
    case HotelCategory::LUXURY:   return 6000;
    default:                      return 3000;
    }
}

int CustomPackage::calculateBill()
{
    int hotelCostPerNight = getHotelPrice();
    int carCostPerDay = 3500;
    int routeCostPerKm = 10;

    int    count = 0;
    City* cities = loadCities(count);
    double distance = 0;

    if (cities != nullptr)
    {
        City fromCity{}, toCity{};
        bool foundFrom = false, foundTo = false;

        for (int i = 0; i < count; i++)
        {
            if (cities[i].name == getStart()) { fromCity = cities[i]; foundFrom = true; }
            if (cities[i].name == getDestination()) { toCity = cities[i]; foundTo = true; }
        }
        delete[] cities;

        if (foundFrom && foundTo)
            distance = calculateDistance(fromCity.lat, fromCity.lon,
                toCity.lat, toCity.lon);
    }

    int hotelCost = hotelCostPerNight * nights;
    int carCost = carRental ? (carCostPerDay * nights) : 0;
    int routeCost = static_cast<int>(distance * routeCostPerKm);
    int total = hotelCost + carCost + routeCost;

    cout << "\n--- Bill Breakdown ---\n";
    cout << "Hotel (" << nights << " nights):    Rs. " << hotelCost << "\n";
    if (carRental)
        cout << "Car Rental (" << nights << " days):  Rs. " << carCost << "\n";
    cout << "Route Cost:           Rs. " << routeCost << "\n";
    cout << "----------------------\n";
    cout << "Total:                Rs. " << total << "\n";

    return total;
}

void CustomPackage::enterDetails(const string& customerUsername)
{
    string from, to, date;
    int    nightsInput, hotelChoice;
    char   carInput;

    cout << "\n--- Custom Package Request ---\n";
    cout << "Enter Start City: ";
    // Caller may have left a newline pending; handle both cases.
    if (cin.peek() == '\n') cin.ignore();
    getline(cin, from);

    cout << "Enter Destination: ";
    getline(cin, to);

    cout << "Select Hotel Category:\n";
    cout << "1. Budget   (Rs. 1500/night)\n";
    cout << "2. Standard (Rs. 3000/night)\n";
    cout << "3. Luxury   (Rs. 6000/night)\n";
    cout << "Choice: ";
    cin >> hotelChoice;

    setHotelCategory(intToHotelCategory(hotelChoice));
    string hotelStr = CategoryToString(getHotelCategory());

    cout << "Enter Number of Nights: ";
    cin >> nightsInput;
    if (nightsInput <= 0)
    {
        cout << "Number of nights must be positive. Aborting request.\n";
        return;
    }

    cout << "Need Car Rental? (y/n): ";
    cin >> carInput;
    cout << "Enter Departure Date (YYYY-MM-DD): ";
    cin >> date;

    setCustomerUsername(customerUsername);
    setStart(from);
    setDestination(to);
    setNights(nightsInput);
    setCarRental(carInput == 'y' || carInput == 'Y');
    setDepartureDate(date);
    setStatus(PENDING);

    int total = calculateBill();

    cout << "\nTotal Estimate: Rs. " << total << "\n";
    cout << "Submit this request? (y/n): ";
    char confirm;
    cin >> confirm;

    if (confirm != 'y' && confirm != 'Y')
    {
        cout << "Request cancelled.\n";
        return;
    }

    // Generate a safe ID even when usernames or destinations are short.
    auto safeSubstr = [](const string& s, size_t n) -> string {
        return s.substr(0, min(n, s.size()));
        };
    string id = "C-" + safeSubstr(customerUsername, 3) + safeSubstr(to, 3);
    setPackageID(id);

    bool isEmpty = false;
    {
        ifstream checkFile("custom_packages.csv");
        if (checkFile.is_open())
        {
            checkFile.seekg(0, ios::end);
            isEmpty = (checkFile.tellg() == 0);
        }
        else
        {
            isEmpty = true;
        }
    }

    ofstream file("custom_packages.csv", ios::app);
    if (!file.is_open())
    {
        cout << "Cannot open custom_packages.csv for writing!\n";
        return;
    }

    if (isEmpty)
        file << "PackageID,CustomerUsername,Start,Destination,HotelCategory,"
        "Nights,CarRental,DepartureDate,TotalCost,Status\n";

    file << id << "," << customerUsername << ","
        << from << "," << to << ","
        << hotelStr << "," << nightsInput << ","
        << (carInput == 'y' || carInput == 'Y' ? "Yes" : "No") << ","
        << date << "," << total << ",PENDING\n";
    file.close();

    cout << "Request submitted! ID: " << id << "\n";
    cout << "Waiting for Admin approval...\n";
}

void CustomPackage::viewPending()
{
    ifstream file("custom_packages.csv");
    if (!file.is_open())
    {
        cout << "No pending requests found!\n";
        return;
    }

    string line;
    getline(file, line); // header

    cout << "\n--- Pending Custom Requests ---\n";
    cout << left
        << setw(10) << "ID"
        << setw(15) << "Customer"
        << setw(12) << "From"
        << setw(12) << "To"
        << setw(8) << "Nights"
        << setw(8) << "Car"
        << setw(10) << "Status"
        << "\n";
    cout << string(75, '-') << "\n";

    bool any = false;
    while (getline(file, line))
    {
        stringstream ss(line);
        string id, customer, from, to, hotel, nights, car, date, bill, status;
        getline(ss, id, ',');
        getline(ss, customer, ',');
        getline(ss, from, ',');
        getline(ss, to, ',');
        getline(ss, hotel, ',');
        getline(ss, nights, ',');
        getline(ss, car, ',');
        getline(ss, date, ',');
        getline(ss, bill, ',');
        getline(ss, status, ',');

        // Strip CR/LF
        while (!status.empty() && (status.back() == '\r' || status.back() == '\n'))
            status.pop_back();

        if (status == "PENDING")
        {
            any = true;
            cout << left
                << setw(10) << id
                << setw(15) << customer
                << setw(12) << from
                << setw(12) << to
                << setw(8) << nights
                << setw(8) << car
                << setw(10) << status
                << "\n";
        }
    }
    file.close();
    if (!any) cout << "(none)\n";
}

bool CustomPackage::approvePackage()
{
    string targetID;
    cout << "Enter Package ID to approve: ";
    cin >> targetID;

    ifstream fileIn("custom_packages.csv");
    if (!fileIn.is_open())
    {
        cout << "Cannot open custom_packages.csv!\n";
        return false;
    }

    ofstream fileOut("temp_custom.csv");
    if (!fileOut.is_open())
    {
        cout << "Cannot open temp_custom.csv for writing!\n";
        fileIn.close();
        return false;
    }

    string line;
    bool   found = false;
    bool   processed = false;

    getline(fileIn, line); // header
    fileOut << line << "\n";

    while (getline(fileIn, line))
    {
        // After we've handled the target row, just copy the rest verbatim.
        if (processed)
        {
            fileOut << line << "\n";
            continue;
        }

        stringstream ss(line);
        string id, customer, from, to, hotel, nights, car, date, bill, status;
        getline(ss, id, ',');
        getline(ss, customer, ',');
        getline(ss, from, ',');
        getline(ss, to, ',');
        getline(ss, hotel, ',');
        getline(ss, nights, ',');
        getline(ss, car, ',');
        getline(ss, date, ',');
        getline(ss, bill, ',');
        getline(ss, status, ',');

        while (!status.empty() && (status.back() == '\r' || status.back() == '\n'))
            status.pop_back();

        if (id == targetID && status == "PENDING")
        {
            found = true;
            processed = true;

            cout << "\n--- Request Details ---\n";
            cout << "Customer:   " << customer << "\n";
            cout << "From:       " << from << "\n";
            cout << "To:         " << to << "\n";
            cout << "Hotel:      " << hotel << "\n";
            cout << "Nights:     " << nights << "\n";
            cout << "Car:        " << car << "\n";
            cout << "Date:       " << date << "\n";
            cout << "Total Bill: Rs. " << bill << "\n";
            cout << "----------------------\n";
            cout << "Confirm Approval? (y/n): ";
            char confirm;
            cin >> confirm;

            string newStatus;
            if (confirm == 'y' || confirm == 'Y')
            {
                newStatus = "APPROVED";
                setStatus(APPROVED);
                cout << "Package " << targetID << " approved!\n";
            }
            else if (confirm == 'n' || confirm == 'N')
            {
                newStatus = "REJECTED";
                setStatus(REJECTED);
                cout << "Package " << targetID << " rejected!\n";
            }
            else
            {
                // Anything other than y/n: keep the original PENDING line
                // and let the admin know nothing was changed.
                cout << "Invalid choice -- package " << targetID
                    << " left as PENDING.\n";
                fileOut << line << "\n";
                continue;
            }

            fileOut << id << "," << customer << ","
                << from << "," << to << ","
                << hotel << "," << nights << ","
                << car << "," << date << ","
                << bill << "," << newStatus << "\n";
        }
        else
        {
            fileOut << line << "\n";
        }
    }

    fileIn.close();
    fileOut.close();

    if (found)
    {
        remove("custom_packages.csv");
        rename("temp_custom.csv", "custom_packages.csv");
    }
    else
    {
        remove("temp_custom.csv");
        cout << "Package ID not found or already processed!\n";
    }
    return found;
}
