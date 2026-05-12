
#include "GuiCore.h"
#include "Widgets.h"

class ScreenAddCity : public Screen
{
    Label titleLbl;
    Label nameLbl, latLbl, lonLbl, statusLbl;
    TextField nameField, latField, lonField;
    Button btnGeocode, btnSave, btnBack;
    Dialog dialog;

    bool gotCoords = false;

public:
    string title() const override { return "Add City"; }

    void onEnter() override
    {
        sf::Font* f = ctx->font;
        gotCoords = false;

        titleLbl = Label(f, "Add a New City", 40.f, 100.f, 24);
        titleLbl.setStyle(sf::Text::Bold);

        float cx = (Theme::WIN_W - 500.f) / 2.f;
        float y0 = 170.f;

        nameLbl = Label(f, "City Name", cx, y0, 13);
        nameLbl.setColor(Theme::TEXT_MUTED);
        nameField = TextField(f, cx, y0 + 20.f, 380.f, 38.f);
        nameField.setPlaceholder("e.g. Murree");

        btnGeocode = Button(f, "Geocode", cx + 390.f, y0 + 20.f, 110.f, 38.f, Button::PRIMARY);

        statusLbl = Label(f, "Type a city name and click Geocode to fetch coordinates.",
            cx, y0 + 72.f, 12);
        statusLbl.setColor(Theme::TEXT_MUTED);
        statusLbl.setStyle(sf::Text::Italic);

        latLbl = Label(f, "Latitude", cx, y0 + 110.f, 13);
        latLbl.setColor(Theme::TEXT_MUTED);
        latField = TextField(f, cx, y0 + 130.f, 240.f, 38.f);
        latField.setPlaceholder("e.g. 33.6844");

        lonLbl = Label(f, "Longitude", cx + 260.f, y0 + 110.f, 13);
        lonLbl.setColor(Theme::TEXT_MUTED);
        lonField = TextField(f, cx + 260.f, y0 + 130.f, 240.f, 38.f);
        lonField.setPlaceholder("e.g. 73.0479");

        btnSave = Button(f, "Save City", cx, y0 + 200.f, 500.f, 44.f, Button::SUCCESS);

        btnBack = Button(f, "< Back", 40.f,
            Theme::WIN_H - Theme::FOOTER_H - 56.f,
            100.f, 38.f, Button::NEUTRAL);

        dialog.setFont(f);
    }

    void doGeocode()
    {
        string name = nameField.getText();
        if (name.empty())
        {
            dialog.show("Missing", "Please type a city name first.", Dialog::ERROR_KIND);
            return;
        }

        double lat = 0, lon = 0;
        bool ok = geocode(name, lat, lon);
        if (!ok)
        {
            statusLbl.setText("Geocoding failed. Enter coordinates manually if you know them.");
            statusLbl.setColor(Theme::BTN_DANGER);
            return;
        }
        char latBuf[32], lonBuf[32];
        snprintf(latBuf, sizeof(latBuf), "%.6f", lat);
        snprintf(lonBuf, sizeof(lonBuf), "%.6f", lon);
        latField.setText(latBuf);
        lonField.setText(lonBuf);
        gotCoords = true;
        statusLbl.setText("Coordinates fetched. Click Save to add city.");
        statusLbl.setColor(Theme::BTN_SUCCESS);
    }

    void doSave()
    {
        string name = nameField.getText();
        string latS = latField.getText();
        string lonS = lonField.getText();

        if (name.empty() || latS.empty() || lonS.empty())
        {
            dialog.show("Missing Fields", "Name, latitude, and longitude are all required.",
                Dialog::ERROR_KIND);
            return;
        }

        double lat = 0, lon = 0;
        try
        {
            lat = stod(latS);
            lon = stod(lonS);
        }
        catch (...)
        {
            dialog.show("Invalid Coordinates", "Lat/Lon must be valid numbers.",
                Dialog::ERROR_KIND);
            return;
        }

        if (lat < -90 || lat > 90)
        {
            dialog.show("Invalid Latitude", "Latitude must be between -90 and 90.",
                Dialog::ERROR_KIND);
            return;
        }
        if (lon < -180 || lon > 180)
        {
            dialog.show("Invalid Longitude", "Longitude must be between -180 and 180.",
                Dialog::ERROR_KIND);
            return;
        }

        PlannedPackage helper;
        helper.setNewcity(name, lat, lon);

        dialog.show("City Saved!",
            "City has been added to cities.csv.\n"
            "Name: " +
            name + "\n"
            "Coordinates: " +
            latS + ", " + lonS + "\n\n"
            "You can now add packages to this destination.",
            Dialog::SUCCESS);

        nameField.setText("");
        latField.setText("");
        lonField.setText("");
        gotCoords = false;
    }

    void handleEvent(const sf::Event& e) override
    {
        if (dialog.isOpen())
        {
            dialog.handleEvent(e);
            return;
        }

        nameField.handleEvent(e);
        latField.handleEvent(e);
        lonField.handleEvent(e);

        if (btnGeocode.handleEvent(e))
            doGeocode();
        if (btnSave.handleEvent(e))
            doSave();
        if (btnBack.handleEvent(e))
            ctx->back();
    }

    void update(float dt) override
    {
        nameField.update(dt);
        latField.update(dt);
        lonField.update(dt);
    }

    void draw(sf::RenderWindow& w) override
    {
        float cx = (Theme::WIN_W - 540.f) / 2.f;
        sf::RectangleShape panel;
        panel.setSize(sf::Vector2f(540.f, 320.f));
        panel.setPosition(cx - 20.f, 150.f);
        panel.setFillColor(Theme::PANEL_ALPHA);
        panel.setOutlineColor(Theme::DIVIDER);
        panel.setOutlineThickness(1.f);
        w.draw(panel);
        sf::RectangleShape accent;
        accent.setSize(sf::Vector2f(4.f, 320.f));
        accent.setPosition(cx - 20.f, 150.f);
        accent.setFillColor(Theme::BTN_SUCCESS);
        w.draw(accent);

        titleLbl.draw(w);
        nameLbl.draw(w);
        nameField.draw(w);
        btnGeocode.draw(w);
        statusLbl.draw(w);
        latLbl.draw(w);
        latField.draw(w);
        lonLbl.draw(w);
        lonField.draw(w);
        btnSave.draw(w);
        btnBack.draw(w);
        dialog.draw(w);
    }
};

Screen* makeAddCity() { return new ScreenAddCity(); }
