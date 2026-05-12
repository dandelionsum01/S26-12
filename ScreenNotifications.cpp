#include "GuiCore.h"
#include "Widgets.h"

class ScreenNotifications : public Screen
{
    Label titleLbl, statusLbl;
    Button btnSubscribe, btnRefresh, btnBack;
    Table notifTable;
    Dialog dialog;

    bool subscribed = false;

    static string trimWS(string s)
    {
        while (!s.empty() && (s.back() == '\r' || s.back() == '\n' ||
            s.back() == ' ' || s.back() == '\t'))
            s.pop_back();
        return s;
    }

    bool isSubscribed()
    {
        ifstream f("subscriptions.csv");
        if (!f.is_open())
            return false;
        string line;
        getline(f, line); // header
        while (getline(f, line))
        {
            if (trimWS(line) == ctx->username)
                return true;
        }
        return false;
    }

    void loadNotifications()
    {
        notifTable.setHeaders({ "#", "Date", "Message" });
        notifTable.setRows({});

        if (!subscribed)
            return;

        ifstream f("notifications.csv");
        if (!f.is_open())
            return;

        string line;
        getline(f, line); // skip header

        vector<vector<string>> rows;
        int idx = 1;
        while (getline(f, line))
        {
            if (line.empty())
                continue;

            stringstream ss(line);
            string uname, message, date, isRead;
            getline(ss, uname, ',');
            getline(ss, message, ',');
            getline(ss, date, ',');
            getline(ss, isRead, ',');

            if (trimWS(uname) != ctx->username)
                continue;

            rows.push_back({ to_string(idx++), trimWS(date), message });
        }
        notifTable.setRows(rows);
    }

    void doSubscribe()
    {
        if (subscribed)
        {
            dialog.show("Already Subscribed",
                "You are already subscribed to notifications.",
                Dialog::INFO);
            return;
        }
        bool isEmpty = false;
        {
            ifstream chk("subscriptions.csv");
            if (!chk.is_open())
                isEmpty = true;
            else
            {
                chk.seekg(0, ios::end);
                isEmpty = (chk.tellg() == 0);
            }
        }
        ofstream f("subscriptions.csv", ios::app);
        if (!f.is_open())
        {
            dialog.show("File Error",
                "Could not open subscriptions.csv.",
                Dialog::ERROR_KIND);
            return;
        }
        if (isEmpty)
            f << "username\n";
        f << ctx->username << "\n";
        f.close();

        subscribed = true;
        statusLbl.setText("Subscribed -- you'll receive package alerts.");
        statusLbl.setColor(Theme::BTN_SUCCESS);
        btnSubscribe.setLabel("Subscribed");
        btnSubscribe.setEnabled(false);

        dialog.show("Subscribed!",
            "You'll now receive notifications when new\n"
            "packages are added or important updates occur.",
            Dialog::SUCCESS);
    }

public:
    string title() const override { return "Notifications"; }

    void onEnter() override
    {
        sf::Font* f = ctx->font;

        titleLbl = Label(f, "Notifications & Alerts", 40.f, 100.f, 24);
        titleLbl.setStyle(sf::Text::Bold);

        subscribed = isSubscribed();

        statusLbl = Label(f,
            subscribed ? "You are subscribed to notifications."
            : "Subscribe to receive package alerts.",
            40.f, 138.f, 13);
        statusLbl.setColor(subscribed ? Theme::BTN_SUCCESS : Theme::TEXT_MUTED);

        btnSubscribe = Button(f,
            subscribed ? "Subscribed" : "Subscribe",
            40.f, 170.f, 200.f, 40.f,
            subscribed ? Button::NEUTRAL : Button::SUCCESS);
        btnSubscribe.setEnabled(!subscribed);

        btnRefresh = Button(f, "Refresh List", 250.f, 170.f, 140.f, 40.f,
            Button::NEUTRAL);

        btnBack = Button(f, "< Back", 40.f,
            Theme::WIN_H - Theme::FOOTER_H - 56.f,
            100.f, 38.f, Button::NEUTRAL);

        notifTable = Table(f, 40.f, 240.f, Theme::WIN_W - 80.f, 360.f);
        loadNotifications();

        dialog.setFont(f);
    }

    void handleEvent(const sf::Event& e) override
    {
        if (dialog.isOpen())
        {
            dialog.handleEvent(e);
            return;
        }

        if (btnSubscribe.handleEvent(e))
        {
            doSubscribe();
            loadNotifications();
        }
        if (subscribed && btnRefresh.handleEvent(e))
            loadNotifications();
        if (btnBack.handleEvent(e))
            ctx->back();

        if (subscribed)
            notifTable.handleEvent(e);
    }

    void draw(sf::RenderWindow& w) override
    {
        titleLbl.draw(w);
        statusLbl.draw(w);
        btnSubscribe.draw(w);

        if (!subscribed)
        {
            sf::RectangleShape lockPanel;
            lockPanel.setSize(sf::Vector2f((float)(Theme::WIN_W - 80), 340.f));
            lockPanel.setPosition(40.f, 235.f);
            lockPanel.setFillColor(Theme::PANEL_ALPHA);
            lockPanel.setOutlineColor(Theme::DIVIDER);
            lockPanel.setOutlineThickness(1.f);
            w.draw(lockPanel);

            sf::Font* fnt = ctx->font;
            if (fnt)
            {
                sf::Text icon("[ Notifications Locked ]", *fnt, 20);
                icon.setStyle(sf::Text::Bold);
                icon.setFillColor(Theme::TEXT_MUTED);
                sf::FloatRect ib = icon.getLocalBounds();
                icon.setPosition((Theme::WIN_W - ib.width) / 2.f, 330.f);
                w.draw(icon);

                sf::Text hint(
                    "Subscribe above to start receiving package alerts and updates.",
                    *fnt, 14);
                hint.setFillColor(Theme::TEXT_MUTED);
                hint.setStyle(sf::Text::Italic);
                sf::FloatRect hb = hint.getLocalBounds();
                hint.setPosition((Theme::WIN_W - hb.width) / 2.f, 372.f);
                w.draw(hint);
            }
        }
        else
        {
            btnRefresh.draw(w);
            notifTable.draw(w);
        }

        btnBack.draw(w);
        dialog.draw(w);
    }
};

Screen* makeNotifications() { return new ScreenNotifications(); }
