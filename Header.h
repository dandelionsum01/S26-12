#pragma once
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <fstream>
#include <chrono>
#include <thread>
#include <cmath>
#include <iomanip>
#include <SFML/Graphics.hpp>
using namespace std;
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