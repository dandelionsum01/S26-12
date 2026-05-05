#define _USE_MATH_DEFINES
#include <iostream>
#include "header.h"
#include <fstream>
#include <string>
#include <cmath>
using namespace std;

Package::Package()
{
    packageFee = 0;
    capacity = 0;
}
void Package::setPackageID(string packageID)
{
    this->packageID = packageID;
}
void Package::setStart(string start)
{
    this->start = start;
}
void Package::setDestination(string destination)
{
    this->destination = destination;
}
void Package::setPackageFee(int packageFee)
{
    this->packageFee = packageFee;
}
void Package::setHotel(string hotel)
{
    this->hotel = hotel;
}
void Package::setCapacity(int capacity)
{
    this->capacity = capacity;
}
void Package::setDepartureDate(string departureDate)
{
    this->departureDate = departureDate;
}
string Package::getPackageID()
{
    return packageID;
}
string Package::getStart()
{
    return start;
}
string Package::getDestination()
{
    return destination;
}
int Package::getPackageFee()
{
    return packageFee;
}
string Package::getHotel()
{
    return hotel;
}
int Package::getCapacity()
{
    return capacity;
}
string Package::getDepartureDate()
{
    return departureDate;
}
double Package::calculateDistance(double lat1, double lon1,
    double lat2, double lon2)
{
    lat1 *= M_PI / 180.0; // M_PI needs library cmath
    lat2 *= M_PI / 180.0;
    lon1 *= M_PI / 180.0;
    lon2 *= M_PI / 180.0;
    const double R = 6371; // Earth radius in km
    double dLat = (lat2 - lat1);
    double dLon = (lon2 - lon1);
    double a = pow(sin(dLat / 2), 2) + cos(lat1) * cos(lat2) * pow(sin(dLon / 2), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    double d = R * c;
    return d;
}
void Package::setNewcity(string cityName, double lat, double lon)
{

    // checing if city already exists
    bool isEmpty = 0;
    ifstream checkFile("cities.csv");
    if (checkFile.is_open())
    {
        checkFile.seekg(0, ios::end); ////chhecking if file is empty
        isEmpty = (checkFile.tellg() == 0);
        if (isEmpty == 1)
        {
            checkFile.close();
        }
        else
        {
            string line;
            getline(checkFile, line); // skip header
            while (getline(checkFile, line))
            {
                stringstream ss(line);
                string name;
                getline(ss, name, ',');
                if (name == cityName)
                {
                    cout << cityName << " already exists!\n";
                    checkFile.close();
                    return;
                }
            }
            checkFile.close();
        }
    }

    //  header only if file is empty
    ofstream cityFile("cities.csv", ios::app);
    if (!cityFile.is_open())
    {
        cout << "Cannot open cityFile!\n";
        return;
    }

    if (isEmpty)
    {
        cityFile << "City Name,Latitude,Longitude\n"; // header once
    }

    //  Append the new city
    cityFile << cityName << "," << lat << "," << lon << "\n";
    cityFile.close();
    cout << "City '" << cityName << "' added!\n";
}

City* Package::loadCities(int& count)
{
    ifstream cityFile("cities.csv");
    if (!cityFile.is_open())
    {
        cout << "Cannot open cityFile!\n";
        count = 0;
        return nullptr;
    }

    string line;
    getline(cityFile, line); // skip header
    while (getline(cityFile, line))
    {
        count++;
    }
    cityFile.close();

    City* cities = new City[count];

    ifstream cityFile2("cities.csv");
    int i = 0;
    getline(cityFile2, line);
    while (getline(cityFile2, line))
    {
        string lat, lon;
        string name;
        stringstream ss(line);
        getline(ss, cities[i].name, ',');
        getline(ss, lat, ',');
        getline(ss, lon, ',');
        cities[i].lat = stod(lat); // string to double
        cities[i].lon = stod(lon);
        i++;
    }
    cityFile2.close();
    return cities;
}

int Package::displayRouteInfo(string From, string To)
{
    int count = 0;
    City* cities = loadCities(count);
    if (cities == nullptr)
    {
        cout << "No cities found!\n";
        return 0;
    }
    City fromCity, toCity; //
    bool fromFound = false, toFound = false;
    for (int i = 0; i < count; i++)
    {
        if (cities[i].name == From) // if cityname  matches
        {
            fromCity = cities[i]; // then all atributs of that city are stored in fromCity
            fromFound = true;
        }
        if (cities[i].name == To)
        {
            toCity = cities[i];
            toFound = true;
        }
    }
    delete[] cities; // freeing the heap of loaded cities
    if (!fromFound || !toFound)
    { // if either city is not found
        cout << "City not found!\n";
        return 0;
    }
    double distance = calculateDistance(fromCity.lat, fromCity.lon, toCity.lat, toCity.lon);
    double travelTime = distance / 80.0;              // eg 10.17 hrs
    int Travelhours = (int)travelTime;                // eg 10
    int Travelmins = (travelTime - Travelhours) * 60; // 10 minutes
    double FuelPerLitre = 390;
    double FuelEfficiency = 5;                                 // 5km per litre
    int FuelCost = (distance / FuelEfficiency) * FuelPerLitre; // eg 3120

    cout << "\n--- Route Information ---\n";
    cout << "From:        " << From << "\n";
    cout << "To:          " << To << "\n";
    cout << "Distance:    " << (int)distance << " km\n";
    cout << "Travel Time: ~" << Travelhours << " hrs " << Travelmins << " mins\n";
    cout << "Fuel Cost:   Rs. " << FuelCost << "\n";
    cout << "-------------------------\n";
    return FuelCost;
}

void Package::displayCities()
{
    int count = 0;
    City* cities = loadCities(count);
    if (count == 0) { cout << "No cities found!\n"; return; }

    cout << "\n--- Available Cities ---\n";
    for (int i = 0; i < count; i++)
        cout << i + 1 << ". " << cities[i].name << "\n";

    delete[] cities;
}

int Package::getDynamicPrice()
{
    // if more than 80% booked → increase price by 20%
    // if less than 50% booked → decrease price by 10%
    //waiting for member 3 booking
    return getPackageFee();
}