#include "GuiCore.h"
#include "Widgets.h"

class ScreenCustHome : public Screen
{
    Label  welcomeLbl;
    Label  subLbl;
    Label  sectionLbl1, sectionLbl2;

    Button btnBrowse;
    Button btnMyBookings;
    Button btnCustomReq;

    Button btnReviews;
    Button btnNotifications;

    Button btnSignOut;

public:
    string title() const override { return "Customer Home"; }

    void onEnter() override
    {
        sf::Font* f = ctx->font;

        welcomeLbl = Label(f, "Welcome back, " + ctx->username + "!",
            40.f, 100.f, 26);
        welcomeLbl.setStyle(sf::Text::Bold);

        subLbl = Label(f,
            "What would you like to do today?",
            40.f, 138.f, 14);
        subLbl.setColor(Theme::TEXT_MUTED);

        sectionLbl1 = Label(f, "PLAN YOUR TRIP", 40.f, 185.f, 11);
        sectionLbl1.setColor(Theme::TEXT_MUTED);
        sectionLbl1.setStyle(sf::Text::Bold);

        float x = 40.f, y = 210.f;
        float bw = 320.f, bh = 86.f, gap = 18.f;

        btnBrowse = Button(f, "Browse Packages",
            x, y, bw, bh, Button::PRIMARY);   // cobalt blue
        btnBrowse.setSubtitle("Pick from available trips and book instantly");

        btnCustomReq = Button(f, "Request Custom Package",
            x + bw + gap, y, bw, bh, Button::SUCCESS); // forest teal
        btnCustomReq.setSubtitle("Design a trip with your preferred hotel & nights");

        btnMyBookings = Button(f, "My Bookings",
            x + 2 * (bw + gap), y, bw, bh, Button::PURPLE); // violet
        btnMyBookings.setSubtitle("View trips you have booked");

        sectionLbl2 = Label(f, "REVIEWS & UPDATES", 40.f, 320.f, 11);
        sectionLbl2.setColor(Theme::TEXT_MUTED);
        sectionLbl2.setStyle(sf::Text::Bold);

        float y2 = 345.f;
        float bw2 = (bw * 3 + gap * 2) / 2.f - gap / 2.f;

        btnReviews = Button(f, "Reviews",
            x, y2, bw2, 70.f, Button::WARN);  // amber/gold
        btnReviews.setSubtitle("Read reviews or rate a package you booked");

        btnNotifications = Button(f, "Notifications",
            x + bw2 + gap, y2, bw2, 70.f, Button::DANGER); // burgundy
        btnNotifications.setSubtitle("Subscribe and view package updates");

        btnSignOut = Button(f, "Sign Out",
            Theme::WIN_W - 130.f - 30.f,
            Theme::WIN_H - Theme::FOOTER_H - 56.f,
            130.f, 38.f, Button::DANGER);
    }

    void handleEvent(const sf::Event& e) override
    {
        if (btnBrowse.handleEvent(e))        ctx->go(ScreenID::PACKAGES);
        if (btnCustomReq.handleEvent(e))     ctx->go(ScreenID::CUSTOM_PACKAGE);
        if (btnMyBookings.handleEvent(e))    ctx->go(ScreenID::MY_BOOKINGS);
        if (btnReviews.handleEvent(e))       ctx->go(ScreenID::REVIEWS);
        if (btnNotifications.handleEvent(e)) ctx->go(ScreenID::NOTIFICATIONS);
        if (btnSignOut.handleEvent(e))
        {
            ctx->clearSession();
            ctx->go(ScreenID::LAUNCHER);
        }
    }

    void draw(sf::RenderWindow& w) override
    {
        welcomeLbl.draw(w);
        subLbl.draw(w);
        sectionLbl1.draw(w);
        sectionLbl2.draw(w);

        btnBrowse.draw(w);
        btnCustomReq.draw(w);
        btnMyBookings.draw(w);

        btnReviews.draw(w);
        btnNotifications.draw(w);

        btnSignOut.draw(w);
    }
};

Screen* makeCustHome() { return new ScreenCustHome(); }
