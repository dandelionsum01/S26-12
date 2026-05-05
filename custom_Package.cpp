#include <iostream>
#include "header.h"
using namespace std;

HotelCategory CustomPackage::intToHotelCategory(int choice)
{
    switch (choice)
    {
    case 1:
        return HotelCategory::BUDGET;
    case 2:
        return HotelCategory::STANDARD;
    case 3:
        return HotelCategory::LUXURY;
    default:
        return HotelCategory::STANDARD;
    }
}
string CustomPackage::CategoryToString(HotelCategory choice)
{
    switch (choice)
    {
    case HotelCategory::BUDGET:
        return "BUDGET";
    case HotelCategory::STANDARD:
        return "STANDARD";
    case HotelCategory::LUXURY:
        return "LUXURY";
    default:
        return "STANDARD";
    }
}
int CustomPackage::getHotelPrice()
{
    switch (hotelCategory)
    {
    case HotelCategory::BUDGET:
        return 1500;
    case HotelCategory::STANDARD:
        return 3000;
    case HotelCategory::LUXURY:
        return 6000;
    default:
        return 3000;
    }
}

CustomPackage::CustomPackage()
{
    packageApproval = false;
    customerUsername = "";
    nights = 0;
    carRental = false;
    status = PENDING;
}

void CustomPackage::setNights(int nights)
{
    this->nights = nights;
}
void CustomPackage::setCarRental(bool carRental)
{
    this->carRental = carRental;
}
void CustomPackage::setCustomerUsername(string username)
{
    this->customerUsername = username;
}
void CustomPackage::setStatus(PackageStatus status)
{
    this->status = status;
}
int CustomPackage::getNights()
{
    return nights;
}
bool CustomPackage::getCarRental()
{
    return carRental;
}
string CustomPackage::getCustomerUsername()
{
    return customerUsername;
}
PackageStatus CustomPackage::getStatus()
{
    return status;
}
bool CustomPackage::checkCapacity()
{
    return true; // member 3's needed ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
void CustomPackage::setHotelCategory(HotelCategory hc)
{
    this->hotelCategory = hc;
}

HotelCategory CustomPackage::getHotelCategory()
{
    return hotelCategory;
}
int CustomPackage::calculateBill()
{
    int hotelCostPerNight = getHotelPrice();
    int carCostPerDay = 3500;
    int routeCostPerKm = 10;

    // get distance
    int count = 0;
    City* cities = loadCities(count);
    double distance = 0;

    if (cities != nullptr)
    {
        City fromCity, toCity;
        bool foundFrom = false, foundTo = false;

        for (int i = 0; i < count; i++)
        {
            if (cities[i].name == getStart())
            {
                fromCity = cities[i];
                foundFrom = true;
            }
            if (cities[i].name == getDestination())
            {
                toCity = cities[i];
                foundTo = true;
            }
        }
        delete[] cities;

        if (foundFrom && foundTo)
            distance = calculateDistance(fromCity.lat, fromCity.lon,
                toCity.lat, toCity.lon);
    }

    int hotelCost = hotelCostPerNight * nights;
    int carCost = carRental ? (carCostPerDay * nights) : 0;
    int routeCost = distance * routeCostPerKm;
    int total = hotelCost + carCost + routeCost;

    cout << "\n--- Bill Breakdown ---\n";
    cout << "Hotel (" << nights << " nights):   Rs. " << hotelCost << "\n";
    if (carRental)
        cout << "Car Rental (" << nights << " days): Rs. " << carCost << "\n";
    cout << "Route Cost:          Rs. " << routeCost << "\n";
    cout << "--------------------\n";
    cout << "Total:               Rs. " << total << "\n";

    return total;
}

void CustomPackage::enterDetails(string customerUsername)
{
    string from, to, date;
    int nightsInput;
    char carInput;
    string hotelStr;

    cout << "\n--- Custom Package Request ---\n";
    cout << "Enter Start City: ";
    cin.ignore();
    getline(cin, from);
    cout << "Enter Destination: ";
    getline(cin, to);

    // cout << "\n--- Route Information ---\n";
    // cout << "From:        " << from << "\n";
    // cout << "To:          " << to << "\n";
    // cout << "-------------------------\n";

    cout << "Select Hotel Category:\n";
    cout << "1. Budget   (Rs. 1500/night)\n";
    cout << "2. Standard (Rs. 3000/night)\n";
    cout << "3. Luxury   (Rs. 6000/night)\n";
    cout << "Choice: ";
    int hotelChoice;
    cin >> hotelChoice;

    setHotelCategory((intToHotelCategory(hotelChoice)));
    hotelStr = CategoryToString(getHotelCategory());

    cout << "Enter Number of Nights: ";
    cin >> nightsInput;
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

    if (confirm == 'y' || confirm == 'Y')
    {
        // generate ID
        string id = "C-" + customerUsername.substr(0, 3) + to.substr(0, 3);
        setPackageID(id);

        bool isEmpty = 0;
        ifstream checkFile("custom_packages.csv");
        if (checkFile.is_open())
        {
            checkFile.seekg(0, ios::end);
            isEmpty = (checkFile.tellg() == 0);
            checkFile.close();
        }
        else
            cout << "Cannot open custom_packages.csv!\n";

        // write to custom_packages.csv
        ofstream file("custom_packages.csv", ios::app);
        if (file.is_open())
        {

            if (isEmpty)
            {
                file << "PackageID,CustomerUsername,Start,Destination,HotelCategory,Nights,CarRental,DepartureDate,TotalCost,Status\n"; // header
            }

            file << id << "," << customerUsername << ","
                << from << "," << to << ","
                << hotelStr << "," << nightsInput << ","
                << (carInput == 'y' ? "Yes" : "No") << ","
                << date << "," << total << ",PENDING\n";
            file.close();
            cout << "Request submitted! ID: " << id << "\n";
            cout << "Waiting for Admin approval...\n";
        }
    }
    else
        cout << "Request cancelled.\n";
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
    getline(file, line); // skip header

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

        if (status == "PENDING")
        {
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
    string line;
    bool found = false;

    // copy header
    getline(fileIn, line);
    fileOut << line << "\n";

    while (getline(fileIn, line))
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

        if (id == targetID && status == "PENDING")
        {
            found = true;

            cout << "\n--- Request Details ---\n";
            cout << "Customer:  " << customer << "\n";
            cout << "From:      " << from << "\n";
            cout << "To:        " << to << "\n";
            cout << "Hotel:     " << hotel << "\n";
            cout << "Nights:    " << nights << "\n";
            cout << "Car:       " << car << "\n";
            cout << "Date:      " << date << "\n";
            cout << "Total Bill: Rs. " << bill << "\n";
            cout << "----------------------\n";
            cout << "Confirm Approval? (y/n): ";
            char confirm;
            cin >> confirm;

            if (confirm == 'y' || confirm == 'Y')
            {
                fileOut << id << "," << customer << ","
                    << from << "," << to << ","
                    << hotel << "," << nights << ","
                    << car << "," << date << ","
                    << bill << ",APPROVED\n"; // approved
                cout << "Package " << targetID << " approved!\n";
                setStatus(APPROVED);
                break;
            }
            else if (confirm == 'n' || confirm == 'N')
            {
                fileOut << id << "," << customer << ","
                    << from << "," << to << ","
                    << hotel << "," << nights << ","
                    << car << "," << date << ","
                    << bill << ",REJECTED\n"; // R
                cout << "Package " << targetID << " rejected!\n";
                setStatus(REJECTED);
                break;
            }
            else
            {
                fileOut << line << "\n"; //  PENDING
                break;
            }
        }
    }
    while (getline(fileIn, line))
    {
        fileOut << line << "\n"; // copy rest 
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
