#include "header.h"
using namespace std;

Accounts::Accounts()
{
	netRevenue = 0;
}

void Accounts::calculateRevenue()
{
	netRevenue = 0; 
	ifstream fileIn("BookingPayments.csv");
	if (fileIn.is_open())
	{
		string line;
		getline(fileIn, line);
		while (getline(fileIn, line))
		{
			if (line.empty())
			{
				continue;
			}
			int payment = 0;
			int delimPos = 0;
			for (int i = 0; i < (int)line.length() && line[i] != ','; i++)
			{
				delimPos++;
			}
			for (int i = delimPos + 1; i < (int)line.length() && line[i] != ','; i++)
			{
				if (line[i] >= '0' && line[i] <= '9')
				{
					payment = payment * 10 + (line[i] - '0');
				}
			}
			netRevenue += payment;
		}
		fileIn.close();
	}
	ifstream fileIn("bookings.csv");
	if (fileIn.is_open())
	{
		string line;
		getline(fileIn, line);
		while (getline(fileIn, line))
		{
			if (line.empty())
			{
				continue;
			}
			int commaCount = 0;
			int i = 0;
			while (i < (int)line.length() && commaCount < 4)
			{
				if (line[i] == ',')
				{
					commaCount++;
				}
				i++;
			}
			int payment = 0;
			for (; i < (int)line.length() && line[i] != ','; i++)
			{
				if (line[i] >= '0' && line[i] <= '9')
				{
					payment = payment * 10 + (line[i] - '0');
				}
			}
			netRevenue += payment;
		}
		fileIn.close();
	}
}

void Accounts::displayRevenue()
{
	cout << "\nNet Revenue: " << netRevenue << "\n";
}

payData* Accounts::readPayments(int& numPayments)
{
	numPayments = 0;
	int seedRows = 0;
	ifstream f("BookingPayments.csv");
	if (f.is_open())
	{
		string line;
		getline(f, line);
		while (getline(f, line))
		{
			if (!line.empty())
			{
				seedRows++;
			}
		}
		f.close();
	}
	int liveRows = 0;
	ifstream f("bookings.csv");
	if (f.is_open())
	{
		string line;
		getline(f, line);
		while (getline(f, line))
		{
			if (!line.empty()) 
			{ 
				liveRows++;
			}
		}
		f.close();
	}
	numPayments = seedRows + liveRows;
	if (numPayments == 0)
	{
		return nullptr;
	}
	payData* paydata = new payData[numPayments];
	int payIn = 0;
	ifstream fileIn("BookingPayments.csv");
	if (fileIn.is_open())
	{
		string line;
		getline(fileIn, line); 
		while (getline(fileIn, line) && payIn < numPayments)
		{
			if (line.empty())
			{
				continue;
			}
			int delimPos = 0;
			int payment = 0;
			paydata[payIn].username = "";
			for (int i = 0; i < (int)line.length() && line[i] != ','; i++)
			{
				paydata[payIn].username += line[i];
				delimPos++;
			}
			for (int i = delimPos + 1; i < (int)line.length() && line[i] != ','; i++)
			{
				if (line[i] >= '0' && line[i] <= '9')
				{
					payment = payment * 10 + (line[i] - '0');
				}
			}
			paydata[payIn].payment = payment;
			payIn++;
		}
		fileIn.close();
	}
	ifstream fileIn("bookings.csv");
	if (fileIn.is_open())
	{
		string line;
		getline(fileIn, line);
		while (getline(fileIn, line) && payIn < numPayments)
		{
			if (line.empty())
			{
				continue;
			}
			int i = 0;
			while (i < (int)line.length() && line[i] != ',')
			{
				i++;
			}
			if (i < (int)line.length())
			{
				i++;
			}
			paydata[payIn].username = "";
			while (i < (int)line.length() && line[i] != ',')
			{
				paydata[payIn].username += line[i];
				i++;
			}
			if (i < (int)line.length())
			{
				i++;
			}
			while (i < (int)line.length() && line[i] != ',')
			{
				i++;
			}
			if (i < (int)line.length())
			{
				i++;
			}
			while (i < (int)line.length() && line[i] != ',')
			{
				i++;
			}
			if (i < (int)line.length())
			{
				i++;
			}
			int payment = 0;
			while (i < (int)line.length() && line[i] != ',')
			{
				if (line[i] >= '0' && line[i] <= '9')
				{
					payment = payment * 10 + (line[i] - '0');
				}
				i++;
			}
			paydata[payIn].payment = payment;
			payIn++;
		}
		fileIn.close();
	}
	numPayments = payIn;
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
