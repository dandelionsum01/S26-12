#include "GuiCore.h"
#include "Widgets.h"

class ScreenCustom : public Screen
{
    Label titleLbl;

    Label fromLbl, toLbl, hotelLbl, nightsLbl, carLbl, peopleLbl, dateLbl;
    TextField fromField, toField, nightsField, peopleField, dateField;
    Button btnHBudget, btnHStandard, btnHLuxury;
    Button btnCarYes, btnCarNo;
    Button btnPickDate;

    Label billTitleLbl;
    Label hotelCostLbl, hotelCostVal;
    Label carCostLbl, carCostVal;
    Label routeCostLbl, routeCostVal;
    Label distLbl, distVal;
    Label totalLbl, totalVal;
    Label noteLbl;

    Button btnBack, btnSubmit;
    Dialog dialog;
    Calendar calendar;

    HotelCategory chosenHotel = HotelCategory::STANDARD;
    bool chosenCar = false;

    int cachedNights = 0;
    int cachedHotelCost = 0;
    int cachedCarCost = 0;
    int cachedRouteCost = 0;
    int cachedDistance = 0;
    int cachedTotal = 0;

    static int hotelPrice(HotelCategory hc)
    {
        switch (hc)
        {
        case HotelCategory::BUDGET:
            return 1500;
        case HotelCategory::STANDARD:
            return 3000;
        case HotelCategory::LUXURY:
            return 6000;
        }
        return 3000;
    }

    static string toLower(string s)
    {
        for (auto& c : s)
            c = (char)tolower(c);
        return s;
    }

    static bool lookupCity(const string& name, double& lat, double& lon)
    {
        if (name.empty())
            return false;
        PlannedPackage helper;
        int count = 0;
        City* cities = helper.loadCities(count);
        if (!cities || count == 0)
        {
            delete[] cities;
            return false;
        }

        string target = toLower(name);
        bool found = false;
        for (int i = 0; i < count; ++i)
        {
            if (toLower(cities[i].name) == target)
            {
                lat = cities[i].lat;
                lon = cities[i].lon;
                found = true;
                break;
            }
        }
        delete[] cities;
        return found;
    }

    void recompute()
    {
        cachedNights = 0;
        try
        {
            cachedNights = stoi(nightsField.getText());
        }
        catch (...)
        {
        }
        if (cachedNights < 0)
            cachedNights = 0;

        cachedHotelCost = hotelPrice(chosenHotel) * cachedNights;
        cachedCarCost = chosenCar ? (3500 * cachedNights) : 0;

        int people = 1;
        try
        {
            people = stoi(peopleField.getText());
        }
        catch (...)
        {
        }
        if (people < 1)
            people = 1;

        double lat1 = 0, lon1 = 0, lat2 = 0, lon2 = 0;
        bool gotFrom = lookupCity(fromField.getText(), lat1, lon1);
        bool gotTo = lookupCity(toField.getText(), lat2, lon2);
        cachedDistance = 0;
        if (gotFrom && gotTo)
        {
            PlannedPackage helper;
            cachedDistance = (int)helper.calculateDistance(lat1, lon1, lat2, lon2);
        }
        cachedRouteCost = cachedDistance * 10;
        cachedTotal = (cachedHotelCost * people) + cachedCarCost + cachedRouteCost;

        hotelCostVal.setText("Rs. " + to_string(cachedHotelCost));
        carCostVal.setText(chosenCar ? "Rs. " + to_string(cachedCarCost)
            : "Not included");
        carCostVal.setColor(chosenCar ? Theme::TEXT_DARK : Theme::TEXT_MUTED);
        distVal.setText(cachedDistance > 0
            ? to_string(cachedDistance) + " km"
            : "(both cities required)");
        distVal.setColor(cachedDistance > 0 ? Theme::TEXT_DARK : Theme::TEXT_MUTED);
        routeCostVal.setText("Rs. " + to_string(cachedRouteCost));
        totalVal.setText("Rs. " + to_string(cachedTotal) +
            (people > 1 ? "  (hotel x" + to_string(people) + " people)" : ""));

        if (cachedDistance == 0 && (!fromField.getText().empty() ||
            !toField.getText().empty()))
            noteLbl.setText("Tip: city names must exist in cities.csv");
        else
            noteLbl.setText("");
    }

    void refreshToggles()
    {
        btnHBudget.setVariant(chosenHotel == HotelCategory::BUDGET
            ? Button::PRIMARY
            : Button::NEUTRAL);
        btnHStandard.setVariant(chosenHotel == HotelCategory::STANDARD
            ? Button::PRIMARY
            : Button::NEUTRAL);
        btnHLuxury.setVariant(chosenHotel == HotelCategory::LUXURY
            ? Button::PRIMARY
            : Button::NEUTRAL);
        btnCarYes.setVariant(chosenCar ? Button::SUCCESS : Button::NEUTRAL);
        btnCarNo.setVariant(!chosenCar ? Button::DANGER : Button::NEUTRAL);
    }

public:
    string title() const override { return "Custom Package"; }

    void onEnter() override
    {
        sf::Font* f = ctx->font;

        chosenHotel = HotelCategory::STANDARD;
        chosenCar = false;

        titleLbl = Label(f, "Design Your Trip", 40.f, 94.f, 24);
        titleLbl.setStyle(sf::Text::Bold);

        const float lx = 50.f;

        fromLbl = Label(f, "From City", lx, 150.f, 13);
        fromLbl.setColor(Theme::TEXT_MUTED);
        fromField = TextField(f, lx, 170.f, 400.f, 36.f);
        fromField.setPlaceholder("e.g. Karachi");

        toLbl = Label(f, "To City", lx, 220.f, 13);
        toLbl.setColor(Theme::TEXT_MUTED);
        toField = TextField(f, lx, 240.f, 400.f, 36.f);
        toField.setPlaceholder("e.g. Hunza");

        hotelLbl = Label(f, "Hotel Category", lx, 292.f, 13);
        hotelLbl.setColor(Theme::TEXT_MUTED);
        const float hbw = 125.f, hbh = 50.f, hgap = 8.f;
        btnHBudget = Button(f, "Budget", lx, 312.f, hbw, hbh);
        btnHStandard = Button(f, "Standard", lx + hbw + hgap, 312.f, hbw, hbh);
        btnHLuxury = Button(f, "Luxury", lx + 2 * (hbw + hgap), 312.f, hbw, hbh);
        btnHBudget.setSubtitle("Rs. 1,500 / night");
        btnHStandard.setSubtitle("Rs. 3,000 / night");
        btnHLuxury.setSubtitle("Rs. 6,000 / night");

        nightsLbl = Label(f, "Number of Nights", lx, 378.f, 13);
        nightsLbl.setColor(Theme::TEXT_MUTED);
        nightsField = TextField(f, lx, 398.f, 120.f, 36.f);
        nightsField.setPlaceholder("e.g. 5");
        nightsField.setNumeric(true);
        nightsField.setMaxLen(3);

        peopleLbl = Label(f, "Number of People", lx + 145.f, 378.f, 13);
        peopleLbl.setColor(Theme::TEXT_MUTED);
        peopleField = TextField(f, lx + 145.f, 398.f, 100.f, 36.f);
        peopleField.setPlaceholder("1");
        peopleField.setNumeric(true);
        peopleField.setMaxLen(2);
        peopleField.setText("1");

        carLbl = Label(f, "Car Rental", lx, 450.f, 13);
        carLbl.setColor(Theme::TEXT_MUTED);
        btnCarYes = Button(f, "Yes", lx, 470.f, 120.f, 50.f);
        btnCarYes.setSubtitle("Rs. 3,500 / day");
        btnCarNo = Button(f, "No", lx + 128.f, 470.f, 80.f, 50.f);

        dateLbl = Label(f, "Departure Date", lx, 536.f, 13);
        dateLbl.setColor(Theme::TEXT_MUTED);
        dateField = TextField(f, lx, 556.f, 210.f, 36.f);
        dateField.setPlaceholder("YYYY-MM-DD");
        btnPickDate = Button(f, "Pick Date", lx + 218.f, 556.f, 115.f, 36.f,
            Button::PRIMARY);

        const float rx = 580.f, ry = 150.f;
        billTitleLbl = Label(f, "Live Estimate", rx, ry, 19);
        billTitleLbl.setStyle(sf::Text::Bold);

        const float bxLbl = rx, bxVal = rx + 185.f;
        const float by = ry + 56.f, bdy = 46.f;

        hotelCostLbl = Label(f, "Hotel cost:", bxLbl, by, 13);
        hotelCostLbl.setColor(Theme::TEXT_MUTED);
        hotelCostVal = Label(f, "Rs. 0", bxVal, by, 14);

        carCostLbl = Label(f, "Car rental:", bxLbl, by + bdy, 13);
        carCostLbl.setColor(Theme::TEXT_MUTED);
        carCostVal = Label(f, "Not included", bxVal, by + bdy, 14);
        carCostVal.setColor(Theme::TEXT_MUTED);

        distLbl = Label(f, "Distance:", bxLbl, by + 2 * bdy, 13);
        distLbl.setColor(Theme::TEXT_MUTED);
        distVal = Label(f, "(enter both cities)", bxVal, by + 2 * bdy, 13);
        distVal.setColor(Theme::TEXT_MUTED);

        routeCostLbl = Label(f, "Route cost:", bxLbl, by + 3 * bdy, 13);
        routeCostLbl.setColor(Theme::TEXT_MUTED);
        routeCostVal = Label(f, "Rs. 0", bxVal, by + 3 * bdy, 14);

        totalLbl = Label(f, "TOTAL ESTIMATE", bxLbl, by + 4 * bdy + 28.f, 13);
        totalLbl.setStyle(sf::Text::Bold);
        totalLbl.setColor(Theme::TEXT_MUTED);
        totalVal = Label(f, "Rs. 0", bxLbl, by + 4 * bdy + 50.f, 30);
        totalVal.setStyle(sf::Text::Bold);
        totalVal.setColor(Theme::ACCENT); // burgundy pop

        noteLbl = Label(f, "", rx, by + 5 * bdy + 60.f, 11);
        noteLbl.setColor(Theme::BTN_DANGER);
        noteLbl.setStyle(sf::Text::Italic);

        btnBack = Button(f, "< Back", 40.f,
            Theme::WIN_H - Theme::FOOTER_H - 56.f,
            100.f, 38.f, Button::NEUTRAL);
        btnSubmit = Button(f, "Submit Request",
            Theme::WIN_W - 220.f - 40.f,
            Theme::WIN_H - Theme::FOOTER_H - 56.f,
            220.f, 44.f, Button::SUCCESS);

        dialog.setFont(f);
        calendar.setFont(f);
        calendar.setPos((Theme::WIN_W - 320) / 2.f, 130.f);

        refreshToggles();
        recompute();
    }

    void doSubmit()
    {
        string from = fromField.getText();
        string to = toField.getText();
        string date = dateField.getText();
        if (from.empty() || to.empty() || nightsField.getText().empty() ||
            date.empty())
        {
            dialog.show("Missing Fields",
                "Please fill all fields:\n"
                "From, To, Nights, Departure Date.",
                Dialog::ERROR_KIND);
            return;
        }
        if (cachedNights <= 0)
        {
            dialog.show("Invalid Nights",
                "Number of nights must be at least 1.",
                Dialog::ERROR_KIND);
            return;
        }
        if (cachedDistance == 0)
        {
            dialog.show("City Not Found",
                "One or both cities are not in cities.csv.\n"
                "Please use a known city name.",
                Dialog::ERROR_KIND);
            return;
        }

        auto take = [](const string& s, size_t n)
            {
                return s.substr(0, min(n, s.size()));
            };
        string id = "C-" + take(ctx->username, 3) + take(to, 3);

        bool isEmpty = false;
        {
            ifstream chk("custom_packages.csv");
            if (!chk.is_open())
                isEmpty = true;
            else
            {
                chk.seekg(0, ios::end);
                isEmpty = (chk.tellg() == 0);
            }
        }
        ofstream f("custom_packages.csv", ios::app);
        if (!f.is_open())
        {
            dialog.show("File Error",
                "Could not open custom_packages.csv.",
                Dialog::ERROR_KIND);
            return;
        }
        if (isEmpty)
        {
            f << "PackageID,CustomerUsername,Start,Destination,"
                "HotelCategory,Nights,CarRental,DepartureDate,"
                "TotalCost,Status\n";
        }
        string hotelStr = CustomPackage::CategoryToString(chosenHotel);
        f << id << "," << ctx->username << ","
            << from << "," << to << ","
            << hotelStr << "," << cachedNights << ","
            << (chosenCar ? "Yes" : "No") << ","
            << date << "," << cachedTotal << ",PENDING\n";
        f.close();

        dialog.show("Request Submitted!",
            "Custom package request created.\n"
            "ID: " +
            id + "\n"
            "Estimated Total: Rs. " +
            to_string(cachedTotal) + "\n\n"
            "Status: PENDING\n"
            "An admin will review your request soon.",
            Dialog::SUCCESS);
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
            if (!dialog.isOpen())
            {
                Dialog::Result r = dialog.result();
                if (r == Dialog::OK)
                    ctx->go(ScreenID::CUST_HOME);
            }
            return;
        }

        fromField.handleEvent(e);
        toField.handleEvent(e);
        nightsField.handleEvent(e);
        peopleField.handleEvent(e);
        dateField.handleEvent(e);

        if (btnHBudget.handleEvent(e))
        {
            chosenHotel = HotelCategory::BUDGET;
            refreshToggles();
        }
        if (btnHStandard.handleEvent(e))
        {
            chosenHotel = HotelCategory::STANDARD;
            refreshToggles();
        }
        if (btnHLuxury.handleEvent(e))
        {
            chosenHotel = HotelCategory::LUXURY;
            refreshToggles();
        }
        if (btnCarYes.handleEvent(e))
        {
            chosenCar = true;
            refreshToggles();
        }
        if (btnCarNo.handleEvent(e))
        {
            chosenCar = false;
            refreshToggles();
        }

        if (btnPickDate.handleEvent(e))
            calendar.openAt(0, 0);
        if (calendar.hasSelection())
        {
            dateField.setText(calendar.getSelected());
            calendar.clearResult();
        }

        if (btnBack.handleEvent(e))
        {
            ctx->back();
            return;
        }
        if (btnSubmit.handleEvent(e))
            doSubmit();

        recompute();
    }

    void update(float dt) override
    {
        fromField.update(dt);
        toField.update(dt);
        nightsField.update(dt);
        peopleField.update(dt);
        dateField.update(dt);
    }

    void draw(sf::RenderWindow& w) override
    {
        sf::RectangleShape p1;
        p1.setSize(sf::Vector2f(490.f, 470.f));
        p1.setPosition(30.f, 130.f);
        p1.setFillColor(Theme::PANEL_ALPHA);
        p1.setOutlineColor(Theme::DIVIDER);
        p1.setOutlineThickness(1.f);
        w.draw(p1);
        sf::RectangleShape a1;
        a1.setSize(sf::Vector2f(5.f, 470.f));
        a1.setPosition(30.f, 130.f);
        a1.setFillColor(Theme::ACCENT);
        w.draw(a1);

        sf::RectangleShape p2;
        p2.setSize(sf::Vector2f(510.f, 470.f));
        p2.setPosition(555.f, 130.f);
        p2.setFillColor(Theme::PANEL_ALPHA);
        p2.setOutlineColor(Theme::DIVIDER);
        p2.setOutlineThickness(1.f);
        w.draw(p2);
        sf::RectangleShape a2;
        a2.setSize(sf::Vector2f(5.f, 470.f));
        a2.setPosition(555.f, 130.f);
        a2.setFillColor(Theme::BTN_SUCCESS);
        w.draw(a2);

        titleLbl.draw(w);
        fromLbl.draw(w);
        fromField.draw(w);
        toLbl.draw(w);
        toField.draw(w);
        hotelLbl.draw(w);
        btnHBudget.draw(w);
        btnHStandard.draw(w);
        btnHLuxury.draw(w);
        nightsLbl.draw(w);
        nightsField.draw(w);
        peopleLbl.draw(w);
        peopleField.draw(w);
        carLbl.draw(w);
        btnCarYes.draw(w);
        btnCarNo.draw(w);
        dateLbl.draw(w);
        dateField.draw(w);
        btnPickDate.draw(w);

        billTitleLbl.draw(w);
        hotelCostLbl.draw(w);
        hotelCostVal.draw(w);
        carCostLbl.draw(w);
        carCostVal.draw(w);
        distLbl.draw(w);
        distVal.draw(w);
        routeCostLbl.draw(w);
        routeCostVal.draw(w);

        sf::RectangleShape div;
        div.setSize(sf::Vector2f(470.f, 1.f));
        div.setPosition(575.f, 396.f);
        div.setFillColor(Theme::DIVIDER);
        w.draw(div);

        totalLbl.draw(w);
        totalVal.draw(w);
        noteLbl.draw(w);

        btnBack.draw(w);
        btnSubmit.draw(w);
        dialog.draw(w);
        calendar.draw(w);
    }
};

Screen* makeCustomPackage() { return new ScreenCustom(); }