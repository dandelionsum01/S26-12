// ============================================================
//  Notification.cpp
//  Reads / writes:  subscriptions.csv, notifications.csv
// ============================================================
#include "header.h"
using namespace std;

namespace
{
    string trimWSN(string s)
    {
        while (!s.empty() && (s.back() == '\r' || s.back() == '\n' ||
            s.back() == ' ' || s.back() == '\t'))
            s.pop_back();
        return s;
    }

    string todayStrN()
    {
        time_t t = time(0);
        tm* now = localtime(&t);
        char   buf[20];
        strftime(buf, sizeof(buf), "%Y-%m-%d", now);
        return string(buf);
    }
}

NotificationPanel::NotificationPanel()
{
    notification = "";
    isSubscribed = false;
}

void   NotificationPanel::setNotification(const string& msg) { notification = msg; }
string NotificationPanel::getNotification() const { return notification; }
bool   NotificationPanel::getIsSubscribed() const { return isSubscribed; }

void NotificationPanel::subscribe(const string& username)
{
    {
        ifstream chk("subscriptions.csv");
        if (chk.is_open())
        {
            string line;
            getline(chk, line); // header
            while (getline(chk, line))
            {
                if (trimWSN(line) == username)
                {
                    cout << username << " is already subscribed to notifications.\n";
                    isSubscribed = true;
                    return;
                }
            }
        }
    }

    bool isEmpty = true;
    {
        ifstream chk("subscriptions.csv");
        if (chk.is_open())
        {
            chk.seekg(0, ios::end);
            isEmpty = (chk.tellg() == 0);
        }
    }

    ofstream out("subscriptions.csv", ios::app);
    if (!out.is_open())
    {
        cout << "Cannot open subscriptions.csv!\n";
        return;
    }
    if (isEmpty) out << "Username\n";
    out << username << "\n";
    out.close();

    isSubscribed = true;
    cout << username << " has subscribed to notifications.\n";

    setNotification("Welcome! You are now subscribed to package updates.");
    {
        bool isEmpty2 = true;
        {
            ifstream chk("notifications.csv");
            if (chk.is_open())
            {
                chk.seekg(0, ios::end);
                isEmpty2 = (chk.tellg() == 0);
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
        }
    }
}

void broadcastNotification(const string& message)
{
    ifstream subFile("subscriptions.csv");
    if (!subFile.is_open()) return; // Quietly skip if no subscribers

    bool isEmpty = true;
    {
        ifstream chk("notifications.csv");
        if (chk.is_open())
        {
            chk.seekg(0, ios::end);
            isEmpty = (chk.tellg() == 0);
        }
    }

    ofstream nout("notifications.csv", ios::app);
    if (!nout.is_open())
    {
        cout << "Cannot open notifications.csv!\n";
        return;
    }
    if (isEmpty)
        nout << "Username,Message,Date,IsRead\n";

    string line;
    getline(subFile, line); // header
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

    if (sentCount > 0)
        cout << "(Notification sent to " << sentCount << " subscriber(s))\n";
}

void NotificationPanel::displayNotification(const string& username)
{
    bool subscribed = false;
    {
        ifstream chk("subscriptions.csv");
        if (chk.is_open())
        {
            string line;
            getline(chk, line); // header
            while (getline(chk, line))
            {
                if (trimWSN(line) == username)
                {
                    subscribed = true;
                    break;
                }
            }
        }
    }
    isSubscribed = subscribed;

    if (!subscribed)
    {
        cout << username << " is not subscribed to notifications.\n";
        cout << "Use 'Subscribe' from the menu to start receiving updates.\n";
        return;
    }

    ifstream fileIn("notifications.csv");
    if (!fileIn.is_open())
    {
        cout << "No notifications found.\n";
        return;
    }

    ofstream fileOut("temp_notifications.csv");
    if (!fileOut.is_open())
    {
        cout << "Cannot open temp file for notifications.\n";
        fileIn.close();
        return;
    }

    string line;
    getline(fileIn, line); // header
    fileOut << line << "\n";

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

    remove("notifications.csv");
    rename("temp_notifications.csv", "notifications.csv");
}
