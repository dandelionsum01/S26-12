#include "GuiCore.h"
#include "Widgets.h"

class ScreenAdminHome : public Screen
{
    Label welcomeLbl, subLbl, sec1Lbl, sec2Lbl;
    Button btnAddPkg, btnAddCity, btnApprovals;
    Button btnRevenue, btnBroadcast;
    Button btnSignOut;
    Dialog dialog;

    Label bcLbl;
    TextField bcField;
    Button btnBcSend, btnBcCancel;
    bool bcOpen = false;

    static string today()
    {
        time_t t = time(0);
        tm* n = localtime(&t);
        char b[20];
        strftime(b, sizeof(b), "%Y-%m-%d", n);
        return string(b);
    }

    void doBroadcast()
    {
        string msg = bcField.getText();
        if (msg.empty())
        {
            dialog.show("Empty", "Message cannot be empty.", Dialog::ERROR_KIND);
            return;
        }

        string clean;
        for (char c : msg)
        {
            if (c == ',')
                clean += ';';
            else if (c == '\n' || c == '\r')
                clean += ' ';
            else
                clean += c;
        }

        vector<string> subs;
        {
            ifstream sf("subscriptions.csv");
            if (sf.is_open())
            {
                string sl;
                getline(sf, sl); // header
                while (getline(sf, sl))
                {
                    while (!sl.empty() && (sl.back() == '\r' || sl.back() == '\n' || sl.back() == ' '))
                        sl.pop_back();
                    if (!sl.empty())
                        subs.push_back(sl);
                }
            }
        }
        if (subs.empty())
        {
            dialog.show("No Subscribers", "No customers are subscribed yet.", Dialog::INFO);
            return;
        }

        bool empty = false;
        {
            ifstream chk("notifications.csv");
            if (!chk.is_open())
                empty = true;
            else
            {
                chk.seekg(0, ios::end);
                empty = (chk.tellg() == 0);
            }
        }
        ofstream f("notifications.csv", ios::app);
        if (!f.is_open())
        {
            dialog.show("File Error", "Could not open notifications.csv.", Dialog::ERROR_KIND);
            return;
        }
        if (empty)
            f << "Username,Message,Date,IsRead\n";
        for (const auto& u : subs)
            f << u << "," << clean << "," << today() << ",NO\n";
        f.close();

        bcOpen = false;
        bcField.setText("");
        dialog.show("Broadcast Sent",
            "Notification has been broadcast to all\n"
            "subscribed customers.",
            Dialog::SUCCESS);
    }

public:
    string title() const override { return "Admin Dashboard"; }

    void onEnter() override
    {
        sf::Font* f = ctx->font;

        welcomeLbl = Label(f, "Admin Console -- " + ctx->username,
            40.f, 100.f, 26);
        welcomeLbl.setStyle(sf::Text::Bold);

        subLbl = Label(f, "Manage packages, cities, requests, and revenue.",
            40.f, 138.f, 14);
        subLbl.setColor(Theme::TEXT_MUTED);

        sec1Lbl = Label(f, "PACKAGES & CITIES", 40.f, 185.f, 11);
        sec1Lbl.setColor(Theme::TEXT_MUTED);
        sec1Lbl.setStyle(sf::Text::Bold);

        float x = 40.f, y = 210.f, bw = 320.f, bh = 86.f, gap = 18.f;

        btnAddPkg = Button(f, "Add Package", x, y, bw, bh, Button::PRIMARY);
        btnAddPkg.setSubtitle("Create a new planned package");

        btnAddCity = Button(f, "Add City",
            x + bw + gap, y, bw, bh, Button::SUCCESS);
        btnAddCity.setSubtitle("Add city with auto-geocoding");

        btnApprovals = Button(f, "Custom Requests",
            x + 2 * (bw + gap), y, bw, bh, Button::DANGER);
        btnApprovals.setSubtitle("Approve or reject pending requests");

        sec2Lbl = Label(f, "REVENUE & COMMUNICATION", 40.f, 320.f, 11);
        sec2Lbl.setColor(Theme::TEXT_MUTED);
        sec2Lbl.setStyle(sf::Text::Bold);

        float y2 = 345.f;
        float bw2 = (bw * 3 + gap * 2) / 2.f - gap / 2.f;

        btnRevenue = Button(f, "Revenue Report",
            x, y2, bw2, 70.f, Button::NEUTRAL);
        btnRevenue.setSubtitle("View totals and sorted payments");

        btnBroadcast = Button(f, "Broadcast Notification",
            x + bw2 + gap, y2, bw2, 70.f, Button::NEUTRAL);
        btnBroadcast.setSubtitle("Send message to all subscribers");

        btnSignOut = Button(f, "Sign Out",
            Theme::WIN_W - 160.f, Theme::WIN_H - Theme::FOOTER_H - 56.f,
            130.f, 38.f, Button::DANGER);

        bcLbl = Label(f, "Broadcast Message",
            Theme::WIN_W / 2.f - 200.f,
            Theme::WIN_H / 2.f - 110.f, 18);
        bcLbl.setStyle(sf::Text::Bold);
        bcField = TextField(f,
            Theme::WIN_W / 2.f - 200.f,
            Theme::WIN_H / 2.f - 70.f,
            400.f, 80.f);
        bcField.setPlaceholder("Type message for all subscribers...");
        bcField.setMaxLen(300);
        btnBcSend = Button(f, "Send",
            Theme::WIN_W / 2.f - 200.f,
            Theme::WIN_H / 2.f + 30.f, 195.f, 40.f, Button::SUCCESS);
        btnBcCancel = Button(f, "Cancel",
            Theme::WIN_W / 2.f + 5.f,
            Theme::WIN_H / 2.f + 30.f, 195.f, 40.f, Button::NEUTRAL);
        dialog.setFont(f);
    }

    void handleEvent(const sf::Event& e) override
    {
        if (dialog.isOpen())
        {
            dialog.handleEvent(e);
            return;
        }

        if (bcOpen)
        {
            bcField.handleEvent(e);
            if (btnBcSend.handleEvent(e))
                doBroadcast();
            if (btnBcCancel.handleEvent(e))
                bcOpen = false;
            return;
        }

        if (btnAddPkg.handleEvent(e))
            ctx->go(ScreenID::ADD_PACKAGE);
        if (btnAddCity.handleEvent(e))
            ctx->go(ScreenID::ADD_CITY);
        if (btnApprovals.handleEvent(e))
            ctx->go(ScreenID::CUSTOM_APPROVAL);
        if (btnRevenue.handleEvent(e))
            ctx->go(ScreenID::REVENUE);
        if (btnBroadcast.handleEvent(e))
            bcOpen = true;
        if (btnSignOut.handleEvent(e))
        {
            ctx->clearSession();
            ctx->go(ScreenID::LAUNCHER);
        }
    }

    void update(float dt) override
    {
        if (bcOpen)
            bcField.update(dt);
    }

    void draw(sf::RenderWindow& w) override
    {
        welcomeLbl.draw(w);
        subLbl.draw(w);
        sec1Lbl.draw(w);
        sec2Lbl.draw(w);
        btnAddPkg.draw(w);
        btnAddCity.draw(w);
        btnApprovals.draw(w);
        btnRevenue.draw(w);
        btnBroadcast.draw(w);
        btnSignOut.draw(w);

        if (bcOpen)
        {
            sf::RectangleShape dim;
            dim.setSize(sf::Vector2f((float)Theme::WIN_W, (float)Theme::WIN_H));
            dim.setFillColor(sf::Color(0, 0, 0, 160));
            w.draw(dim);
            sf::RectangleShape panel;
            panel.setSize(sf::Vector2f(440.f, 220.f));
            panel.setPosition(Theme::WIN_W / 2.f - 220.f, Theme::WIN_H / 2.f - 130.f);
            panel.setFillColor(Theme::PANEL_ALPHA);
            panel.setOutlineColor(Theme::HEADER);
            panel.setOutlineThickness(2.f);
            w.draw(panel);
            sf::RectangleShape stripe;
            stripe.setSize(sf::Vector2f(440.f, 5.f));
            stripe.setPosition(Theme::WIN_W / 2.f - 220.f, Theme::WIN_H / 2.f - 130.f);
            stripe.setFillColor(Theme::ACCENT);
            w.draw(stripe);
            bcLbl.draw(w);
            bcField.draw(w);
            btnBcSend.draw(w);
            btnBcCancel.draw(w);
        }
        dialog.draw(w);
    }
};

Screen* makeAdminHome() { return new ScreenAdminHome(); }
