#include "GuiCore.h"
#include "Widgets.h"

class ScreenCustomApproval : public Screen
{
    struct Row
    {
        string id, user, from, to, hotel, nights, car, depDate, total, status;
    };

    vector<Row> all;
    vector<Row> pending;
    Table table;
    Label titleLbl, countLbl, hintLbl;
    Button btnRefresh, btnApprove, btnReject, btnBack;
    Dialog dialog;

    static string trimWS(string s)
    {
        while (!s.empty() && (s.back() < 32 || s.back() == 127))
            s.pop_back();
        size_t a = 0;
        while (a < s.size() && (s[a] < 32 || s[a] == 127))
            a++;
        return s.substr(a);
    }

    void loadCSV()
    {
        all.clear();
        pending.clear();
        ifstream f("custom_packages.csv");
        if (!f.is_open())
        {
            countLbl.setText("No custom_packages.csv yet.");
            countLbl.setColor(Theme::TEXT_MUTED);
            table.setHeaders({ "ID", "User", "From", "To", "Total", "Status" });
            table.setRows({});
            return;
        }
        string line;
        getline(f, line);
        while (getline(f, line))
        {
            if (line.empty())
                continue;
            stringstream ss(line);
            Row r;
            getline(ss, r.id, ',');
            getline(ss, r.user, ',');
            getline(ss, r.from, ',');
            getline(ss, r.to, ',');
            getline(ss, r.hotel, ',');
            getline(ss, r.nights, ',');
            getline(ss, r.car, ',');
            getline(ss, r.depDate, ',');
            getline(ss, r.total, ',');
            getline(ss, r.status, ',');
            r.id = trimWS(r.id);
            r.user = trimWS(r.user);
            r.from = trimWS(r.from);
            r.to = trimWS(r.to);
            r.hotel = trimWS(r.hotel);
            r.nights = trimWS(r.nights);
            r.car = trimWS(r.car);
            r.depDate = trimWS(r.depDate);
            r.total = trimWS(r.total);
            r.status = trimWS(r.status);
            all.push_back(r);
            if (r.status == "PENDING")
                pending.push_back(r);
        }
        rebuildTable();
    }

    void rebuildTable()
    {
        table.setHeaders({ "ID", "Customer", "Route", "Hotel", "Nights", "Car", "Departure", "Total" });
        vector<vector<string>> rows;
        for (const Row& r : pending)
        {
            rows.push_back({ r.id, r.user, r.from + " -> " + r.to,
                            r.hotel, r.nights, r.car, r.depDate,
                            "Rs. " + r.total });
        }
        table.setRows(rows);
        size_t n = pending.size();
        countLbl.setText(to_string(n) + " pending request" + (n == 1 ? "" : "s"));
        countLbl.setColor(n == 0 ? Theme::TEXT_MUTED : Theme::HEADER);
    }

    void rewriteCSV(const string& targetID, const string& newStatus)
    {
        for (Row& r : all)
            if (r.id == targetID)
                r.status = newStatus;

        ofstream f("custom_packages.csv");
        if (!f.is_open())
        {
            dialog.show("File Error", "Could not write custom_packages.csv.", Dialog::ERROR_KIND);
            return;
        }
        f << "PackageID,CustomerUsername,Start,Destination,HotelCategory,"
            "Nights,CarRental,DepartureDate,TotalCost,Status\n";
        for (const Row& r : all)
            f << r.id << "," << r.user << "," << r.from << "," << r.to << ","
            << r.hotel << "," << r.nights << "," << r.car << ","
            << r.depDate << "," << r.total << "," << r.status << "\n";
        f.close();
    }

    void doApprove()
    {
        int sel = table.getSelectedIndex();
        if (sel < 0 || sel >= (int)pending.size())
        {
            dialog.show("No Selection", "Click a row first.", Dialog::ERROR_KIND);
            return;
        }
        string id = pending[sel].id;
        rewriteCSV(id, "APPROVED");
        dialog.show("Approved",
            "Request " + id + " has been approved.\n"
            "Customer: " +
            pending[sel].user,
            Dialog::SUCCESS);
        loadCSV();
    }

    void doReject()
    {
        int sel = table.getSelectedIndex();
        if (sel < 0 || sel >= (int)pending.size())
        {
            dialog.show("No Selection", "Click a row first.", Dialog::ERROR_KIND);
            return;
        }
        string id = pending[sel].id;
        rewriteCSV(id, "REJECTED");
        dialog.show("Rejected",
            "Request " + id + " has been rejected.\n"
            "Customer: " +
            pending[sel].user,
            Dialog::INFO);
        loadCSV();
    }

public:
    string title() const override { return "Custom Approvals"; }

    void onEnter() override
    {
        sf::Font* f = ctx->font;

        titleLbl = Label(f, "Pending Custom Requests", 40.f, 100.f, 24);
        titleLbl.setStyle(sf::Text::Bold);

        countLbl = Label(f, "Loading...", 40.f, 138.f, 13);
        countLbl.setStyle(sf::Text::Bold);

        hintLbl = Label(f, "Click a row, then Approve or Reject.",
            40.f, 165.f, 11);
        hintLbl.setColor(Theme::TEXT_MUTED);
        hintLbl.setStyle(sf::Text::Italic);

        btnRefresh = Button(f, "Refresh",
            Theme::WIN_W - 130.f - 40.f, 134.f,
            130.f, 36.f, Button::NEUTRAL);

        table = Table(f, 40.f, 200.f, Theme::WIN_W - 80.f, 340.f);

        btnBack = Button(f, "< Back", 40.f,
            Theme::WIN_H - Theme::FOOTER_H - 56.f,
            100.f, 38.f, Button::NEUTRAL);
        btnApprove = Button(f, "Approve",
            Theme::WIN_W - 260.f - 40.f,
            Theme::WIN_H - Theme::FOOTER_H - 56.f,
            120.f, 42.f, Button::SUCCESS);
        btnReject = Button(f, "Reject",
            Theme::WIN_W - 130.f - 40.f,
            Theme::WIN_H - Theme::FOOTER_H - 56.f,
            120.f, 42.f, Button::DANGER);

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

        table.handleEvent(e);
        if (btnRefresh.handleEvent(e))
            loadCSV();
        if (btnBack.handleEvent(e))
            ctx->back();
        if (btnApprove.handleEvent(e))
            doApprove();
        if (btnReject.handleEvent(e))
            doReject();
    }

    void draw(sf::RenderWindow& w) override
    {
        titleLbl.draw(w);
        countLbl.draw(w);
        hintLbl.draw(w);
        btnRefresh.draw(w);
        table.draw(w);
        btnBack.draw(w);
        btnApprove.draw(w);
        btnReject.draw(w);
        dialog.draw(w);
    }
};

Screen* makeCustomApproval() { return new ScreenCustomApproval(); }
