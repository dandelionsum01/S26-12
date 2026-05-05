#include "header.h"
using namespace std;
Accounts::Accounts()
{
	netRevenue = 0;
}
void Accounts::calculateRevenue()
{
	int payment, delimPos;
	string line;
	ifstream fileIn("BookingPayments.csv");
	if (!fileIn.is_open())
	{
		cout << "Error: File failed to open\n";
		return;
	}
	getline(fileIn, line);
	while (getline(fileIn, line))
	{
		payment = 0;
		delimPos = 0;
		for (int i = 0; line[i] != ','; i++)
		{
			delimPos++;
		}
		for (int i = delimPos + 1; line[i] != ','; i++)
		{
			payment = payment * 10 + (line[i] - '0');
		}
		netRevenue += payment;
	}
	fileIn.close();
}
void Accounts::displayRevenue()
{
	cout << "\nNet Revenue: " << netRevenue << "\n";
}
payData* Accounts::readPayments(int& numPayments)
{
	int payment, delimPos = 0, payIn = 0;
	payData* paydata;
	string line;
	ifstream fileIn("BookingPayments.csv");
	if (!fileIn.is_open())
	{
		cout << "\nError: File failed to open\n";
		return nullptr;
	}
	getline(fileIn, line);
	while (getline(fileIn, line))
	{
		numPayments++;
	}
	paydata = new payData[numPayments];
	fileIn.clear();
	fileIn.seekg(0, ios::beg);
	getline(fileIn, line);
	while (getline(fileIn, line))
	{
		delimPos = 0;
		payment = 0;
		for (int i = 0; line[i] != ','; i++)
		{
			paydata[payIn].username = paydata[payIn].username + line[i];
			delimPos++;
		}
		for (int i = delimPos + 1; line[i] != ','; i++)
		{
			payment = payment * 10 + (line[i] - '0');
		}
		paydata[payIn].payment = payment;
		payIn++;
	}
	fileIn.close();
	return paydata;
}
int Accounts::partition(payData* paydata, int start, int end)
{
	int pivot = paydata[end].payment;
	int i = start - 1;
	for (int j = start; j < end; j++)
	{
		if (paydata[j].payment < pivot)
		{
			i++;
			payData temp = paydata[i];
			paydata[i] = paydata[j];
			paydata[j] = temp;
		}
	}
	i++;
	payData temp = paydata[i];
	paydata[i] = paydata[end];
	paydata[end] = temp;
	return i;
}
void Accounts::quicksortPayments(payData* paydata, int start, int end)
{
	if (start >= end)
	{
		return;
	}
	int pivot = partition(paydata, start, end);
	quicksortPayments(paydata, start, pivot - 1);
	quicksortPayments(paydata, pivot + 1, end);
}