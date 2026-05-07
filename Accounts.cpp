#include "header.h"
using namespace std;

Accounts::Accounts()
{
	netRevenue = 0;
}

void Accounts::calculateRevenue()
{
	netRevenue = 0; // reset every call so repeated calls don't double-count

	// ---- Read seed payments from BookingPayments.csv ----
	{
		ifstream seedFile("BookingPayments.csv");
		if (seedFile.is_open())
		{
			string seedLine;
			getline(seedFile, seedLine); // header
			while (getline(seedFile, seedLine))
			{
				if (seedLine.empty()) continue; // skip blank trailing line

				int payment = 0;
				int delimPos = 0;

				// walk to the first comma
				for (int i = 0; i < (int)seedLine.length() && seedLine[i] != ','; i++)
					delimPos++;

				// payment is the column AFTER the first comma, BEFORE the second
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

	// ---- Also include live bookings from bookings.csv ----
	// bookings.csv columns: BookingID,CustomerUsername,PackageID,DepartureDate,Payment,BookingDate
	// payment is the 5th column (index 4)
	{
		ifstream liveFile("bookings.csv");
		if (liveFile.is_open())
		{
			string liveLine;
			getline(liveFile, liveLine); // header
			while (getline(liveFile, liveLine))
			{
				if (liveLine.empty()) continue;

				// skip the first 4 commas
				int commaCount = 0;
				int i = 0;
				while (i < (int)liveLine.length() && commaCount < 4)
				{
					if (liveLine[i] == ',') commaCount++;
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
}

void Accounts::displayRevenue()
{
	cout << "\nNet Revenue: " << netRevenue << "\n";
}

payData* Accounts::readPayments(int& numPayments)
{
	numPayments = 0; // initialize to 0 (caller may have passed any value)

	// ---- First pass: count rows in BOTH files ----
	int seedRows = 0;
	{
		ifstream countSeed("BookingPayments.csv");
		if (countSeed.is_open())
		{
			string countLine;
			getline(countSeed, countLine); // header
			while (getline(countSeed, countLine))
				if (!countLine.empty()) seedRows++;
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
				if (!countLine.empty()) liveRows++;
			countLive.close();
		}
	}

	numPayments = seedRows + liveRows;
	if (numPayments == 0)
		return nullptr;

	payData* paydata = new payData[numPayments];
	int payIn = 0;

	// ---- Second pass: fill array from BookingPayments.csv ----
	{
		ifstream readSeed("BookingPayments.csv");
		if (readSeed.is_open())
		{
			string readLine;
			getline(readSeed, readLine); // header
			while (getline(readSeed, readLine) && payIn < numPayments)
			{
				if (readLine.empty()) continue;

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

	// ---- Second pass: fill array from bookings.csv ----
	// columns: BookingID,CustomerUsername,PackageID,DepartureDate,Payment,BookingDate
	{
		ifstream readLive("bookings.csv");
		if (readLive.is_open())
		{
			string readLine;
			getline(readLive, readLine); // header
			while (getline(readLive, readLine) && payIn < numPayments)
			{
				if (readLine.empty()) continue;

				// 1st field: BookingID  -> skip
				int i = 0;
				while (i < (int)readLine.length() && readLine[i] != ',') i++;
				if (i < (int)readLine.length()) i++; // step past the comma

				// 2nd field: CustomerUsername  -> store as username
				paydata[payIn].username = "";
				while (i < (int)readLine.length() && readLine[i] != ',')
				{
					paydata[payIn].username += readLine[i];
					i++;
				}
				if (i < (int)readLine.length()) i++;

				// 3rd field: PackageID  -> skip
				while (i < (int)readLine.length() && readLine[i] != ',') i++;
				if (i < (int)readLine.length()) i++;

				// 4th field: DepartureDate -> skip
				while (i < (int)readLine.length() && readLine[i] != ',') i++;
				if (i < (int)readLine.length()) i++;

				// 5th field: Payment
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

	numPayments = payIn; // actual filled count (in case some rows were skipped)
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
		return;

	int pivot = partition(paydata, start, end);
	quicksortPayments(paydata, start, pivot - 1);
	quicksortPayments(paydata, pivot + 1, end);
}
