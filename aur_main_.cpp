// ============================================================
//  main.cpp  --  Travel Management System driver
// ============================================================
#include "header.h"
using namespace std;

// Read an integer from the user, restricted to [minValue, maxValue].
static int readIntChoice(int minValue, int maxValue)
{
    int choice;
    while (true)
    {
        cout << "Enter choice (" << minValue << "-" << maxValue << "): ";
        cin >> choice;
        if (cin.fail())
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Try again.\n";
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (choice < minValue || choice > maxValue)
        {
            cout << "Choice out of range. Try again.\n";
            continue;
        }
        return choice;
    }
}

static string getUsernamePrompt(const string& currentUser)
{
    if (!currentUser.empty())
    {
        char useCurrent;
        cout << "Use current user '" << currentUser << "'? (y/n): ";
        cin >> useCurrent;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (useCurrent == 'y' || useCurrent == 'Y')
            return currentUser;
    }

    string username;
    cout << "Enter username: ";
    getline(cin, username);
    return username;
}

static void adminMenu()
{
    Admin          admin;
    Accounts       accounts;
    PlannedPackage planned;
    CustomPackage  custom;

    bool signedIn = false;

    while (true)
    {
        cout << "\n=== Admin Menu ===\n";
        cout << "1. Sign in\n";
        cout << "2. Add city\n";
        cout << "3. Add planned package\n";
        cout << "4. Delete planned package\n";
        cout << "5. Display planned packages\n";
        cout << "6. Find package by destination\n";
        cout << "7. Remove expired packages\n";
        cout << "8. View pending custom requests\n";
        cout << "9. Approve or reject custom request\n";
        cout << "10. View revenue\n";
        cout << "11. Sort payments\n";
        cout << "0. Back\n";

        int choice = readIntChoice(0, 11);
        if (choice == 0) return;

        if (choice == 1)
        {
            try { admin.signin(); signedIn = true; }
            catch (const exception& e) { cout << "Sign-in error: " << e.what() << "\n"; }
            continue;
        }

        if (!signedIn)
        {
            cout << "Please sign in first.\n";
            continue;
        }

        try
        {
            switch (choice)
            {
            case 2:
            {
                string name;
                double lat, lon;
                cout << "City name: ";
                getline(cin, name);
                lat = readValue<double>("Latitude: ");
                lon = readValue<double>("Longitude: ");
                planned.setNewcity(name, lat, lon);
                break;
            }
            case 3:  planned.addPackage(); break;
            case 4:  planned.deletePackage(); break;
            case 5:  planned.displayPackage(); break;
            case 6:  planned.findPackage(); break;
            case 7:  planned.removeExpiredPackages(); break;
            case 8:  custom.viewPending(); break;
            case 9:  custom.approvePackage(); break;
            case 10: admin.findRevenue(&accounts); break;
            case 11: admin.sortedPayments(&accounts); break;
            default: break;
            }
        }
        catch (const exception& e)
        {
            cout << "Error: " << e.what() << "\n";
        }
    }
}

static void customerMenu()
{
    Customer          customer;
    PlannedPackage    planned;
    CustomPackage     custom;
    Booking           booking;
    CustomerReview    review;
    NotificationPanel notifications;

    string currentUser;

    while (true)
    {
        cout << "\n=== Customer Menu ===\n";
        cout << "1. Sign up\n";
        cout << "2. Sign in\n";
        cout << "3. Display cities\n";
        cout << "4. Display planned packages\n";
        cout << "5. Find package by destination\n";
        cout << "6. Make booking\n";
        cout << "7. Request custom package\n";
        cout << "8. Add review\n";
        cout << "9. Read reviews for package\n";
        cout << "10. Subscribe to notifications\n";
        cout << "11. View notifications\n";
        cout << "0. Back\n";

        int choice = readIntChoice(0, 11);
        if (choice == 0) return;

        try
        {
            switch (choice)
            {
            case 1:
                customer.signup();
                break;
            case 2:
                customer.signin();
                currentUser = customer.getUsername();
                break;
            case 3:
                planned.displayCities();
                break;
            case 4:
                planned.displayPackage();
                break;
            case 5:
                planned.findPackage();
                break;
            case 6:
            {
                string user = getUsernamePrompt(currentUser);
                booking.makeBooking(user);
                break;
            }
            case 7:
            {
                string user = getUsernamePrompt(currentUser);
                custom.enterDetails(user);
                break;
            }
            case 8:
            {
                string user = getUsernamePrompt(currentUser);
                string pkg;
                cout << "Enter Package ID to review: ";
                getline(cin, pkg);
                review.addReview(user, pkg);
                break;
            }
            case 9:
            {
                string pkg;
                cout << "Enter Package ID to view reviews: ";
                getline(cin, pkg);
                review.readReview(pkg);
                break;
            }
            case 10:
            {
                string user = getUsernamePrompt(currentUser);
                notifications.subscribe(user);
                break;
            }
            case 11:
            {
                string user = getUsernamePrompt(currentUser);
                notifications.displayNotification(user);
                break;
            }
            default: break;
            }
        }
        catch (const exception& e)
        {
            cout << "Error: " << e.what() << "\n";
        }
    }
}

int main()
{
    cout << "Travel Management System\n";

    try
    {
        while (true)
        {
            cout << "\n=== Main Menu ===\n";
            cout << "1. Admin\n";
            cout << "2. Customer\n";
            cout << "0. Exit\n";

            int choice = readIntChoice(0, 2);
            if (choice == 0) break;
            if (choice == 1) adminMenu();
            else if (choice == 2) customerMenu();
        }
    }
    catch (const exception& e)
    {
        cout << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    cout << "Goodbye.\n";
    return 0;
}
