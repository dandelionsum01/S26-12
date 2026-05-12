#include "GuiCore.h"
#include "Widgets.h"

class ScreenMyBookings : public Screen
{
    Label titleLbl, summaryLbl, hintLbl;
    Table table;
    Button btnBack, btnRefresh;
    int totalSpent = 0;
    int tripCount = 0;

    static string trimWS(string s)
    {
        while (!s.empty() && (s.back() == '\r' || s.back() == '\n' ||
            s.back() == ' ' || s.back() == '\t'))
            s.pop_back();
        return s;
    }

    void loadBookings()
    {
        totalSpent = 0;
        tripCount = 0;

        table.setHeaders({ "Booking ID", "Package", "Departure",
                          "Amount", "Booked On" });

        ifstream f("bookings.csv");
        if (!f.is_open())
        {
            table.setRows({});
            summaryLbl.setText("No bookings file found yet.");
            summaryLbl.setColor(Theme::TEXT_MUTED);
            return;
        }

        string line;
        getline(f, line); // header

        vector<vector<string>> rows;
        while (getline(f, line))
        {
            if (line.empty())
                continue;
            stringstream ss(line);
            string bid, uname, pid, dep, pay, bookDate;
            getline(ss, bid, ',');
            getline(ss, uname, ',');
            getline(ss, pid, ',');
            getline(ss, dep, ',');
            getline(ss, pay, ',');
            getline(ss, bookDate, ',');

            if (trimWS(uname) != ctx->username)
                continue;

            int p = 0;
            try
            {
                p = stoi(trimWS(pay));
            }
            catch (...)
            {
                p = 0;
            }

            rows.push_back({ trimWS(bid),
                            trimWS(pid),
                            trimWS(dep),
                            "Rs. " + to_string(p),
                            trimWS(bookDate) });
            totalSpent += p;
            tripCount++;
        }

        table.setRows(rows);

        if (tripCount == 0)
        {
            summaryLbl.setText("You have no bookings yet. "
                "Browse packages to make your first trip.");
            summaryLbl.setColor(Theme::TEXT_MUTED);
        }
        else
        {
            summaryLbl.setText(
                to_string(tripCount) + " booking" +
                (tripCount == 1 ? "" : "s") +
                "  /  Total spent: Rs. " + to_string(totalSpent));
            summaryLbl.setColor(Theme::BTN_SUCCESS);
        }
    }

public:
    string title() const override { return "My Bookings"; }

    void onEnter() override
    {
        sf::Font* f = ctx->font;

        titleLbl = Label(f, "My Bookings", 40.f, 100.f, 24);
        titleLbl.setStyle(sf::Text::Bold);

        summaryLbl = Label(f, "Loading...", 40.f, 138.f, 13);
        summaryLbl.setStyle(sf::Text::Bold);

        hintLbl = Label(f,
            "Hover or click a row to highlight a trip. Scroll to see more.",
            40.f, 165.f, 11);
        hintLbl.setColor(Theme::TEXT_MUTED);
        hintLbl.setStyle(sf::Text::Italic);

        btnRefresh = Button(f, "Refresh",
            Theme::WIN_W - 140.f - 40.f, 134.f,
            140.f, 36.f, Button::NEUTRAL);

        table = Table(f, 40.f, 200.f, Theme::WIN_W - 80.f, 400.f);

        btnBack = Button(f, "< Back", 40.f,
            Theme::WIN_H - Theme::FOOTER_H - 56.f,
            100.f, 38.f, Button::NEUTRAL);

        loadBookings();
    }

    void handleEvent(const sf::Event& e) override
    {
        if (btnBack.handleEvent(e))
            ctx->back();
        if (btnRefresh.handleEvent(e))
            loadBookings();
        table.handleEvent(e);
    }

    void draw(sf::RenderWindow& w) override
    {
        titleLbl.draw(w);
        summaryLbl.draw(w);
        hintLbl.draw(w);
        btnRefresh.draw(w);
        table.draw(w);
        btnBack.draw(w);
    }
};

Screen* makeMyBookings() { return new ScreenMyBookings(); }
