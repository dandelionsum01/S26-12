#include "GuiCore.h"
#include "Widgets.h"

class ScreenPackages : public Screen
{
    struct Row
    {
        string id, from, to, hotel, depDate, expiry, category;
        int price = 0, capacity = 0, seatsLeft = 0;
    };

    vector<Row> allRows;     // raw from CSV
    vector<Row> visibleRows; // after filter
    Table table;
    Label titleLbl, countLbl, searchLbl;
    TextField searchField;
    Button btnBack, btnRefresh, btnBook, btnClearSearch;
    Dialog dialog;

    string lastSearch;
    bool loadOK = false;

    static string trimWS(string s)
    {
        while (!s.empty() && (s.back() == '\r' || s.back() == '\n' ||
            s.back() == ' ' || s.back() == '\t'))
            s.pop_back();
        return s;
    }
    static string today()
    {
        time_t t = time(0);
        tm* now = localtime(&t);
        char buf[20];
        strftime(buf, sizeof(buf), "%Y-%m-%d", now);
        return string(buf);
    }
    static int countBookingsFor(const string& pkgID)
    {
        ifstream f("bookings.csv");
        if (!f.is_open())
            return 0;
        int n = 0;
        string line;
        getline(f, line); // header
        while (getline(f, line))
        {
            stringstream ss(line);
            string bid, uname, pid;
            getline(ss, bid, ',');
            getline(ss, uname, ',');
            getline(ss, pid, ',');
            if (trimWS(pid) == pkgID)
                n++;
        }
        return n;
    }

    void loadCSV()
    {
        allRows.clear();
        ifstream f("packages.csv");
        if (!f.is_open())
        {
            loadOK = false;
            return;
        }

        string line;
        getline(f, line); // header

        while (getline(f, line))
        {
            if (line.empty())
                continue;
            stringstream ss(line);
            string id, from, to, price, hotel, cap, dep, exp, cat;
            getline(ss, id, ',');
            getline(ss, from, ',');
            getline(ss, to, ',');
            getline(ss, price, ',');
            getline(ss, hotel, ',');
            getline(ss, cap, ',');
            getline(ss, dep, ',');
            getline(ss, exp, ',');
            getline(ss, cat, ',');

            Row r;
            r.id = trimWS(id);
            r.from = trimWS(from);
            r.to = trimWS(to);
            r.hotel = trimWS(hotel);
            r.depDate = trimWS(dep);
            r.expiry = trimWS(exp);
            r.category = trimWS(cat);
            try
            {
                r.price = stoi(trimWS(price));
                r.capacity = stoi(trimWS(cap));
            }
            catch (...)
            {
                continue;
            }

            int booked = countBookingsFor(r.id);
            r.seatsLeft = r.capacity - booked;
            if (r.seatsLeft < 0)
                r.seatsLeft = 0;

            allRows.push_back(r);
        }
        loadOK = true;
        applyFilter();
    }

    void applyFilter()
    {
        visibleRows.clear();
        string td = today();
        string q = lastSearch;
        for (auto& c : q)
            c = (char)tolower(c);

        for (const auto& r : allRows)
        {
            if (!r.expiry.empty() && r.expiry < td)
                continue;
            if (r.seatsLeft <= 0)
                continue;
            if (!q.empty())
            {
                string blob = r.id + " " + r.from + " " + r.to + " " + r.category;
                for (auto& c : blob)
                    c = (char)tolower(c);
                if (blob.find(q) == string::npos)
                    continue;
            }
            visibleRows.push_back(r);
        }
        rebuildTable();
    }

    void rebuildTable()
    {
        table.setHeaders({ "ID", "From", "To", "Price", "Hotel", "Seats", "Departure", "Category" });
        vector<vector<string>> rows;
        for (const auto& r : visibleRows)
        {
            rows.push_back({ r.id, r.from, r.to,
                            "Rs. " + to_string(r.price),
                            r.hotel,
                            to_string(r.seatsLeft) + " / " + to_string(r.capacity),
                            r.depDate,
                            r.category });
        }
        table.setRows(rows);
        countLbl.setText(to_string(visibleRows.size()) + " package" +
            (visibleRows.size() == 1 ? "" : "s") +
            " available");
    }

public:
    string title() const override { return "Browse Packages"; }

    void onEnter() override
    {
        sf::Font* f = ctx->font;

        titleLbl = Label(f, "Available Packages", 40.f, 100.f, 24);
        titleLbl.setStyle(sf::Text::Bold);

        countLbl = Label(f, "Loading...", 40.f, 132.f, 13);
        countLbl.setColor(Theme::TEXT_MUTED);

        searchLbl = Label(f, "Search", 40.f, 165.f, 12);
        searchField = TextField(f, 40.f, 184.f, 360.f, 36.f);
        searchField.setPlaceholder("e.g. Hunza, Karachi, Northern, P001...");

        btnClearSearch = Button(f, "Clear", 410.f, 184.f, 80.f, 36.f, Button::NEUTRAL);
        btnRefresh = Button(f, "Refresh", 500.f, 184.f, 90.f, 36.f, Button::NEUTRAL);

        table = Table(f, 40.f, 240.f, Theme::WIN_W - 80.f, 350.f);

        btnBack = Button(f, "< Back", 40.f,
            Theme::WIN_H - Theme::FOOTER_H - 56.f,
            100.f, 38.f, Button::NEUTRAL);

        btnBook = Button(f, "Book Selected",
            Theme::WIN_W - 200.f - 40.f,
            Theme::WIN_H - Theme::FOOTER_H - 56.f,
            200.f, 38.f, Button::SUCCESS);
        btnBook.setEnabled(false);

        dialog.setFont(f);
        loadCSV();
    }

    void handleEvent(const sf::Event& e) override
    {
        if (dialog.isOpen())
        {
            dialog.handleEvent(e);
            return;
        }

        searchField.handleEvent(e);
        table.handleEvent(e);

        if (searchField.getText() != lastSearch)
        {
            lastSearch = searchField.getText();
            applyFilter();
        }

        if (btnClearSearch.handleEvent(e))
        {
            searchField.setText("");
            lastSearch.clear();
            applyFilter();
        }

        if (btnRefresh.handleEvent(e))
            loadCSV();

        if (btnBack.handleEvent(e))
            ctx->back();

        int sel = table.getSelectedIndex();
        btnBook.setEnabled(sel >= 0 && sel < (int)visibleRows.size());

        if (btnBook.handleEvent(e))
        {
            if (sel >= 0 && sel < (int)visibleRows.size())
            {
                const Row& r = visibleRows[sel];
                ctx->selectedPackageID = r.id;
                ctx->selectedStart = r.from;
                ctx->selectedDestination = r.to;
                ctx->selectedPrice = r.price;
                ctx->selectedHotel = r.hotel;
                ctx->selectedCapacity = r.capacity;
                ctx->selectedDepartureDate = r.depDate;
                ctx->go(ScreenID::BOOKING);
            }
        }
    }

    void update(float dt) override { searchField.update(dt); }

    void draw(sf::RenderWindow& w) override
    {
        titleLbl.draw(w);
        countLbl.draw(w);
        searchLbl.draw(w);
        searchField.draw(w);
        btnClearSearch.draw(w);
        btnRefresh.draw(w);
        table.draw(w);
        btnBack.draw(w);
        btnBook.draw(w);
        dialog.draw(w);

        if (!loadOK)
        {
            sf::Font& f = *ctx->font;
            sf::Text msg("Could not open packages.csv", f, 14);
            msg.setFillColor(Theme::BTN_DANGER);
            msg.setPosition(40.f, 410.f);
            w.draw(msg);
        }
    }
};

Screen* makePackages() { return new ScreenPackages(); }
