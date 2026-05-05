// ============================================================
// MEMBER C  —  Notification.cpp
// Produces:    subscriptions.csv   (who is subscribed)
//              notifications.csv   (messages per user)
//
// subscriptions.csv columns:  Username
// notifications.csv columns:  Username, Message, Date, IsRead
// ============================================================

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <ctime>
#include "header.h"
using namespace std;

// ─── tiny helpers (local to this translation unit) ───────────────────────────

static string trimWSN(string s)
{
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' ||
        s.back() == ' ' || s.back() == '\t'))
        s.pop_back();
    return s;
}

static string todayStrN()
{
    time_t t = time(0);
    tm* now = localtime(&t);
    char   buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d", now);
    return string(buf);
}

// ─── NotificationPanel constructor ──────────────────────────────────────────

NotificationPanel::NotificationPanel()
{
    notification = "";
    isSubscribed = false;
}

// ─── Setters / Getters ────────────────────────────────────────────────────────

void NotificationPanel::setNotification(string msg) { notification = msg; }
string NotificationPanel::getNotification() { return notification; }
bool   NotificationPanel::getIsSubscribed() { return isSubscribed; }

// ─── subscribe ───────────────────────────────────────────────────────────────
// Adds the username to subscriptions.csv (if not already subscribed).
// Sets isSubscribed flag on this object.
void NotificationPanel::subscribe(string username)
{
    // ── Check if already subscribed ─────────────────────────────────────────
    {
        ifstream chk("subscriptions.csv");
        if (chk.is_open())
        {
            string line;
            getline(chk, line);         // skip header
            while (getline(chk, line))
            {
                if (trimWSN(line) == username)
                {
                    cout << username << " is already subscribed to notifications.\n";
                    isSubscribed = true;
                    chk.close();
                    return;
                }
            }
            chk.close();
        }
    }

    // ── Write to subscriptions.csv ───────────────────────────────────────────
    bool isEmpty = true;
    {
        ifstream chk("subscriptions.csv");
        if (chk.is_open())
        {
            chk.seekg(0, ios::end);
            isEmpty = (chk.tellg() == 0);
            chk.close();
        }
    }

    ofstream out("subscriptions.csv", ios::app);
    if (!out.is_open())
    {
        cout << "Cannot open subscriptions.csv!\n";
        return;
    }
    if (isEmpty)
        out << "Username\n";

    out << username << "\n";
    out.close();

    isSubscribed = true;
    cout << username << " has subscribed to notifications.\n";

    // ── Send a welcome notification ──────────────────────────────────────────
    setNotification("Welcome! You are now subscribed to package updates.");
    // broadcast to just this user
    {
        bool isEmpty2 = true;
        {
            ifstream chk("notifications.csv");
            if (chk.is_open())
            {
                chk.seekg(0, ios::end);
                isEmpty2 = (chk.tellg() == 0);
                chk.close();
            }
        }
        ofstream nout("notifications.csv", ios::app);
        if (nout.is_open())
        {
            if (isEmpty2)
                nout << "Username,Message,Date,IsRead\n";
            nout << username << ","
                << getNotification() << ","
                << todayStrN() << ","
                << "NO\n";
            nout.close();
        }
    }
}

// ─── broadcastNotification (free helper used internally) ─────────────────────
// Sends a message to EVERY subscribed user in subscriptions.csv.
// Called by admin/system code when a new package is added or prices change.
void broadcastNotification(string message)
{
    // Load all subscribers
    ifstream subFile("subscriptions.csv");
    if (!subFile.is_open())
    {
        cout << "No subscribers found.\n";
        return;
    }

    bool isEmpty = true;
    {
        ifstream chk("notifications.csv");
        if (chk.is_open())
        {
            chk.seekg(0, ios::end);
            isEmpty = (chk.tellg() == 0);
            chk.close();
        }
    }

    ofstream nout("notifications.csv", ios::app);
    if (!nout.is_open())
    {
        cout << "Cannot open notifications.csv!\n";
        subFile.close();
        return;
    }

    if (isEmpty)
        nout << "Username,Message,Date,IsRead\n";

    string line;
    getline(subFile, line);             // skip header
    int sentCount = 0;
    while (getline(subFile, line))
    {
        string uname = trimWSN(line);
        if (!uname.empty())
        {
            nout << uname << ","
                << message << ","
                << todayStrN() << ","
                << "NO\n";
            sentCount++;
        }
    }
    subFile.close();
    nout.close();

    cout << "Notification sent to " << sentCount << " subscriber(s).\n";
}

// ─── displayNotification ─────────────────────────────────────────────────────
// Shows all UNREAD notifications for the given username,
// then marks them as read.
void NotificationPanel::displayNotification(string username)
{
    // ── Check subscription status ────────────────────────────────────────────
    bool subscribed = false;
    {
        ifstream chk("subscriptions.csv");
        if (chk.is_open())
        {
            string line;
            getline(chk, line);         // skip header
            while (getline(chk, line))
            {
                if (trimWSN(line) == username)
                {
                    subscribed = true;
                    break;
                }
            }
            chk.close();
        }
    }
    isSubscribed = subscribed;

    if (!subscribed)
    {
        cout << username << " is not subscribed to notifications.\n";
        cout << "Use 'Subscribe' from the menu to start receiving updates.\n";
        return;
    }

    // ── Read notifications.csv ───────────────────────────────────────────────
    ifstream fileIn("notifications.csv");
    if (!fileIn.is_open())
    {
        cout << "No notifications found.\n";
        return;
    }

    // We'll collect ALL rows; display unread ones; mark them read in temp file.
    ofstream fileOut("temp_notifications.csv");

    string line;
    getline(fileIn, line);
    fileOut << line << "\n";            // copy header

    int unreadCount = 0;

    cout << "\n========== Notifications for " << username << " ==========\n";

    while (getline(fileIn, line))
    {
        stringstream ss(line);
        string uname, msg, date, isRead;
        getline(ss, uname, ',');
        getline(ss, msg, ',');
        getline(ss, date, ',');
        getline(ss, isRead, ',');
        isRead = trimWSN(isRead);

        if (trimWSN(uname) == username && isRead == "NO")
        {
            unreadCount++;
            cout << "  [" << date << "] " << msg << "\n";
            // Mark as read in output
            fileOut << uname << "," << msg << "," << date << ",YES\n";
        }
        else
        {
            fileOut << line << "\n";
        }
    }

    fileIn.close();
    fileOut.close();

    if (unreadCount == 0)
        cout << "  (No new notifications)\n";

    cout << "===================================================\n";

    // Replace original file with the updated (all marked as read) version
    remove("notifications.csv");
    rename("temp_notifications.csv", "notifications.csv");
}
