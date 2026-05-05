// ============================================================
//  Review.cpp
//  Reads:    bookings.csv, reviews.csv
//  Writes:   reviews.csv
// ============================================================
#include "header.h"
using namespace std;

namespace
{
    string trimWSR(string s)
    {
        while (!s.empty() && (s.back() == '\r' || s.back() == '\n' ||
            s.back() == ' ' || s.back() == '\t'))
            s.pop_back();
        return s;
    }

    string todayStrR()
    {
        time_t t = time(0);
        tm* now = localtime(&t);
        char   buf[20];
        strftime(buf, sizeof(buf), "%Y-%m-%d", now);
        return string(buf);
    }

    bool hasBooking(const string& username, const string& packageID)
    {
        ifstream f("bookings.csv");
        if (!f.is_open()) return false;

        string line;
        getline(f, line); // header

        while (getline(f, line))
        {
            stringstream ss(line);
            string bid, uname, pkgID;
            getline(ss, bid, ',');
            getline(ss, uname, ',');
            getline(ss, pkgID, ',');
            if (trimWSR(uname) == username && trimWSR(pkgID) == packageID)
                return true;
        }
        return false;
    }

    string generateReviewID()
    {
        int count = 0;
        ifstream f("reviews.csv");
        if (f.is_open())
        {
            string line;
            getline(f, line); // header
            while (getline(f, line)) if (!line.empty()) count++;
        }
        return "RV-" + to_string(count + 1);
    }
}

void   CustomerReview::setComment(const string& c) { comment = c; }
string CustomerReview::getComment() const { return comment; }

void CustomerReview::addReview(const string& username, const string& packageID)
{
    if (!hasBooking(username, packageID))
    {
        cout << "You can only review a package you have booked.\n";
        return;
    }

    {
        ifstream f("reviews.csv");
        if (f.is_open())
        {
            string line;
            getline(f, line); // header
            while (getline(f, line))
            {
                stringstream ss(line);
                string rid, uname, pkgID;
                getline(ss, rid, ',');
                getline(ss, uname, ',');
                getline(ss, pkgID, ',');
                if (trimWSR(uname) == username && trimWSR(pkgID) == packageID)
                {
                    cout << "You have already reviewed this package.\n";
                    return;
                }
            }
        }
    }

    cout << "\n--- Write Your Review for Package: " << packageID << " ---\n";
    string reviewText;
    // Drain a possible pending newline (caller may have used cin >>)
    if (cin.peek() == '\n') cin.ignore();
    cout << "Your comment: ";
    getline(cin, reviewText);

    for (char& c : reviewText)
        if (c == ',') c = ';';

    bool isEmpty = true;
    {
        ifstream chk("reviews.csv");
        if (chk.is_open())
        {
            chk.seekg(0, ios::end);
            isEmpty = (chk.tellg() == 0);
        }
    }

    ofstream out("reviews.csv", ios::app);
    if (!out.is_open())
    {
        cout << "Cannot open reviews.csv!\n";
        return;
    }

    if (isEmpty)
        out << "ReviewID,Username,PackageID,Comment,Date\n";

    string reviewID = generateReviewID();
    out << reviewID << ","
        << username << ","
        << packageID << ","
        << reviewText << ","
        << todayStrR() << "\n";
    out.close();

    setComment(reviewText);
    cout << "Review submitted successfully! (ID: " << reviewID << ")\n";
}

void CustomerReview::readReview(const string& packageID)
{
    ifstream f("reviews.csv");
    if (!f.is_open())
    {
        cout << "No reviews found!\n";
        return;
    }

    string line;
    getline(f, line); // header

    bool found = false;
    cout << "\n========== Reviews for Package: " << packageID << " ==========\n";

    while (getline(f, line))
    {
        stringstream ss(line);
        string rid, uname, pkgID, cmt, date;
        getline(ss, rid, ',');
        getline(ss, uname, ',');
        getline(ss, pkgID, ',');
        getline(ss, cmt, ',');
        getline(ss, date, ',');
        date = trimWSR(date);

        if (trimWSR(pkgID) == packageID)
        {
            found = true;
            cout << "  [" << rid << "]  " << uname
                << "  (" << date << ")\n";
            cout << "  " << cmt << "\n";
            cout << string(55, '-') << "\n";
        }
    }

    if (!found)
        cout << "No reviews yet for this package.\n";

    cout << "====================================================\n";
}
