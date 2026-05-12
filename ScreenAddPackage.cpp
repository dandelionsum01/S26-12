
#include "GuiCore.h"
#include "Widgets.h"

class ScreenAddPackage : public Screen
{
    Label titleLbl;
    Label fromLbl, toLbl, priceLbl, hotelLbl, capLbl, catLbl, depLbl, expLbl;
    TextField fromField, toField, priceField, hotelField, capField;
    TextField depField, expField;
    Button btnPickDep, btnPickExp;
    Button btnCatN, btnCatS, btnCatC, btnCatCoast;
    Button btnBack, btnSubmit;
    Dialog dialog;
    Calendar calendar;
    int calTarget = 0;
    Category chosenCat = Category::NORTHERN_AREAS;

    static string trimWS(string s)
    {
        while (!s.empty() && (s.back() < 32 || s.back() == 127))
            s.pop_back();
        size_t a = 0;
        while (a < s.size() && (s[a] < 32 || s[a] == 127))
            a++;
        return s.substr(a);
    }
    static string toLower(string s)
    {
        for (auto& c : s)
            c = (char)tolower(c);
        return s;
    }

    bool cityExists(const string& name)
    {
        if (name.empty())
            return false;
        ifstream f("cities.csv");
        if (!f.is_open())
            return false;
        string line, target = toLower(name);
        getline(f, line);
        while (getline(f, line))
        {
            stringstream ss(line);
            string n;
            getline(ss, n, ',');
            if (toLower(trimWS(n)) == target)
                return true;
        }
        return false;
    }

    string makeID(const string& from, const string& to)
    {
        auto code = [](const string& s)
            {
                string t;
                for (char c : s)
                    if (isalpha((unsigned char)c))
                        t += (char)toupper(c);
                return t.substr(0, 3);
            };
        string prefix = code(from) + "-" + code(to) + "-";
        ifstream f("packages.csv");
        int n = 1;
        if (f.is_open())
        {
            string line;
            getline(f, line);
            while (getline(f, line))
            {
                stringstream ss(line);
                string id;
                getline(ss, id, ',');
                id = trimWS(id);
                if (id.find(prefix) == 0)
                {
                    try
                    {
                        int x = stoi(id.substr(prefix.size()));
                        if (x >= n)
                            n = x + 1;
                    }
                    catch (...)
                    {
                    }
                }
            }
        }
        return prefix + to_string(n);
    }

    string catToString(Category c)
    {
        switch (c)
        {
        case Category::NORTHERN_AREAS:
            return "NorthernAreas";
        case Category::SOUTH:
            return "South";
        case Category::CENTRAL:
            return "Central";
        case Category::COASTAL:
            return "Coastal";
        }
        return "Central";
    }

    void refreshCatToggles()
    {
        btnCatN.setVariant(chosenCat == Category::NORTHERN_AREAS ? Button::PRIMARY : Button::NEUTRAL);
        btnCatS.setVariant(chosenCat == Category::SOUTH ? Button::PRIMARY : Button::NEUTRAL);
        btnCatC.setVariant(chosenCat == Category::CENTRAL ? Button::PRIMARY : Button::NEUTRAL);
        btnCatCoast.setVariant(chosenCat == Category::COASTAL ? Button::PRIMARY : Button::NEUTRAL);
    }

    void doSubmit()
    {
        string from = trimWS(fromField.getText());
        string to = trimWS(toField.getText());
        string hotel = hotelField.getText();
        string dep = trimWS(depField.getText());
        string exp = trimWS(expField.getText());

        if (from.empty() || to.empty() || hotel.empty() ||
            priceField.getText().empty() || capField.getText().empty() ||
            dep.empty() || exp.empty())
        {
            dialog.show("Missing Fields", "Please fill all fields.", Dialog::ERROR_KIND);
            return;
        }

        if (!cityExists(from))
        {
            dialog.show("Unknown City",
                "From city '" + from + "' is not in cities.csv.\n"
                "Add it first via the Add City screen.",
                Dialog::ERROR_KIND);
            return;
        }
        if (!cityExists(to))
        {
            dialog.show("Unknown City",
                "To city '" + to + "' is not in cities.csv.\n"
                "Add it first via the Add City screen.",
                Dialog::ERROR_KIND);
            return;
        }

        int price = 0, cap = 0;
        try
        {
            price = stoi(priceField.getText());
        }
        catch (...)
        {
        }
        try
        {
            cap = stoi(capField.getText());
        }
        catch (...)
        {
        }
        if (price <= 0)
        {
            dialog.show("Invalid Price", "Price must be positive.", Dialog::ERROR_KIND);
            return;
        }
        if (cap <= 0)
        {
            dialog.show("Invalid Capacity", "Capacity must be positive.", Dialog::ERROR_KIND);
            return;
        }
        if (exp < dep)
        {
            dialog.show("Invalid Dates", "Expiry must be on or after departure.", Dialog::ERROR_KIND);
            return;
        }

        string id = makeID(from, to);
        string catStr = catToString(chosenCat);

        bool empty = false;
        {
            ifstream chk("packages.csv");
            if (!chk.is_open())
                empty = true;
            else
            {
                chk.seekg(0, ios::end);
                empty = (chk.tellg() == 0);
            }
        }
        ofstream f("packages.csv", ios::app);
        if (!f.is_open())
        {
            dialog.show("File Error", "Could not open packages.csv.", Dialog::ERROR_KIND);
            return;
        }
        if (empty)
            f << "PackageID,Start,Destination,Price,Hotel,Capacity,DepartureDate,ExpiryDate,Category\n";
        f << id << "," << from << "," << to << "," << price << ","
            << hotel << "," << cap << "," << dep << "," << exp << ","
            << catStr << "\n";
        f.close();

        {
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
                        if (!sl.empty()) subs.push_back(sl);
                    }
                }
            }
            if (!subs.empty())
            {
                bool nempty = false;
                {
                    ifstream chk("notifications.csv");
                    if (!chk.is_open()) nempty = true;
                    else { chk.seekg(0, ios::end); nempty = (chk.tellg() == 0); }
                }
                ofstream nf("notifications.csv", ios::app);
                if (nf.is_open())
                {
                    if (nempty) nf << "Username,Message,Date,IsRead\n";
                    time_t t = time(0);
                    tm* tn = localtime(&t);
                    char db[20];
                    strftime(db, sizeof(db), "%Y-%m-%d", tn);
                    string notifMsg = "New package available: " + id +
                        " (" + from + " -> " + to + ")";
                    for (const auto& u : subs)
                        nf << u << "," << notifMsg << "," << db << ",NO\n";
                }
            }
        }

        dialog.show("Package Added!",
            "Package created successfully.\n"
            "ID: " +
            id + "\n"
            "Route: " +
            from + " -> " + to + "\n"
            "Price: Rs. " +
            to_string(price) + "\n"
            "Capacity: " +
            to_string(cap) + "\n\n"
            "Subscribers have been notified.",
            Dialog::SUCCESS);
    }

public:
    string title() const override { return "Add Package"; }

    void onEnter() override
    {
        sf::Font* f = ctx->font;
        chosenCat = Category::NORTHERN_AREAS;

        titleLbl = Label(f, "Add a New Package", 40.f, 100.f, 24);
        titleLbl.setStyle(sf::Text::Bold);

        float lx = 50.f, rx = 540.f, w1 = 440.f, w2 = 440.f;

        fromLbl = Label(f, "From City", lx, 150.f, 13);
        fromLbl.setColor(Theme::TEXT_MUTED);
        fromField = TextField(f, lx, 170.f, w1, 36.f);
        fromField.setPlaceholder("e.g. Lahore");

        toLbl = Label(f, "Destination", lx, 220.f, 13);
        toLbl.setColor(Theme::TEXT_MUTED);
        toField = TextField(f, lx, 240.f, w1, 36.f);
        toField.setPlaceholder("e.g. Hunza");

        priceLbl = Label(f, "Base Price (Rs.)", lx, 290.f, 13);
        priceLbl.setColor(Theme::TEXT_MUTED);
        priceField = TextField(f, lx, 310.f, 200.f, 36.f);
        priceField.setPlaceholder("e.g. 25000");
        priceField.setNumeric(true);
        priceField.setMaxLen(8);

        hotelLbl = Label(f, "Hotel", lx, 360.f, 13);
        hotelLbl.setColor(Theme::TEXT_MUTED);
        hotelField = TextField(f, lx, 380.f, w1, 36.f);
        hotelField.setPlaceholder("e.g. Serena Hotel");

        capLbl = Label(f, "Capacity (seats)", lx, 430.f, 13);
        capLbl.setColor(Theme::TEXT_MUTED);
        capField = TextField(f, lx, 450.f, 200.f, 36.f);
        capField.setPlaceholder("e.g. 30");
        capField.setNumeric(true);
        capField.setMaxLen(4);

        depLbl = Label(f, "Departure Date", rx, 150.f, 13);
        depLbl.setColor(Theme::TEXT_MUTED);
        depField = TextField(f, rx, 170.f, 260.f, 36.f);
        depField.setPlaceholder("YYYY-MM-DD");
        btnPickDep = Button(f, "Pick", rx + 270.f, 170.f, 80.f, 36.f, Button::PRIMARY);

        expLbl = Label(f, "Expiry Date", rx, 220.f, 13);
        expLbl.setColor(Theme::TEXT_MUTED);
        expField = TextField(f, rx, 240.f, 260.f, 36.f);
        expField.setPlaceholder("YYYY-MM-DD");
        btnPickExp = Button(f, "Pick", rx + 270.f, 240.f, 80.f, 36.f, Button::PRIMARY);

        catLbl = Label(f, "Category", rx, 290.f, 13);
        catLbl.setColor(Theme::TEXT_MUTED);
        float cbw = 100.f, cbh = 36.f, cgap = 6.f;
        btnCatN = Button(f, "Northern", rx + 0 * (cbw + cgap), 310.f, cbw, cbh);
        btnCatS = Button(f, "South", rx + 1 * (cbw + cgap), 310.f, cbw, cbh);
        btnCatC = Button(f, "Central", rx + 2 * (cbw + cgap), 310.f, cbw, cbh);
        btnCatCoast = Button(f, "Coastal", rx + 3 * (cbw + cgap), 310.f, cbw, cbh);
        refreshCatToggles();

        btnBack = Button(f, "< Back", 40.f,
            Theme::WIN_H - Theme::FOOTER_H - 56.f,
            100.f, 38.f, Button::NEUTRAL);
        btnSubmit = Button(f, "Add Package",
            Theme::WIN_W - 220.f - 40.f,
            Theme::WIN_H - Theme::FOOTER_H - 56.f,
            220.f, 44.f, Button::SUCCESS);

        dialog.setFont(f);
        calendar.setFont(f);
        calendar.setPos((Theme::WIN_W - 320) / 2.f, 130.f);
    }

    void handleEvent(const sf::Event& e) override
    {
        if (calendar.isOpen())
        {
            calendar.handleEvent(e);
            return;
        }
        if (dialog.isOpen())
        {
            dialog.handleEvent(e);
            if (!dialog.isOpen() && dialog.result() == Dialog::OK)
                ctx->go(ScreenID::ADMIN_HOME);
            return;
        }

        fromField.handleEvent(e);
        toField.handleEvent(e);
        priceField.handleEvent(e);
        hotelField.handleEvent(e);
        capField.handleEvent(e);
        depField.handleEvent(e);
        expField.handleEvent(e);

        if (btnPickDep.handleEvent(e))
        {
            calTarget = 0;
            calendar.openAt(0, 0);
        }
        if (btnPickExp.handleEvent(e))
        {
            calTarget = 1;
            calendar.openAt(0, 0);
        }
        if (calendar.hasSelection())
        {
            if (calTarget == 0)
                depField.setText(calendar.getSelected());
            else
                expField.setText(calendar.getSelected());
            calendar.clearResult();
        }

        if (btnCatN.handleEvent(e))
        {
            chosenCat = Category::NORTHERN_AREAS;
            refreshCatToggles();
        }
        if (btnCatS.handleEvent(e))
        {
            chosenCat = Category::SOUTH;
            refreshCatToggles();
        }
        if (btnCatC.handleEvent(e))
        {
            chosenCat = Category::CENTRAL;
            refreshCatToggles();
        }
        if (btnCatCoast.handleEvent(e))
        {
            chosenCat = Category::COASTAL;
            refreshCatToggles();
        }

        if (btnBack.handleEvent(e))
            ctx->back();
        if (btnSubmit.handleEvent(e))
            doSubmit();
    }

    void update(float dt) override
    {
        fromField.update(dt);
        toField.update(dt);
        priceField.update(dt);
        hotelField.update(dt);
        capField.update(dt);
        depField.update(dt);
        expField.update(dt);
    }

    void draw(sf::RenderWindow& w) override
    {
        sf::RectangleShape p1;
        p1.setSize(sf::Vector2f(490.f, 360.f));
        p1.setPosition(30.f, 140.f);
        p1.setFillColor(Theme::PANEL_ALPHA);
        p1.setOutlineColor(Theme::DIVIDER);
        p1.setOutlineThickness(1.f);
        w.draw(p1);
        sf::RectangleShape a1;
        a1.setSize(sf::Vector2f(4.f, 360.f));
        a1.setPosition(30.f, 140.f);
        a1.setFillColor(Theme::ACCENT);
        w.draw(a1);

        sf::RectangleShape p2;
        p2.setSize(sf::Vector2f(490.f, 360.f));
        p2.setPosition(525.f, 140.f);
        p2.setFillColor(Theme::PANEL_ALPHA);
        p2.setOutlineColor(Theme::DIVIDER);
        p2.setOutlineThickness(1.f);
        w.draw(p2);
        sf::RectangleShape a2;
        a2.setSize(sf::Vector2f(4.f, 360.f));
        a2.setPosition(525.f, 140.f);
        a2.setFillColor(Theme::BTN_PRIMARY);
        w.draw(a2);

        titleLbl.draw(w);
        fromLbl.draw(w);
        fromField.draw(w);
        toLbl.draw(w);
        toField.draw(w);
        priceLbl.draw(w);
        priceField.draw(w);
        hotelLbl.draw(w);
        hotelField.draw(w);
        capLbl.draw(w);
        capField.draw(w);
        depLbl.draw(w);
        depField.draw(w);
        btnPickDep.draw(w);
        expLbl.draw(w);
        expField.draw(w);
        btnPickExp.draw(w);
        catLbl.draw(w);
        btnCatN.draw(w);
        btnCatS.draw(w);
        btnCatC.draw(w);
        btnCatCoast.draw(w);
        btnBack.draw(w);
        btnSubmit.draw(w);
        dialog.draw(w);
        calendar.draw(w);
    }
};

Screen* makeAddPackage() { return new ScreenAddPackage(); }
