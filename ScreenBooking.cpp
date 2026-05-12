#include "GuiCore.h"
#include "Widgets.h"

class ScreenBooking : public Screen
{
    Label titleLbl;
    Label fromLbl, toLbl, hotelLbl, deptLbl;
    Label fromVal, toVal, hotelVal, deptVal;
    Label seatsAvailLbl, seatsAvailVal;

    Label basePriceLbl, demandLbl, finalPriceLbl, seatsTotalLbl;
    Label basePriceVal, demandVal, finalPriceVal, seatsTotalVal;

    Label dateLbl, seatsLbl, cardLbl;
    TextField dateField, seatsField, cardField;
    Button btnPickDate;
    Label attemptsLbl;

    Button btnBack, btnConfirm;
    Dialog dialog;
    Calendar calendar;

    int cardAttempts = 0;
    int finalPrice = 0;
    int seatsLeft = 0;
    int bookedNow = 0;
    string priceNote;

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
        tm* n = localtime(&t);
        char buf[20];
        strftime(buf, sizeof(buf), "%Y-%m-%d", n);
        return string(buf);
    }
    static int countBookingsFor(const string& pkgID)
    {
        ifstream f("bookings.csv");
        if (!f.is_open())
            return 0;
        int n = 0;
        string line;
        getline(f, line);
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
    static bool alreadyBooked(const string& user, const string& pkgID)
    {
        ifstream f("bookings.csv");
        if (!f.is_open())
            return false;
        string line;
        getline(f, line);
        while (getline(f, line))
        {
            stringstream ss(line);
            string bid, uname, pid;
            getline(ss, bid, ',');
            getline(ss, uname, ',');
            getline(ss, pid, ',');
            if (trimWS(uname) == user && trimWS(pid) == pkgID)
                return true;
        }
        return false;
    }

    int parsedSeats() const
    {
        int s = 1;
        try
        {
            s = stoi(seatsField.getText());
        }
        catch (...)
        {
        }
        if (s < 1)
            s = 1;
        if (s > seatsLeft)
            s = seatsLeft;
        return s;
    }

    void computePricing()
    {
        bookedNow = countBookingsFor(ctx->selectedPackageID);
        seatsLeft = ctx->selectedCapacity - bookedNow;
        if (seatsLeft < 0)
            seatsLeft = 0;

        double fill = (ctx->selectedCapacity > 0)
            ? (double)bookedNow / ctx->selectedCapacity
            : 0.0;
        finalPrice = ctx->selectedPrice;
        priceNote.clear();
        if (fill > 0.8)
        {
            finalPrice = (int)(ctx->selectedPrice * 1.2);
            priceNote = "High Demand (+20%)";
        }
        else if (fill < 0.5)
        {
            finalPrice = (int)(ctx->selectedPrice * 0.9);
            priceNote = "Early Bird (-10%)";
        }
        else
            priceNote = "Standard rate";
    }

    void refreshTotal()
    {
        int seats = parsedSeats();
        int total = finalPrice * seats;
        seatsTotalVal.setText("Rs. " + to_string(total) +
            "  (" + to_string(seats) + " x Rs. " +
            to_string(finalPrice) + ")");
    }

    void buildLayout()
    {
        sf::Font* f = ctx->font;
        titleLbl = Label(f, "Confirm Your Booking", 40.f, 100.f, 24);
        titleLbl.setStyle(sf::Text::Bold);

        float lx = 50.f, vx = 200.f, y = 155.f, dy = 30.f;

        fromLbl = Label(f, "From:", lx, y, 13);
        fromLbl.setColor(Theme::TEXT_MUTED);
        fromVal = Label(f, ctx->selectedStart, vx, y, 14);

        toLbl = Label(f, "To:", lx, y + dy, 13);
        toLbl.setColor(Theme::TEXT_MUTED);
        toVal = Label(f, ctx->selectedDestination, vx, y + dy, 14);

        hotelLbl = Label(f, "Hotel:", lx, y + 2 * dy, 13);
        hotelLbl.setColor(Theme::TEXT_MUTED);
        hotelVal = Label(f, ctx->selectedHotel, vx, y + 2 * dy, 14);

        deptLbl = Label(f, "Departure:", lx, y + 3 * dy, 13);
        deptLbl.setColor(Theme::TEXT_MUTED);
        deptVal = Label(f, ctx->selectedDepartureDate, vx, y + 3 * dy, 14);

        seatsAvailLbl = Label(f, "Seats Left:", lx, y + 4 * dy, 13);
        seatsAvailLbl.setColor(Theme::TEXT_MUTED);
        seatsAvailVal = Label(f, to_string(seatsLeft) + " available",
            vx, y + 4 * dy, 14);
        if (seatsLeft == 0)
            seatsAvailVal.setColor(Theme::BTN_DANGER);
        else
            seatsAvailVal.setColor(Theme::BTN_SUCCESS);

        basePriceLbl = Label(f, "Base Price:", lx, y + 5 * dy + 8.f, 13);
        basePriceLbl.setColor(Theme::TEXT_MUTED);
        basePriceVal = Label(f, "Rs. " + to_string(ctx->selectedPrice), vx, y + 5 * dy + 8.f, 14);

        demandLbl = Label(f, "Adjustment:", lx, y + 6 * dy + 8.f, 13);
        demandLbl.setColor(Theme::TEXT_MUTED);
        demandVal = Label(f, priceNote, vx, y + 6 * dy + 8.f, 14);
        if (priceNote.find("High") != string::npos)
            demandVal.setColor(Theme::BTN_DANGER);
        else if (priceNote.find("Early") != string::npos)
            demandVal.setColor(Theme::BTN_SUCCESS);

        finalPriceLbl = Label(f, "Price/Seat:", lx, y + 7 * dy + 8.f, 13);
        finalPriceLbl.setColor(Theme::TEXT_MUTED);
        finalPriceVal = Label(f, "Rs. " + to_string(finalPrice), vx, y + 7 * dy + 8.f, 16);
        finalPriceVal.setStyle(sf::Text::Bold);

        seatsTotalLbl = Label(f, "TOTAL:", lx, y + 8 * dy + 18.f, 14);
        seatsTotalLbl.setStyle(sf::Text::Bold);
        seatsTotalVal = Label(f, "Rs. " + to_string(finalPrice), vx, y + 8 * dy + 14.f, 22);
        seatsTotalVal.setStyle(sf::Text::Bold);
        seatsTotalVal.setColor(Theme::ACCENT);  // burgundy — contrasts against dark panel

        float rx = 560.f, ry = 155.f;

        seatsLbl = Label(f, "Number of Seats  (max " + to_string(seatsLeft) + ")",
            rx, ry, 13);
        seatsLbl.setColor(Theme::TEXT_MUTED);
        seatsField = TextField(f, rx, ry + 20.f, 120.f, 38.f);
        seatsField.setPlaceholder("1");
        seatsField.setNumeric(true);
        seatsField.setMaxLen(2);
        seatsField.setText("1");

        dateLbl = Label(f, "Departure Date (override if needed)", rx, ry + 78.f, 13);
        dateLbl.setColor(Theme::TEXT_MUTED);
        dateField = TextField(f, rx, ry + 98.f, 240.f, 36.f);
        dateField.setText(ctx->selectedDepartureDate);
        btnPickDate = Button(f, "Pick", rx + 250.f, ry + 98.f, 80.f, 36.f, Button::PRIMARY);

        cardLbl = Label(f, "Credit Card Number", rx, ry + 160.f, 13);
        cardLbl.setColor(Theme::TEXT_MUTED);
        cardField = TextField(f, rx, ry + 180.f, 370.f, 38.f);
        cardField.setPlaceholder("e.g. 4111-1111-1111-1111");
        cardField.setMaxLen(24);

        attemptsLbl = Label(f, "", rx, ry + 228.f, 12);
        attemptsLbl.setColor(Theme::BTN_DANGER);

        btnBack = Button(f, "< Back", 40.f,
            Theme::WIN_H - Theme::FOOTER_H - 56.f,
            100.f, 38.f, Button::NEUTRAL);
        btnConfirm = Button(f, "Confirm & Pay",
            Theme::WIN_W - 220.f - 40.f,
            Theme::WIN_H - Theme::FOOTER_H - 56.f,
            220.f, 44.f, Button::SUCCESS);
        if (seatsLeft == 0)
            btnConfirm.setEnabled(false);

        dialog.setFont(f);
        calendar.setFont(f);
        calendar.setPos((Theme::WIN_W - 320) / 2.f, 130.f);
    }

    void doConfirm()
    {
        if (alreadyBooked(ctx->username, ctx->selectedPackageID))
        {
            dialog.show("Already Booked",
                "You already have a booking for this package.",
                Dialog::ERROR_KIND);
            return;
        }
        if (seatsLeft <= 0)
        {
            dialog.show("Sold Out",
                "This package has no seats left.",
                Dialog::ERROR_KIND);
            return;
        }

        int seats = parsedSeats();
        if (seats < 1 || seats > seatsLeft)
        {
            dialog.show("Invalid Seats",
                "Please enter between 1 and " +
                to_string(seatsLeft) + " seats.",
                Dialog::ERROR_KIND);
            return;
        }

        string card = cardField.getText();
        if (card.empty())
        {
            dialog.show("Card Required",
                "Please enter your credit card number.",
                Dialog::ERROR_KIND);
            return;
        }

        Booking b;
        if (!b.checkCreditNum(card))
        {
            cardAttempts++;
            int left = 3 - cardAttempts;
            if (left <= 0)
            {
                dialog.show("Booking Cancelled",
                    "Too many invalid card attempts.\n"
                    "Booking cancelled.",
                    Dialog::ERROR_KIND);
                cardField.setText("");
                cardAttempts = 0;
                ctx->go(ScreenID::PACKAGES);
                return;
            }
            attemptsLbl.setText("Invalid card. " + to_string(left) +
                " attempt" + (left == 1 ? "" : "s") + " remaining.");
            cardField.setText("");
            return;
        }

        string useDate = dateField.getText();
        if (useDate.empty())
            useDate = ctx->selectedDepartureDate;

        int n = countBookingsFor(ctx->selectedPackageID) + 1;
        string bookingID = "BK-" + ctx->selectedPackageID + "-" + to_string(n);
        int totalPayment = finalPrice * seats;

        bool isEmpty = true;
        {
            ifstream chk("bookings.csv");
            if (chk.is_open())
            {
                chk.seekg(0, ios::end);
                isEmpty = (chk.tellg() == 0);
            }
        }
        ofstream out("bookings.csv", ios::app);
        if (!out.is_open())
        {
            dialog.show("File Error", "Could not open bookings.csv.", Dialog::ERROR_KIND);
            return;
        }
        if (isEmpty)
            out << "BookingID,CustomerUsername,PackageID,DepartureDate,Payment,BookingDate\n";
        out << bookingID << "," << ctx->username << ","
            << ctx->selectedPackageID << "," << useDate << ","
            << totalPayment << "," << today() << "\n";
        out.close();

        for (int i = 0; i < seats; ++i)
            b.updateCapacity(ctx->selectedPackageID);

        dialog.show("Booking Confirmed!",
            "Booking ID:   " + bookingID + "\n"
            "Package:      " +
            ctx->selectedPackageID + "\n"
            "Route:        " +
            ctx->selectedStart + " -> " + ctx->selectedDestination + "\n"
            "Seats booked: " +
            to_string(seats) + "\n"
            "Departure:    " +
            useDate + "\n"
            "Total Paid:   Rs. " +
            to_string(totalPayment) + "\n\n"
            "Thank you for booking with us!",
            Dialog::SUCCESS);
    }

public:
    string title() const override { return "Booking"; }

    void onEnter() override
    {
        cardAttempts = 0;
        if (ctx->selectedPackageID.empty())
        {
            ctx->go(ScreenID::PACKAGES);
            return;
        }
        computePricing();
        buildLayout();
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
                ctx->go(ScreenID::CUST_HOME);
            return;
        }

        dateField.handleEvent(e);
        seatsField.handleEvent(e);
        cardField.handleEvent(e);

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
        if (btnConfirm.handleEvent(e))
            doConfirm();

        refreshTotal();
    }

    void update(float dt) override
    {
        dateField.update(dt);
        seatsField.update(dt);
        cardField.update(dt);
    }

    void draw(sf::RenderWindow& w) override
    {
        sf::RectangleShape p1;
        p1.setSize(sf::Vector2f(490.f, 420.f));
        p1.setPosition(30.f, 130.f);
        p1.setFillColor(Theme::PANEL_ALPHA);
        p1.setOutlineColor(Theme::DIVIDER);
        p1.setOutlineThickness(1.f);
        w.draw(p1);
        sf::RectangleShape a1;
        a1.setSize(sf::Vector2f(4.f, 420.f));
        a1.setPosition(30.f, 130.f);
        a1.setFillColor(Theme::ACCENT);
        w.draw(a1);

        sf::RectangleShape p2;
        p2.setSize(sf::Vector2f(470.f, 420.f));
        p2.setPosition(545.f, 130.f);
        p2.setFillColor(Theme::PANEL_ALPHA);
        p2.setOutlineColor(Theme::DIVIDER);
        p2.setOutlineThickness(1.f);
        w.draw(p2);
        sf::RectangleShape a2;
        a2.setSize(sf::Vector2f(4.f, 420.f));
        a2.setPosition(545.f, 130.f);
        a2.setFillColor(Theme::BTN_PRIMARY);
        w.draw(a2);

        titleLbl.draw(w);
        fromLbl.draw(w);
        fromVal.draw(w);
        toLbl.draw(w);
        toVal.draw(w);
        hotelLbl.draw(w);
        hotelVal.draw(w);
        deptLbl.draw(w);
        deptVal.draw(w);
        seatsAvailLbl.draw(w);
        seatsAvailVal.draw(w);
        basePriceLbl.draw(w);
        basePriceVal.draw(w);
        demandLbl.draw(w);
        demandVal.draw(w);
        finalPriceLbl.draw(w);
        finalPriceVal.draw(w);

        sf::RectangleShape div;
        div.setSize(sf::Vector2f(430.f, 1.f));
        div.setPosition(45.f, 435.f);
        div.setFillColor(Theme::DIVIDER);
        w.draw(div);

        seatsTotalLbl.draw(w);
        seatsTotalVal.draw(w);

        seatsLbl.draw(w);
        seatsField.draw(w);
        dateLbl.draw(w);
        dateField.draw(w);
        btnPickDate.draw(w);
        cardLbl.draw(w);
        cardField.draw(w);
        attemptsLbl.draw(w);

        btnBack.draw(w);
        btnConfirm.draw(w);
        dialog.draw(w);
        calendar.draw(w);
    }
};

Screen* makeBooking() { return new ScreenBooking(); }