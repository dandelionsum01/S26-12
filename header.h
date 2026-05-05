#pragma once
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
#include <SFML/Graphics.hpp>
using namespace std;

// ============================================================
// MEMBER A —
// ============================================================

class UserManagement
{
    string username;
    string password;

public:
    virtual void signin() = 0;
    virtual bool checkPassword() = 0;
    virtual bool findUsername() = 0;
    void setUsername(string username);
    void setPassword(string password);
    string getUsername();
    string getPassword();
    void countdownTimer(int& set);
};
class Customer : public UserManagement
{
public:
    void signin() override;
    void signup();
    bool findUsername() override;
    bool checkPassword() override;
    bool checkStrength();
    int checkGeneric(string password, int& numGeneric);
};
struct payData
{
    string username;
    int payment;
};
class Accounts
{
    int netRevenue;

public:
    Accounts();
    void calculateRevenue();
    void displayRevenue();
    payData* readPayments(int& numPayments);
    int partition(payData* paydata, int start, int end);
    void quicksortPayments(payData* paydata, int start, int end);
};
class Admin : public UserManagement
{
    Accounts* accounts;

public:
    Admin();
    void signin() override;
    bool findUsername() override;
    bool checkPassword() override;
    void findRevenue(Accounts* accounts);
    void sortedPayments(Accounts* accounts);
};

// ===================
// MEMBER B —
// ====================

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
    int packageFee;
    string hotel;
    int capacity;
    string departureDate;

public:
    Package();
    void setPackageID(string packageID);
    void setStart(string start);
    void setDestination(string destination);
    void setPackageFee(int packageFee);
    void setHotel(string hotel);
    void setCapacity(int capacity);
    void setDepartureDate(string departureDate);
    void setNewcity(string cityName, double lat, double lon);
    City* loadCities(int& count);

    string getPackageID();
    string getStart();
    string getDestination();
    int getPackageFee();
    string getHotel();
    int getCapacity();
    string getDepartureDate();

    double calculateDistance(double lat1, double lon1,
        double lat2, double lon2);
    void displayCities();
    int displayRouteInfo(string from, string to);
    int getDynamicPrice();
    virtual bool checkCapacity() = 0;
};

class PlannedPackage : public Package
{
    string expiryDate;
    int basePrice;
    Category category; //  added

public:
    PlannedPackage();
    Category intToCategory(int choice);
    string categoryToString(Category cat);
    void setExpiryDate(string expiryDate);
    void setBasePrice(int price);
    void setCategory(Category category); // added
    string getExpiryDate();
    int getBasePrice();
    Category getCategory(); // added
    void addPackage();
    void deletePackage();
    void displayPackage();
    string findPackage();
    void removeExpiredPackages();
    bool checkCapacity() override;
};

class CustomPackage : public Package
{
    bool packageApproval;
    string customerUsername;
    int nights;
    bool carRental;
    PackageStatus status; // added

public:
    CustomPackage();
    void setNights(int nights);
    void setCarRental(bool carRental);
    void setCustomerUsername(string username);
    void setStatus(PackageStatus status); // added
    int getNights();
    bool getCarRental();
    string getCustomerUsername();
    PackageStatus getStatus(); // added
    void enterDetails(string customerUsername);
    void viewPending();
    bool approvePackage();
    int calculateBill();
    bool checkCapacity() override;

    HotelCategory hotelCategory; // member
    void setHotelCategory(HotelCategory hc);

    HotelCategory getHotelCategory();
    HotelCategory intToHotelCategory(int choice);
    string CategoryToString(HotelCategory hc);
    int getHotelPrice();
};

// for testing purposes only
class TestPackage : public Package
{
public:
    // We MUST implement this to allow the object to be created
    bool checkCapacity() override
    {
        return true;
    }
};
// =========================
// MEMBER C —
// ==========================

class Booking
{
    int payment;
    string packageID;
    string customerUsername;

public:
    Booking();
    void setPayment(int payment);
    void setPackageID(string packageID);
    void setCustomerUsername(string username);
    int getPayment();
    string getPackageID();
    string getCustomerUsername();
    void makeBooking(string customerUsername);
    void updateCapacity(string packageID);
    int returnPayment(string customerUsername);
    bool checkCreditNum(string cardNum);
};

class CustomerReview
{
    string comment;

public:
    void setComment(string comment);
    string getComment();
    void addReview(string username, string packageID);
    void readReview(string packageID);
};

class NotificationPanel
{
    string notification;
    bool isSubscribed;

public:
    NotificationPanel();
    void setNotification(string notification);
    string getNotification();
    bool getIsSubscribed();
    void subscribe(string username);
    void displayNotification(string username);
};