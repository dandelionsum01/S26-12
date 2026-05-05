// ============================================================
// MEMBER C  —  Review.cpp
// Produces:    reviews.csv
//
// reviews.csv columns:
//   ReviewID, Username, PackageID, Comment, Date
// ============================================================

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include "header.h"
using namespace std;

// ─── tiny helpers (local to this translation unit) ───────────────────────────

static string trimWSR(string s)
{
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' ||
        s.back() == ' ' || s.back() == '\t'))
        s.pop_back();
    return s;
}

static string todayStrR()
{
    time_t t = time(0);
    tm* now = localtime(&t);
    char   buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d", now);
    return string(buf);
}

// Check whether the customer has an existing booking for this package.
// Returns true if a booking is found in bookings.csv.
static bool hasBooking(const string& username, const string& packageID)
{
    ifstream f("bookings.csv");
    if (!f.is_open()) return false;

    string line;
    getline(f, line);               // skip header

    while (getline(f, line))
    {
        stringstream ss(line);
        string bid, uname, pkgID;
        getline(ss, bid, ',');
        getline(ss, uname, ',');
        getline(ss, pkgID, ',');
        if (trimWSR(uname) == username && trimWSR(pkgID) == packageID)
        {
            f.close();
            return true;
        }
    }
    f.close();
    return false;
}

// Generate a sequential review ID.
static string generateReviewID()
{
    int count = 0;
    ifstream f("reviews.csv");
    if (f.is_open())
    {
        string line;
        getline(f, line);           // skip header
        while (getline(f, line)) count++;
        f.close();
    }
    return "RV-" + to_string(count + 1);
}

// ─── Setters / Getters ────────────────────────────────────────────────────────

void CustomerReview::setComment(string c) { comment = c; }
string CustomerReview::getComment() { return comment; }

// ─── addReview ───────────────────────────────────────────────────────────────
// Lets a customer leave a review for a package they have booked.
void CustomerReview::addReview(string username, string packageID)
{
    // ── 1. Verify the customer actually booked this package ──────────────────
    if (!hasBooking(username, packageID))
    {
        cout << "You can only review a package you have booked.\n";
        return;
    }

    // ── 2. Prevent duplicate reviews ────────────────────────────────────────
    {
        ifstream f("reviews.csv");
        if (f.is_open())
        {
            string line;
            getline(f, line);           // skip header
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
                    f.close();
                    return;
                }
            }
            f.close();
        }
    }

    // ── 3. Collect the review text (allow spaces) ────────────────────────────
    cout << "\n--- Write Your Review for Package: " << packageID << " ---\n";
    string reviewText;
    cin.ignore();
    cout << "Your comment: ";
    getline(cin, reviewText);

    // Replace any commas in the comment with semicolons to keep CSV safe
    for (char& c : reviewText)
        if (c == ',') c = ';';

    // ── 4. Write to reviews.csv ──────────────────────────────────────────────
    bool isEmpty = true;
    {
        ifstream chk("reviews.csv");
        if (chk.is_open())
        {
            chk.seekg(0, ios::end);
            isEmpty = (chk.tellg() == 0);
            chk.close();
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

// ─── readReview ──────────────────────────────────────────────────────────────
// Displays all reviews for the given packageID.
void CustomerReview::readReview(string packageID)
{
    ifstream f("reviews.csv");
    if (!f.is_open())
    {
        cout << "No reviews found!\n";
        return;
    }

    string line;
    getline(f, line);               // skip header

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
    f.close();

    if (!found)
        cout << "No reviews yet for this package.\n";

    cout << "====================================================\n";
}
