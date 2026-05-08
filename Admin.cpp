#include "header.h"
using namespace std;

Admin::Admin()
{
    accounts = nullptr;
}

void Admin::signin()
{
    int tries = 0, set = 0;
    string username;
    string password;
    cout << "Enter username of admin: \n";
    cin >> username;
    setUsername(username);
    if (!findUsername())
    {
        return;
    }
    system("cls");
    cout << "Enter password of admin: \n";
    cin >> password;
    setPassword(password);
    bool valid = checkPassword();
    while (!valid)
    {
        tries++;
        if (tries == 3)
        {
            countdownTimer(set);
            tries = 0;
        }
        cout << "\nWrong password entered, enter correct password: \n";
        cin >> password;
        setPassword(password);
        valid = checkPassword();
    }
    cout << "\nAdmin login was successful\n";
}

bool Admin::findUsername()
{
    ifstream fileIn("AdminInfo.csv");
    if (!fileIn.is_open())
    {
        cout << "\nError: file failed to open\n";
        return false;
    }
    string line;
    getline(fileIn, line);
    while (getline(fileIn, line))
    {
        string username;
        for (size_t i = 0; i < line.length() && line[i] != ','; ++i)
        {
            username += line[i];
        }
        if (username == getUsername())
        {
            fileIn.close();
            return true;
        }
    }
    fileIn.close();
    cout << "\nUsername not found\n";
    return false;
}

bool Admin::checkPassword()
{
    ifstream fileIn("AdminInfo.csv");
    if (!fileIn.is_open())
    {
        cout << "\nError: file failed to open\n";
        return false;
    }
    string line;
    getline(fileIn, line);
    while (getline(fileIn, line))
    {
        size_t comma = line.find(',');
        if (comma == string::npos)
        {
            continue;
        }
        string username = line.substr(0, comma);
        string password = line.substr(comma + 1);
        while (!password.empty() && (password.back() == '\r' || password.back() == '\n'))
        {
            password.pop_back();
        }
        if (username == getUsername() && password == getPassword())
        {
            fileIn.close();
            return true;
        }
    }
    fileIn.close();
    return false;
}

void Admin::findRevenue(Accounts *accounts)
{
    this->accounts = accounts;
    this->accounts->calculateRevenue();
    this->accounts->displayRevenue();
}

void Admin::sortedPayments(Accounts *accounts)
{
    int numPayments = 0;
    payData *paydata = accounts->readPayments(numPayments);
    if (paydata == nullptr || numPayments == 0)
    {
        cout << "No payments to sort.\n";
        delete[] paydata;
        return;
    }
    accounts->quicksortPayments(paydata, 0, numPayments - 1);
    cout << "Payments ordered from smallest to largest:\n\n";
    for (int i = 0; i < numPayments; ++i)
    {
        cout << "Payment " << i + 1 << "|--" << right << setw(15) << paydata[i].username << "|--" << right << setw(15) << paydata[i].payment << "\n";
    }
    delete[] paydata;
}
