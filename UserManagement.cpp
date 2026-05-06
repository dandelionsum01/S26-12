#include "header.h"
using namespace std;

void UserManagement::setUsername(const string& username)
{
    this->username = username;
}
void UserManagement::setPassword(const string& password)
{
    this->password = password;
}
string UserManagement::getUsername() const
{
    return username;
}
string UserManagement::getPassword() const
{
    return password;
}

void UserManagement::countdownTimer(int& set)
{
    int t = 30 * static_cast<int>(pow(2, set));
    system("cls");
    while (t >= 0)
    {
        if (t < 10)
        {
            cout << "Try again after 0" << t << " seconds" << "\r";
        }
        else
        {
            cout << "Try again after " << t << " seconds" << "\r";
        }
        cout.flush();
        this_thread::sleep_for(chrono::seconds(1));
        t--;
    }
    system("cls");
    set++;
}
