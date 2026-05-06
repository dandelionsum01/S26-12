#define _USE_MATH_DEFINES
#include "header.h"
#include <cctype>
using namespace std;

// ----------------------------------------------------------------
//  Local helpers (city-name normalization & URL encoding).
//  Anonymous namespace -> not visible to other .cpp files.
// ----------------------------------------------------------------
namespace
{
    // Case-insensitive, whitespace-tolerant key for comparison.
    string normalizeCityName(string s)
    {
        // trim trailing CR / LF / spaces / tabs
        while (!s.empty() && (s.back() == '\r' || s.back() == '\n' ||
            s.back() == ' ' || s.back() == '\t'))
            s.pop_back();
        // trim leading spaces / tabs
        while (!s.empty() && (s.front() == ' ' || s.front() == '\t'))
            s.erase(0, 1);
        // lowercase
        for (size_t i = 0; i < s.size(); ++i)
            s[i] = static_cast<char>(
                tolower(static_cast<unsigned char>(s[i])));
        return s;
    }

    // Replace spaces with %20 so the city name is safe inside a URL
    // (e.g. "Dera Ismail Khan" -> "Dera%20Ismail%20Khan").
    string urlEncodeSpaces(const string& s)
    {
        string out;
        for (char c : s)
        {
            if (c == ' ') out += "%20";
            else          out += c;
        }
        return out;
    }
}

// ================================================================
//  Online lookup: ask OpenStreetMap (Nominatim) for the
//  coordinates of a city in Pakistan. Returns true on success.
//
//  Requires `curl` on PATH, which is present by default on:
//      - Windows 10 (1803+) / Windows 11
//      - Linux / macOS
//
//  The API is free, no key required, but politely asks every
//  client to send a User-Agent.
// ================================================================
bool fetchCityCoordinates(const string& cityName,
    double& outLat, double& outLon)
{
    const string tempFile = "city_lookup.json";

    string cmd =
        "curl -s -A \"TravelSystemEdu/1.0\" "
        "\"https://nominatim.openstreetmap.org/search?q=" +
        urlEncodeSpaces(cityName) +
        ",Pakistan&format=json&limit=1\" -o " + tempFile;

    int rc = system(cmd.c_str());
    if (rc != 0)
    {
        cout << "Could not reach the geocoding service "
            "(no internet, or curl is not installed).\n";
        return false;
    }

    ifstream f(tempFile);
    if (!f.is_open())
    {
        cout << "Could not read geocoding response.\n";
        return false;
    }

    stringstream buf;
    buf << f.rdbuf();
    string body = buf.str();
    f.close();
    remove(tempFile.c_str());

    // Empty array "[]" means Nominatim found nothing for this query.
    if (body.find("\"lat\"") == string::npos)
    {
        cout << "City not found by online geocoder.\n";
        return false;
    }

    size_t latPos = body.find("\"lat\":\"");
    size_t lonPos = body.find("\"lon\":\"");
    if (latPos == string::npos || lonPos == string::npos)
        return false;

    latPos += 7; // length of '"lat":"'
    lonPos += 7;

    size_t latEnd = body.find('"', latPos);
    size_t lonEnd = body.find('"', lonPos);
    if (latEnd == string::npos || lonEnd == string::npos)
        return false;

    try
    {
        outLat = stod(body.substr(latPos, latEnd - latPos));
        outLon = stod(body.substr(lonPos, lonEnd - lonPos));
    }
    catch (...)
    {
        return false;
    }
    return true;
}

// ================================================================
//  Package implementation
// ================================================================

Package::Package()
{
    packageFee = 0;
    capacity = 0;
}

void Package::setPackageID(const string& packageID) { this->packageID = packageID; }
void Package::setStart(const string& start) { this->start = start; }
void Package::setDestination(const string& destination) { this->destination = destination; }
void Package::setPackageFee(int packageFee) { this->packageFee = packageFee; }
void Package::setHotel(const string& hotel) { this->hotel = hotel; }
void Package::setCapacity(int capacity) { this->capacity = capacity; }
void Package::setDepartureDate(const string& departureDate) { this->departureDate = departureDate; }

string Package::getPackageID()      const { return packageID; }
string Package::getStart()          const { return start; }
string Package::getDestination()    const { return destination; }
int    Package::getPackageFee()     const { return packageFee; }
string Package::getHotel()          const { return hotel; }
int    Package::getCapacity()       const { return capacity; }
string Package::getDepartureDate()  const { return departureDate; }

double Package::calculateDistance(double lat1, double lon1,
    double lat2, double lon2)
{
    lat1 *= M_PI / 180.0;
    lat2 *= M_PI / 180.0;
    lon1 *= M_PI / 180.0;
    lon2 *= M_PI / 180.0;
    const double R = 6371.0;
    double dLat = (lat2 - lat1);
    double dLon = (lon2 - lon1);
    double a = pow(sin(dLat / 2.0), 2) +
        cos(lat1) * cos(lat2) * pow(sin(dLon / 2.0), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return R * c;
}

void Package::setNewcity(const string& cityName, double lat, double lon)
{
    bool isEmpty = false;

    {
        ifstream checkFile("cities.csv");
        if (checkFile.is_open())
        {
            checkFile.seekg(0, ios::end);
            isEmpty = (checkFile.tellg() == 0);

            if (!isEmpty)
            {
                checkFile.clear();
                checkFile.seekg(0, ios::beg);
                string line;
                getline(checkFile, line); // header
                while (getline(checkFile, line))
                {
                    if (line.empty()) continue;
                    stringstream ss(line);
                    string name;
                    getline(ss, name, ',');

                    // Case-insensitive, whitespace-tolerant comparison.
                    // Catches "Karachi", "karachi", "  KARACHI " etc.
                    if (normalizeCityName(name) ==
                        normalizeCityName(cityName))
                    {
                        cout << "'" << cityName
                            << "' already exists in cities.csv "
                            "(stored as '" << name << "').\n";
                        return;
                    }
                }
            }
        }
        else
        {
            isEmpty = true;
        }
    }

    ofstream cityFile("cities.csv", ios::app);
    if (!cityFile.is_open())
    {
        cout << "Cannot open cities.csv!\n";
        return;
    }

    if (isEmpty)
        cityFile << "City Name,Latitude,Longitude\n";

    cityFile << cityName << "," << lat << "," << lon << "\n";
    cityFile.close();
    cout << "City '" << cityName << "' added!\n";
}

City* Package::loadCities(int& count)
{
    count = 0;
    ifstream cityFile("cities.csv");
    if (!cityFile.is_open())
    {
        cout << "Cannot open cities.csv!\n";
        return nullptr;
    }

    string line;
    getline(cityFile, line); // header
    while (getline(cityFile, line))
    {
        if (!line.empty()) count++;
    }
    cityFile.close();

    if (count == 0) return nullptr;

    City* cities = new City[count];
    ifstream cityFile2("cities.csv");
    int i = 0;
    getline(cityFile2, line); // header
    while (getline(cityFile2, line) && i < count)
    {
        if (line.empty()) continue;

        stringstream ss(line);
        string name, lat, lon;
        getline(ss, name, ',');
        getline(ss, lat, ',');
        getline(ss, lon, ',');

        try
        {
            cities[i].name = name;
            cities[i].lat = stod(lat);
            cities[i].lon = stod(lon);
            i++;
        }
        catch (const exception&)
        {
            // Skip malformed row
        }
    }
    cityFile2.close();
    count = i;
    return cities;
}

int Package::displayRouteInfo(const string& From, const string& To)
{
    int count = 0;
    City* cities = loadCities(count);
    if (cities == nullptr)
    {
        cout << "No cities found!\n";
        return 0;
    }

    City fromCity{}, toCity{};
    bool fromFound = false, toFound = false;
    for (int i = 0; i < count; i++)
    {
        if (cities[i].name == From) { fromCity = cities[i]; fromFound = true; }
        if (cities[i].name == To) { toCity = cities[i]; toFound = true; }
    }
    delete[] cities;

    if (!fromFound || !toFound)
    {
        cout << "City not found!\n";
        return 0;
    }

    double distance = calculateDistance(fromCity.lat, fromCity.lon,
        toCity.lat, toCity.lon);
    double travelTime = distance / 80.0;
    int    Travelhours = static_cast<int>(travelTime);
    int    Travelmins = static_cast<int>((travelTime - Travelhours) * 60);
    double FuelPerLitre = 390.0;
    double FuelEfficiency = 5.0;
    int    FuelCost = static_cast<int>((distance / FuelEfficiency) * FuelPerLitre);

    cout << "\n--- Route Information ---\n";
    cout << "From:        " << From << "\n";
    cout << "To:          " << To << "\n";
    cout << "Distance:    " << static_cast<int>(distance) << " km\n";
    cout << "Travel Time: ~" << Travelhours << " hrs " << Travelmins << " mins\n";
    cout << "Fuel Cost:   Rs. " << FuelCost << "\n";
    cout << "-------------------------\n";
    return FuelCost;
}

void Package::displayCities()
{
    int count = 0;
    City* cities = loadCities(count);
    if (count == 0)
    {
        cout << "No cities found!\n";
        delete[] cities;
        return;
    }

    cout << "\n--- Available Cities ---\n";
    for (int i = 0; i < count; i++)
        cout << i + 1 << ". " << cities[i].name << "\n";

    delete[] cities;
}

int Package::getDynamicPrice()
{
    // Dynamic pricing logic is centralised in Booking::makeBooking
    // (which has live capacity info from bookings.csv).
    return getPackageFee();
}
