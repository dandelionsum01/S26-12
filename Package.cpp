
#define _USE_MATH_DEFINES
#include "header.h"
using namespace std;

Package::Package()
{
    packageFee = 0;
    capacity = 0;
}

void Package::setPackageID(const string &packageID)
{
    this->packageID = packageID;
}
void Package::setStart(const string &start)
{
    this->start = start;
}
void Package::setDestination(const string &destination)
{
    this->destination = destination;
}
void Package::setPackageFee(int packageFee)
{
    this->packageFee = packageFee;
}
void Package::setHotel(const string &hotel)
{
    this->hotel = hotel;
}
void Package::setCapacity(int capacity)
{
    this->capacity = capacity;
}
void Package::setDepartureDate(const string &departureDate)
{
    this->departureDate = departureDate;
}

string Package::getPackageID() const
{
    return packageID;
}
string Package::getStart() const
{
    return start;
}
string Package::getDestination() const
{
    return destination;
}
int Package::getPackageFee() const
{
    return packageFee;
}

string Package::getHotel() const
{
    return hotel;
}
int Package::getCapacity() const
{
    return capacity;
}
string Package::getDepartureDate() const
{
    return departureDate;
}
// haversine
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

void Package::setNewcity(const string &cityName, double lat, double lon)
{
    string nice = normalizeCityName(cityName);
    if (nice.empty())
    {
        cout << "City name cannot be empty.\n";
        return;
    }
    string niceLower = toLowerCity(nice);

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
                    if (line.empty())
                        continue;
                    stringstream ss(line);
                    string name;
                    getline(ss, name, ',');

                    if (toLowerCity(name) == niceLower)
                    {
                        cout << nice << " already exists!\n";
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

    cityFile << nice << "," << lat << "," << lon << "\n";
    cityFile.close();
    cout << "City '" << nice << "' added!\n";
}

City *Package::loadCities(int &count)
{
    count = 0;
    ifstream cityFile("cities.csv");
    if (!cityFile.is_open())
    {
        cout << "Cannot open cities.csv!\n";
        return nullptr;
    }

    string line;
    getline(cityFile, line); // skip header row
    while (getline(cityFile, line))
    {
        if (!line.empty())
            count++; // Count each valid data row
    }
    cityFile.close();

    // If no city records found, return nullptr
    if (count == 0)
        return nullptr;

    City *cities = new City[count];
    ifstream cityFile2("cities.csv");
    int i = 0;
    getline(cityFile2, line); // skip header row
    while (getline(cityFile2, line) && i < count)
    {
        if (line.empty())
            continue; // Skip empty lines

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
        catch (const exception &)
        {
        }
    }
    cityFile2.close();
    count = i;
    return cities;
}

int Package::displayRouteInfo(const string &From, const string &To)
{
    int count = 0;
    City *cities = loadCities(count);
    if (cities == nullptr)
    {
        cout << "No cities found!\n";
        return -1;
    }

    string fromLower = toLowerCity(From);
    string toLower = toLowerCity(To);

    City fromCity{}, toCity{};
    bool fromFound = false, toFound = false;
    for (int i = 0; i < count; i++)
    {
        if (toLowerCity(cities[i].name) == fromLower)
        {
            fromCity = cities[i];
            fromFound = true;
        }
        if (toLowerCity(cities[i].name) == toLower)
        {
            toCity = cities[i];
            toFound = true;
        }
    }
    delete[] cities;

    if (!fromFound || !toFound)
    {
        if (!fromFound)
            cout << "Start city '" << From << "' not found in cities.csv!\n";
        if (!toFound)
            cout << "Destination '" << To << "' not found in cities.csv!\n";
        return -1;
    }

    double distance = calculateDistance(fromCity.lat, fromCity.lon,
                                        toCity.lat, toCity.lon);
    double travelTime = distance / 80.0;
    int Travelhours = static_cast<int>(travelTime);
    int Travelmins = static_cast<int>((travelTime - Travelhours) * 60);
    double FuelPerLitre = 390.0;
    double FuelEfficiency = 5.0;
    int FuelCost = static_cast<int>((distance / FuelEfficiency) * FuelPerLitre);

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
    City *cities = loadCities(count);
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

// return the current package fee
int Package::getDynamicPrice()
{
    return getPackageFee();
}