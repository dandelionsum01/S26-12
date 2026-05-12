#include "GuiCore.h"
#include "Widgets.h"

class ScreenLauncher : public Screen
{
    Label  titleLbl;
    Button btnCustomer, btnAdmin, btnExit;

    static constexpr float PAN_Y = 155.f;
    static constexpr float PAN_H = 420.f;
    static constexpr float PAN_W = 500.f;

public:
    string title() const override { return ""; }

    void onEnter() override
    {
        sf::Font* f = ctx->font;
        float cx = Theme::WIN_W / 2.f;
        float bw = 420.f, bh = 80.f, gap = 24.f;
        float top = PAN_Y + 82.f;

        titleLbl = Label(f, "Welcome", cx - 60.f, PAN_Y + 22.f, 34);
        titleLbl.setStyle(sf::Text::Bold);
        titleLbl.setColor(Theme::TEXT_DARK);

        btnCustomer = Button(f, "Customer Portal",
            cx - bw / 2.f, top, bw, bh, Button::PRIMARY);
        btnCustomer.setSubtitle("Sign up, browse packages, make bookings");

        btnAdmin = Button(f, "Admin Portal",
            cx - bw / 2.f, top + bh + gap, bw, bh, Button::DANGER);
        btnAdmin.setSubtitle("Manage packages, approve requests, view revenue");

        btnExit = Button(f, "Exit",
            cx - bw / 2.f, top + 2 * (bh + gap), bw, 50.f,
            Button::NEUTRAL);
    }

    void handleEvent(const sf::Event& e) override
    {
        if (btnCustomer.handleEvent(e)) ctx->go(ScreenID::AUTH_CUSTOMER);
        if (btnAdmin.handleEvent(e))    ctx->go(ScreenID::AUTH_ADMIN);
        if (btnExit.handleEvent(e))     ctx->go(ScreenID::EXIT);
    }

    void draw(sf::RenderWindow& w) override
    {
        float cx = Theme::WIN_W / 2.f;

        sf::RectangleShape shadow;
        shadow.setSize(sf::Vector2f(PAN_W + 4.f, PAN_H + 4.f));
        shadow.setPosition(cx - PAN_W / 2.f + 4.f, PAN_Y + 4.f);
        shadow.setFillColor(sf::Color(0, 0, 0, 90));
        w.draw(shadow);

        sf::RectangleShape panel;
        panel.setSize(sf::Vector2f(PAN_W, PAN_H));
        panel.setPosition(cx - PAN_W / 2.f, PAN_Y);
        panel.setFillColor(Theme::PANEL_ALPHA);
        panel.setOutlineColor(Theme::DIVIDER);
        panel.setOutlineThickness(1.f);
        w.draw(panel);

        sf::RectangleShape bar;
        bar.setSize(sf::Vector2f(5.f, PAN_H));
        bar.setPosition(cx - PAN_W / 2.f, PAN_Y);
        bar.setFillColor(Theme::ACCENT);
        w.draw(bar);

        titleLbl.draw(w);
        btnCustomer.draw(w);
        btnAdmin.draw(w);
        btnExit.draw(w);
    }
};

Screen* makeLauncher() { return new ScreenLauncher(); }
