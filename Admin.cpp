#include "S26-12_header.h"
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
	string line, username;
	if (!fileIn.is_open())
	{
		cout << "\nError: file failed to open\n";
	}
	else
	{
		getline(fileIn, line);
		getline(fileIn, line);
		for (int i = 0; line[i] != ','; i++)
		{
			username = username + line[i];
		}
		if (username == getUsername())
		{
			fileIn.close();
			return true;
		}
		cout << "\nUsername not found\n";
		fileIn.close();
	}
	return false;
}
bool Admin::checkPassword()
{
	ifstream fileIn("AdminInfo.csv");
	string line, password;
	if (!fileIn.is_open())
	{
		cout << "\nError: file failed to open\n";
	}
	else
	{
		getline(fileIn, line);
		getline(fileIn, line);
		password = "";
		int delimPos = 0;
		for (int i = 0; line[i] != ','; i++)
		{
			delimPos++;
		}
		for (int i = delimPos + 1; i < line.length(); i++)
		{
			password = password + line[i];
		}
		if (password == getPassword())
		{
			fileIn.close();
			return true;
		}
		fileIn.close();
	}
	return false;
}
void Admin::findRevenue(Accounts* accounts)
{
	this->accounts = accounts;
	this->accounts->calculateRevenue();
	this->accounts->displayRevenue();
}
void Admin::sortedPayments(Accounts* accounts)
{
	int numPayments = 0;
	payData* paydata = accounts->readPayments(numPayments);
	accounts->quicksortPayments(paydata, 0, numPayments - 1);
	cout << "Payments from largest to smallest: " << "\n\n";
	for (int i = 0; i < numPayments; i++)
	{
		cout << "Payment " << i + 1 << "| " << right << setw(15) << paydata[i].username << " | " << right << setw(15) << paydata[i].payment << "\n";
	}
	delete[] paydata;
}