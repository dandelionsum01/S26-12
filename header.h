#pragma once

// MUST come before any Windows headers (libcurl pulls them in).
// Avoids the "byte: ambiguous symbol" collision between std::byte
// (from <cstddef>) and ::byte (from <rpcndr.h>).
#define _HAS_STD_BYTE 0

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <fstream>
#include <chrono>
#include <sstream>
#include <thread>
#include <cmath>
#include <iomanip>
#include <stdexcept>
#include <vector>
#include <SFML/Graphics.hpp>

// Note: SFML is only required if you use the GUI reCaptcha. If SFML is not
// available on your machine, define NO_SFML before including this file (or
// pass -DNO_SFML to the compiler) and the code will fall back to a console
// based reCaptcha challenge.

using namespace std;

// ============================================================
//  Project-wide custom exception (Requirement: try/catch/throw)
// ============================================================
class ProjectException : public runtime_error
{
public:
    explicit ProjectException(const string& msg) : runtime_error(msg) {}
};

// ============================================================
//  Generic template helper (Requirement: at least one template)
//  Reads a value from cin; re-prompts on bad input.
// ============================================================
template <typename T>
T readValue(const string& prompt)
{
    T value;
    while (true)
    {
        cout << prompt;
        cin >> value;
        if (cin.fail())
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input, please try again.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return value;
    }
}

// ============================================================
//  MEMBER A  --  User management & accounts
// ============================================================

class UserManagement
{
    string username;
    string password;

public:
    virtual ~UserManagement() = default;
    virtual void signin() = 0;
    virtual bool checkPassword() = 0;
    virtual bool findUsername() = 0;

    void   setUsername(const string& username);
    void   setPassword(const string& password);
    string getUsername() const;
    string getPassword() const;
    void   countdownTimer(int& set);
};

class Customer : public UserManagement
{
public:
    void  signin() override;
    void  signup();
    bool  findUsername() override;
    bool  checkPassword() override;
    bool  checkStrength();
    int   checkGeneric(const string& password, int& numGeneric);
};

struct payData
{
    string username;
    int    payment;
};

class Accounts
{
    int netRevenue;

public:
    Accounts();
    void      calculateRevenue();
    void      displayRevenue();
    payData* readPayments(int& numPayments);
    int       partition(payData* paydata, int start, int end);
    void      quicksortPayments(payData* paydata, int start, int end);
};

class Admin : public UserManagement
{
    Accounts* accounts;

public:
    Admin();
    ~Admin();             
    void signin() override;
    bool findUsername() override;
    bool checkPassword() override;
    void findRevenue();
    void sortedPayments();
};

// ============================================================
//  MEMBER B  --  Packages
// ============================================================

struct City
{
    string name;
    double lat;
    double lon;
};

enum PackageStatus
{
    PENDING,
    APPROVED,
    REJECTED
};

enum class Category
{
    NORTHERN_AREAS,
    SOUTH,
    CENTRAL,
    COASTAL
};

enum class HotelCategory
{
    BUDGET,
    STANDARD,
    LUXURY
};

class Package
{
    string packageID;
    string start;
    string destination;
    int    packageFee;
    string hotel;
    int    capacity;
    string departureDate;

public:
    Package();
    virtual ~Package() = default;

    void setPackageID(const string& packageID);
    void setStart(const string& start);
    void setDestination(const string& destination);
    void setPackageFee(int packageFee);
    void setHotel(const string& hotel);
    void setCapacity(int capacity);
    void setDepartureDate(const string& departureDate);
    void setNewcity(const string& cityName, double lat, double lon);

    City* loadCities(int& count);

    string getPackageID() const;
    string getStart() const;
    string getDestination() const;
    int    getPackageFee() const;
    string getHotel() const;
    int    getCapacity() const;
    string getDepartureDate() const;

    double calculateDistance(double lat1, double lon1,
        double lat2, double lon2);
    void   displayCities();
    int    displayRouteInfo(const string& from, const string& to);
    int    getDynamicPrice();

    virtual bool checkCapacity() = 0;
};

class PlannedPackage : public Package
{
    string   expiryDate;
    int      basePrice;
    Category category;

public:
    PlannedPackage();

    static Category intToCategory(int choice);
    static string   categoryToString(Category cat);

    void     setExpiryDate(const string& expiryDate);
    void     setBasePrice(int price);
    void     setCategory(Category category);

    string   getExpiryDate() const;
    int      getBasePrice() const;
    Category getCategory() const;

    void   addPackage();
    void   deletePackage();
    void   displayPackage();
    string findPackage();
    void   removeExpiredPackages();
    bool   checkCapacity() override;
};

class CustomPackage : public Package
{
    bool          packageApproval;
    string        customerUsername;
    int           nights;
    bool          carRental;
    PackageStatus status;
    HotelCategory hotelCategory;

public:
    CustomPackage();

    void          setNights(int nights);
    void          setCarRental(bool carRental);
    void          setCustomerUsername(const string& username);
    void          setStatus(PackageStatus status);
    void          setHotelCategory(HotelCategory hc);

    int           getNights() const;
    bool          getCarRental() const;
    string        getCustomerUsername() const;
    PackageStatus getStatus() const;
    HotelCategory getHotelCategory() const;

    void enterDetails(const string& customerUsername);
    void viewPending();
    bool approvePackage();
    int  calculateBill();
    bool checkCapacity() override;

    static HotelCategory intToHotelCategory(int choice);
    static string        CategoryToString(HotelCategory hc);
    int                  getHotelPrice() const;
};

// ============================================================
//  MEMBER C  --  Booking, Reviews, Notifications
// ============================================================

class Booking
{
    int    payment;
    string packageID;
    string customerUsername;

public:
    Booking();

    void   setPayment(int payment);
    void   setPackageID(const string& packageID);
    void   setCustomerUsername(const string& username);

    int    getPayment() const;
    string getPackageID() const;
    string getCustomerUsername() const;

    void   makeBooking(const string& customerUsername);
    void   updateCapacity(const string& packageID);
    int    returnPayment(const string& customerUsername);
    bool   checkCreditNum(const string& cardNum);
};

class CustomerReview
{
    string comment;

public:
    void   setComment(const string& comment);
    string getComment() const;
    void   addReview(const string& username, const string& packageID);
    void   readReview(const string& packageID);
};

class NotificationPanel
{
    string notification;
    bool   isSubscribed;

public:
    NotificationPanel();

    void   setNotification(const string& notification);
    string getNotification() const;
    bool   getIsSubscribed() const;

    void   subscribe(const string& username);
    void   displayNotification(const string& username);
};

// Free helper used by other modules to push messages to all subscribers.
void broadcastNotification(const string& message);

// ============================================================
//  Geocoding helpers (Geocoder.cpp)
// ============================================================
// Auto-fetches lat/lon for a city name via OpenStreetMap Nominatim.
// Returns true on success, false on network/parse error or no result.
bool   geocode(const string& cityName, double& lat, double& lon);

// Look up a city in cities.csv (case-insensitive). Fills lat/lon on hit.
bool   findCityInCSV(const string& cityName, double& lat, double& lon);

// Normalize raw user input to a canonical "Title Case" name.
string normalizeCityName(const string& raw);

// ASCII lower-case helper used by the case-insensitive comparisons.
string toLowerCity(const string& s);
