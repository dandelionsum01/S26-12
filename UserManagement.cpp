#include "S26-12_header.h"
using namespace std;
void UserManagement::setUsername(string username)
{
	this->username = username;
}
void  UserManagement::setPassword(string password)
{
	this->password = password;
}
string UserManagement::getUsername()
{
	return username;
}
string UserManagement::getPassword()
{
	return password;
}
void UserManagement::countdownTimer(int& set)
{
	int time = 30 * pow(2, set);
	system("cls");
	cout << "\n";
	while (time >= 0)
	{
		if (time < 10)
		{
			cout << "Try again after 0" << time << " seconds" << "\r";
		}
		else
		{
			cout << "Try again after " << time << " seconds" << "\r";
		}
		this_thread::sleep_for(chrono::seconds(1));
		time--;
	}
	system("cls");
	set++;
}