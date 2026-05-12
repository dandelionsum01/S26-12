#include "GuiCore.h"
#include "Widgets.h"

class ScreenReviews : public Screen
{
    enum Mode
    {
        READ,
        ADD
    };
    Mode mode = READ;

    Label titleLbl;
    Button tabRead, tabAdd, btnBack;

    Label readPkgLbl;
    TextField readPkgField;
    Button btnFetch;
    Table reviewTable;
    Label readMsgLbl;

    Label addPkgLbl, addReviewLbl;
    TextField addPkgField, addReviewField;
    Button btnSubmit;
    Label addHintLbl;

    Dialog dialog;

    static string trimWS(string s)
    {
        while (!s.empty() && (s.back() < 32 || s.back() == 127))
            s.pop_back();
        size_t start = 0;
        while (start < s.size() && (s[start] < 32 || s[start] == 127))
            start++;
        return s.substr(start);
    }

    void loadReviewsForPackage(const string& pkgID)
    {
        ifstream f("reviews.csv");
        if (!f.is_open())
        {
            reviewTable.setHeaders({ "Customer", "Review" });
            reviewTable.setRows({});
            readMsgLbl.setText("No reviews file found yet.");
            readMsgLbl.setColor(Theme::TEXT_MUTED);
            return;
        }
        string line;
        getline(f, line);

        vector<vector<string>> rows;
        while (getline(f, line))
        {
            if (line.empty())
                continue;
            stringstream ss(line);
            string user, pkg, comment;
            getline(ss, user, ',');
            getline(ss, pkg, ',');
            getline(ss, comment);
            if (trimWS(pkg) == pkgID)
                rows.push_back({ trimWS(user), comment });
        }
        reviewTable.setHeaders({ "Customer", "Review" });
        reviewTable.setRows(rows);
        if (rows.empty())
        {
            readMsgLbl.setText("No reviews yet for " + pkgID);
            readMsgLbl.setColor(Theme::TEXT_MUTED);
        }
        else
        {
            readMsgLbl.setText(to_string(rows.size()) +
                " review" + (rows.size() == 1 ? "" : "s") +
                " for " + pkgID);
            readMsgLbl.setColor(Theme::BTN_SUCCESS);
        }
    }

    bool hasUserBookedPackage(const string& user, const string& pkgID)
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

    void doAddReview()
    {
        string pkg = trimWS(addPkgField.getText());
        string comment = addReviewField.getText();

        if (pkg.empty() || comment.empty())
        {
            dialog.show("Missing Fields",
                "Please enter package ID and review text.",
                Dialog::ERROR_KIND);
            return;
        }

        if (!hasUserBookedPackage(ctx->username, pkg))
        {
            dialog.show("Cannot Review",
                "You can only review packages you have booked.\n"
                "Book this package first, then come back.",
                Dialog::ERROR_KIND);
            return;
        }

        string cleaned;
        for (char c : comment)
        {
            if (c == ',')
                cleaned += ';';
            else if (c == '\n' || c == '\r')
                cleaned += ' ';
            else
                cleaned += c;
        }

        bool isEmpty = false;
        {
            ifstream chk("reviews.csv");
            if (!chk.is_open())
                isEmpty = true;
            else
            {
                chk.seekg(0, ios::end);
                isEmpty = (chk.tellg() == 0);
            }
        }
        ofstream f("reviews.csv", ios::app);
        if (!f.is_open())
        {
            dialog.show("File Error",
                "Could not open reviews.csv.",
                Dialog::ERROR_KIND);
            return;
        }
        if (isEmpty)
            f << "Username,PackageID,Comment\n";
        f << ctx->username << "," << pkg << "," << cleaned << "\n";
        f.close();

        dialog.show("Review Posted!",
            "Your review for " + pkg + " has been added.",
            Dialog::SUCCESS);

        addReviewField.setText("");
    }

    void buildLayout()
    {
        sf::Font* f = ctx->font;

        titleLbl = Label(f,
            mode == READ ? "Read Reviews" : "Add a Review",
            40.f, 100.f, 24);
        titleLbl.setStyle(sf::Text::Bold);

        tabRead = Button(f, "Read", 40.f, 145.f, 130.f, 34.f,
            mode == READ ? Button::PRIMARY : Button::NEUTRAL);
        tabAdd = Button(f, "Add", 175.f, 145.f, 130.f, 34.f,
            mode == ADD ? Button::PRIMARY : Button::NEUTRAL);

        btnBack = Button(f, "< Back", 40.f,
            Theme::WIN_H - Theme::FOOTER_H - 56.f,
            100.f, 38.f, Button::NEUTRAL);

        dialog.setFont(f);

        if (mode == READ)
        {
            readPkgLbl = Label(f, "Package ID", 40.f, 200.f, 13);
            readPkgLbl.setColor(Theme::TEXT_MUTED);
            readPkgField = TextField(f, 40.f, 220.f, 280.f, 38.f);
            readPkgField.setPlaceholder("e.g. P001  (not BK-P001-1)");
            btnFetch = Button(f, "Search", 330.f, 220.f, 100.f, 38.f, Button::PRIMARY);

            readMsgLbl = Label(f, "Enter a package ID and click Search.",
                40.f, 270.f, 12);
            readMsgLbl.setColor(Theme::TEXT_MUTED);
            readMsgLbl.setStyle(sf::Text::Italic);

            reviewTable = Table(f, 40.f, 300.f, Theme::WIN_W - 80.f, 290.f);
            reviewTable.setHeaders({ "Customer", "Review" });
        }
        else
        {
            addPkgLbl = Label(f,
                "Package ID -- e.g. P001  (NOT the Booking ID like BK-P001-1)",
                40.f, 200.f, 13);
            addPkgLbl.setColor(Theme::TEXT_MUTED);
            addPkgField = TextField(f, 40.f, 220.f, 280.f, 38.f);
            addPkgField.setPlaceholder("e.g. P001");

            addReviewLbl = Label(f, "Your review", 40.f, 282.f, 13);
            addReviewLbl.setColor(Theme::TEXT_MUTED);
            addReviewField = TextField(f, 40.f, 302.f, Theme::WIN_W - 80.f, 80.f);
            addReviewField.setPlaceholder("Tell others about your trip...");
            addReviewField.setMaxLen(300);

            addHintLbl = Label(f,
                "Reviews are gated by booking history -- only customers who "
                "booked the package can review it.",
                40.f, 392.f, 11);
            addHintLbl.setColor(Theme::TEXT_MUTED);
            addHintLbl.setStyle(sf::Text::Italic);

            btnSubmit = Button(f, "Post Review", 40.f, 430.f,
                240.f, 42.f, Button::SUCCESS);
        }
    }

public:
    string title() const override
    {
        return mode == READ ? "Read Reviews" : "Add Review";
    }

    void onEnter() override
    {
        mode = READ;
        buildLayout();
    }

    void handleEvent(const sf::Event& e) override
    {
        if (dialog.isOpen())
        {
            dialog.handleEvent(e);
            return;
        }

        if (tabRead.handleEvent(e) && mode != READ)
        {
            mode = READ;
            buildLayout();
            return;
        }
        if (tabAdd.handleEvent(e) && mode != ADD)
        {
            mode = ADD;
            buildLayout();
            return;
        }
        if (btnBack.handleEvent(e))
        {
            ctx->back();
            return;
        }

        if (mode == READ)
        {
            readPkgField.handleEvent(e);
            reviewTable.handleEvent(e);
            if (btnFetch.handleEvent(e))
            {
                string pkg = trimWS(readPkgField.getText());
                if (pkg.empty())
                {
                    dialog.show("Missing", "Please enter a Package ID.",
                        Dialog::ERROR_KIND);
                    return;
                }
                loadReviewsForPackage(pkg);
            }
        }
        else
        {
            addPkgField.handleEvent(e);
            addReviewField.handleEvent(e);
            if (btnSubmit.handleEvent(e))
                doAddReview();
        }
    }

    void update(float dt) override
    {
        if (mode == READ)
            readPkgField.update(dt);
        else
        {
            addPkgField.update(dt);
            addReviewField.update(dt);
        }
    }

    void draw(sf::RenderWindow& w) override
    {
        titleLbl.draw(w);
        tabRead.draw(w);
        tabAdd.draw(w);

        if (mode == READ)
        {
            readPkgLbl.draw(w);
            readPkgField.draw(w);
            btnFetch.draw(w);
            readMsgLbl.draw(w);
            reviewTable.draw(w);
        }
        else
        {
            addPkgLbl.draw(w);
            addPkgField.draw(w);
            addReviewLbl.draw(w);
            addReviewField.draw(w);
            addHintLbl.draw(w);
            btnSubmit.draw(w);
        }

        btnBack.draw(w);
        dialog.draw(w);
    }
};

Screen* makeReviews() { return new ScreenReviews(); }