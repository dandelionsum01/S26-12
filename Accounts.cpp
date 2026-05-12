#include "header.h"
using namespace std;

Accounts::Accounts()
{
	netRevenue = 0;
}

void Accounts::calculateRevenue()
{
	netRevenue = 0;  

	{
		ifstream seedFile("BookingPayments.csv");
		if (seedFile.is_open())
		{
			string seedLine;
			getline(seedFile, seedLine); // header
			while (getline(seedFile, seedLine))
			{
				if (seedLine.empty())
					continue; // skip blank trailing line

				int payment = 0;
				int delimPos = 0;

				for (int i = 0; i < (int)seedLine.length() && seedLine[i] != ','; i++)
					delimPos++;

				for (int i = delimPos + 1; i < (int)seedLine.length() && seedLine[i] != ','; i++)
				{
					if (seedLine[i] >= '0' && seedLine[i] <= '9')
						payment = payment * 10 + (seedLine[i] - '0');
				}
				netRevenue += payment;
			}
			seedFile.close();
		}
	}

	{
		ifstream liveFile("bookings.csv");
		if (liveFile.is_open())
		{
			string liveLine;
			getline(liveFile, liveLine); // header
			while (getline(liveFile, liveLine))
			{
				if (liveLine.empty())
					continue;

				int commaCount = 0;
				int i = 0;
				while (i < (int)liveLine.length() && commaCount < 4)
				{
					if (liveLine[i] == ',')
						commaCount++;
					i++;
				}

				int payment = 0;
				for (; i < (int)liveLine.length() && liveLine[i] != ','; i++)
				{
					if (liveLine[i] >= '0' && liveLine[i] <= '9')
						payment = payment * 10 + (liveLine[i] - '0');
				}
				netRevenue += payment;
			}
			liveFile.close();
		}
	}

	{
		ifstream customFile("custom_packages.csv");
		if (customFile.is_open())
		{
			string customLine;
			getline(customFile, customLine); // header
			while (getline(customFile, customLine))
			{
				if (customLine.empty())
					continue;

				int commaCount = 0;
				int i = 0;
				while (i < (int)customLine.length() && commaCount < 8)
				{
					if (customLine[i] == ',')
						commaCount++;
					i++;
				}

				int payment = 0;
				while (i < (int)customLine.length() && customLine[i] != ',')
				{
					if (customLine[i] >= '0' && customLine[i] <= '9')
						payment = payment * 10 + (customLine[i] - '0');
					i++;
				}
				if (i < (int)customLine.length())
					i++; // step past comma

				string status;
				while (i < (int)customLine.length() && customLine[i] != ',' &&
					   customLine[i] != '\r' && customLine[i] != '\n')
				{
					status += customLine[i];
					i++;
				}

				if (status == "APPROVED")
					netRevenue += payment;
			}
			customFile.close();
		}
	}
}

void Accounts::displayRevenue()
{
	cout << "\nNet Revenue: " << netRevenue << "\n";
}

payData *Accounts::readPayments(int &numPayments)
{
	numPayments = 0;

	int seedRows = 0;
	{
		ifstream countSeed("BookingPayments.csv");
		if (countSeed.is_open())
		{
			string countLine;
			getline(countSeed, countLine); // header
			while (getline(countSeed, countLine))
				if (!countLine.empty())
					seedRows++;
			countSeed.close();
		}
	}

	int liveRows = 0;
	{
		ifstream countLive("bookings.csv");
		if (countLive.is_open())
		{
			string countLine;
			getline(countLive, countLine); // header
			while (getline(countLive, countLine))
				if (!countLine.empty())
					liveRows++;
			countLive.close();
		}
	}

	int customRows = 0;
	{
		ifstream countCust("custom_packages.csv");
		if (countCust.is_open())
		{
			string countLine;
			getline(countCust, countLine); // header
			while (getline(countCust, countLine))
			{
				if (countLine.empty())
					continue;
				string trimmed = countLine;
				while (!trimmed.empty() && (trimmed.back() == '\r' || trimmed.back() == '\n'))
					trimmed.pop_back();
				size_t lastComma = trimmed.find_last_of(',');
				if (lastComma == string::npos)
					continue;
				string status = trimmed.substr(lastComma + 1);
				if (status == "APPROVED")
					customRows++;
			}
			countCust.close();
		}
	}

	numPayments = seedRows + liveRows + customRows;
	if (numPayments == 0)
		return nullptr;

	payData *paydata = new payData[numPayments];
	int payIn = 0;

	{
		ifstream readSeed("BookingPayments.csv");
		if (readSeed.is_open())
		{
			string readLine;
			getline(readSeed, readLine); // header
			while (getline(readSeed, readLine) && payIn < numPayments)
			{
				if (readLine.empty())
					continue;

				int delimPos = 0;
				int payment = 0;

				paydata[payIn].username = "";
				for (int i = 0; i < (int)readLine.length() && readLine[i] != ','; i++)
				{
					paydata[payIn].username += readLine[i];
					delimPos++;
				}
				for (int i = delimPos + 1; i < (int)readLine.length() && readLine[i] != ','; i++)
				{
					if (readLine[i] >= '0' && readLine[i] <= '9')
						payment = payment * 10 + (readLine[i] - '0');
				}
				paydata[payIn].payment = payment;
				payIn++;
			}
			readSeed.close();
		}
	}

	{
		ifstream readLive("bookings.csv");
		if (readLive.is_open())
		{
			string readLine;
			getline(readLive, readLine); // header
			while (getline(readLive, readLine) && payIn < numPayments)
			{
				if (readLine.empty())
					continue;

				int i = 0;
				while (i < (int)readLine.length() && readLine[i] != ',')
					i++;
				if (i < (int)readLine.length())
					i++; // step past the comma

				paydata[payIn].username = "";
				while (i < (int)readLine.length() && readLine[i] != ',')
				{
					paydata[payIn].username += readLine[i];
					i++;
				}
				if (i < (int)readLine.length())
					i++;

				while (i < (int)readLine.length() && readLine[i] != ',')
					i++;
				if (i < (int)readLine.length())
					i++;
				while (i < (int)readLine.length() && readLine[i] != ',')
					i++;
				if (i < (int)readLine.length())
					i++;

				int payment = 0;
				while (i < (int)readLine.length() && readLine[i] != ',')
				{
					if (readLine[i] >= '0' && readLine[i] <= '9')
						payment = payment * 10 + (readLine[i] - '0');
					i++;
				}
				paydata[payIn].payment = payment;
				payIn++;
			}
			readLive.close();
		}
	}

	{
		ifstream readCust("custom_packages.csv");
		if (readCust.is_open())
		{
			string readLine;
			getline(readCust, readLine); // header
			while (getline(readCust, readLine) && payIn < numPayments)
			{
				if (readLine.empty())
					continue;

				while (!readLine.empty() &&
					   (readLine.back() == '\r' || readLine.back() == '\n'))
					readLine.pop_back();

				size_t lastComma = readLine.find_last_of(',');
				if (lastComma == string::npos)
					continue;
				string status = readLine.substr(lastComma + 1);
				if (status != "APPROVED")
					continue;

				int i = 0;
				while (i < (int)readLine.length() && readLine[i] != ',')
					i++;
				if (i < (int)readLine.length())
					i++;

				paydata[payIn].username = "";
				while (i < (int)readLine.length() && readLine[i] != ',')
				{
					paydata[payIn].username += readLine[i];
					i++;
				}
				if (i < (int)readLine.length())
					i++;

				for (int skip = 0; skip < 6; ++skip)
				{
					while (i < (int)readLine.length() && readLine[i] != ',')
						i++;
					if (i < (int)readLine.length())
						i++;
				}

				int payment = 0;
				while (i < (int)readLine.length() && readLine[i] != ',')
				{
					if (readLine[i] >= '0' && readLine[i] <= '9')
						payment = payment * 10 + (readLine[i] - '0');
					i++;
				}
				paydata[payIn].payment = payment;
				payIn++;
			}
			readCust.close();
		}
	}

	numPayments = payIn;
	return paydata;
}

int Accounts::partition(payData *paydata, int start, int end)
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
	paydata[i] = paydata[end] ;
	paydata[end] = temp;
	return i;
}

void Accounts::quicksortPayments(payData *paydata, int start, int end)
{
	if (start >= end)
		return;

	int pivot = partition(paydata, start, end);
	quicksortPayments(paydata, start, pivot - 1);
	quicksortPayments(paydata, pivot + 1, end);
}
