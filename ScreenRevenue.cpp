#include "GuiCore.h"
#include "Widgets.h"

class ScreenRevenue : public Screen
{
    Label titleLbl;
    Label totalRevLbl, totalRevVal;
    Label totalBookLbl, totalBookVal;
    Label avgLbl, avgVal;
    Label sortLbl;
    Table custTable;
    Button btnRefresh, btnBack;

    static string trimWS(string s)
    {
        while (!s.empty() && (s.back() < 32 || s.back() == 127))
            s.pop_back();
        size_t a = 0;
        while (a < s.size() && (s[a] < 32 || s[a] == 127))
            a++;
        return s.substr(a);
    }

    void loadStats()
    {
        ifstream f("bookings.csv");
        if (!f.is_open())
        {
            totalRevVal.setText("Rs. 0");
            totalBookVal.setText("0");
            avgVal.setText("Rs. 0");
            custTable.setRows({});
            return;
        }

        long long total = 0;
        int count = 0;
        struct Pay
        {
            string user;
            long long amt;
        };
        vector<Pay> rows;

        string line;
        getline(f, line);
        while (getline(f, line))
        {
            if (line.empty())
                continue;
            stringstream ss(line);
            string bid, uname, pid, dep, pay, bd;
            getline(ss, bid, ',');
            getline(ss, uname, ',');
            getline(ss, pid, ',');
            getline(ss, dep, ',');
            getline(ss, pay, ',');
            getline(ss, bd, ',');
            long long p = 0;
            try
            {
                p = stoll(trimWS(pay));
            }
            catch (...)
            {
                p = 0;
            }
            total += p;
            count++;
            rows.push_back({ trimWS(uname), p });
        }

        sort(rows.begin(), rows.end(),
            [](const Pay& a, const Pay& b)
            { return a.amt < b.amt; });

        totalRevVal.setText("Rs. " + to_string(total));
        totalBookVal.setText(to_string(count));
        long long avg = (count > 0) ? (total / count) : 0;
        avgVal.setText("Rs. " + to_string(avg));

        custTable.setHeaders({ "#", "Customer", "Payment" });
        vector<vector<string>> tableRows;
        for (size_t i = 0; i < rows.size(); ++i)
        {
            tableRows.push_back({ to_string(i + 1),
                                 rows[i].user,
                                 "Rs. " + to_string(rows[i].amt) });
        }
        custTable.setRows(tableRows);
    }

public:
    string title() const override { return "Revenue Report"; }

    void onEnter() override
    {
        sf::Font* f = ctx->font;

        titleLbl = Label(f, "Revenue & Payments", 40.f, 100.f, 24);
        titleLbl.setStyle(sf::Text::Bold);

        float cy = 150.f, cw = 300.f, ch = 90.f, cgap = 20.f;
        float cx = 40.f;

        totalRevLbl = Label(f, "TOTAL REVENUE", cx + 18.f, cy + 14.f, 11);
        totalRevLbl.setColor(Theme::TEXT_MUTED);
        totalRevLbl.setStyle(sf::Text::Bold);
        totalRevVal = Label(f, "Rs. 0", cx + 18.f, cy + 38.f, 28);
        totalRevVal.setStyle(sf::Text::Bold);
        totalRevVal.setColor(Theme::TEXT_LINK);

        totalBookLbl = Label(f, "TOTAL BOOKINGS", cx + cw + cgap + 18.f, cy + 14.f, 11);
        totalBookLbl.setColor(Theme::TEXT_MUTED);
        totalBookLbl.setStyle(sf::Text::Bold);
        totalBookVal = Label(f, "0", cx + cw + cgap + 18.f, cy + 38.f, 28);
        totalBookVal.setStyle(sf::Text::Bold);
        totalBookVal.setColor(Theme::BTN_SUCCESS);

        avgLbl = Label(f, "AVG PAYMENT", cx + 2 * (cw + cgap) + 18.f, cy + 14.f, 11);
        avgLbl.setColor(Theme::TEXT_MUTED);
        avgLbl.setStyle(sf::Text::Bold);
        avgVal = Label(f, "Rs. 0", cx + 2 * (cw + cgap) + 18.f, cy + 38.f, 28);
        avgVal.setStyle(sf::Text::Bold);
        avgVal.setColor(Theme::ACCENT);

        sortLbl = Label(f, "Customers sorted by payment (ascending):",
            40.f, 270.f, 12);
        sortLbl.setColor(Theme::TEXT_MUTED);
        sortLbl.setStyle(sf::Text::Italic);

        custTable = Table(f, 40.f, 295.f, Theme::WIN_W - 80.f, 290.f);

        btnRefresh = Button(f, "Refresh",
            Theme::WIN_W - 140.f - 40.f, 250.f,
            140.f, 36.f, Button::NEUTRAL);

        btnBack = Button(f, "< Back", 40.f,
            Theme::WIN_H - Theme::FOOTER_H - 56.f,
            100.f, 38.f, Button::NEUTRAL);

        loadStats();
    }

    void handleEvent(const sf::Event& e) override
    {
        custTable.handleEvent(e);
        if (btnRefresh.handleEvent(e))
            loadStats();
        if (btnBack.handleEvent(e))
            ctx->back();
    }

    void draw(sf::RenderWindow& w) override
    {
        float cy = 150.f, cw = 300.f, ch = 90.f, cgap = 20.f;
        for (int i = 0; i < 3; ++i)
        {
            float cx = 40.f + i * (cw + cgap);
            sf::RectangleShape card;
            card.setSize(sf::Vector2f(cw, ch));
            card.setPosition(cx, cy);
            card.setFillColor(Theme::PANEL_ALPHA);
            card.setOutlineColor(Theme::DIVIDER);
            card.setOutlineThickness(1.f);
            w.draw(card);

            sf::Color stripeColor =
                (i == 0) ? Theme::TEXT_LINK : (i == 1) ? Theme::BTN_SUCCESS
                : Theme::ACCENT;
            sf::RectangleShape stripe;
            stripe.setSize(sf::Vector2f(4.f, ch));
            stripe.setPosition(cx, cy);
            stripe.setFillColor(stripeColor);
            w.draw(stripe);
        }

        titleLbl.draw(w);
        totalRevLbl.draw(w);
        totalRevVal.draw(w);
        totalBookLbl.draw(w);
        totalBookVal.draw(w);
        avgLbl.draw(w);
        avgVal.draw(w);
        sortLbl.draw(w);
        btnRefresh.draw(w);
        custTable.draw(w);
        btnBack.draw(w);
    }
};

Screen* makeRevenue() { return new ScreenRevenue(); }
