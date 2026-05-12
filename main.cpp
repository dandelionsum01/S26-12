
#include "GuiCore.h"

extern Screen *makeLauncher();
extern Screen *makeAuthCustomer();
extern Screen *makeAuthAdmin();

extern Screen *makeCustHome();
extern Screen *makePackages();
extern Screen *makeBooking();

extern Screen *makeCustomPackage();
extern Screen *makeReviews();
extern Screen *makeNotifications();
extern Screen *makeMyBookings();

extern Screen *makeAdminHome();
extern Screen *makeAddPackage();
extern Screen *makeAddCity();
extern Screen *makeCustomApproval();
extern Screen *makeRevenue();

int main()
{
    cout << "[TMS] Starting Travel Management System...\n";

    ScreenManager mgr;
    if (!mgr.init())
    {
        cout << "[TMS] Failed to initialize GUI.\n";
        cout << "      Make sure arial.ttf is in the project folder.\n";
        return 1;
    }

    mgr.registerScreen(ScreenID::LAUNCHER, &makeLauncher);
    mgr.registerScreen(ScreenID::AUTH_CUSTOMER, &makeAuthCustomer);
    mgr.registerScreen(ScreenID::AUTH_ADMIN, &makeAuthAdmin);
    mgr.registerScreen(ScreenID::CUST_HOME, &makeCustHome);
    mgr.registerScreen(ScreenID::PACKAGES, &makePackages);
    mgr.registerScreen(ScreenID::BOOKING, &makeBooking);
    mgr.registerScreen(ScreenID::CUSTOM_PACKAGE, &makeCustomPackage);
    mgr.registerScreen(ScreenID::REVIEWS, &makeReviews);
    mgr.registerScreen(ScreenID::NOTIFICATIONS, &makeNotifications);
    mgr.registerScreen(ScreenID::MY_BOOKINGS, &makeMyBookings);
    mgr.registerScreen(ScreenID::ADMIN_HOME, &makeAdminHome);
    mgr.registerScreen(ScreenID::ADD_PACKAGE, &makeAddPackage);
    mgr.registerScreen(ScreenID::ADD_CITY, &makeAddCity);
    mgr.registerScreen(ScreenID::CUSTOM_APPROVAL, &makeCustomApproval);
    mgr.registerScreen(ScreenID::REVENUE, &makeRevenue);

    try
    {
        mgr.run(ScreenID::LAUNCHER);
    }
    catch (const exception &e)
    {
        cout << "[TMS] Fatal error: " << e.what() << "\n";
        return 1;
    }

    cout << "[TMS] Goodbye.\n";
    return 0;
}
