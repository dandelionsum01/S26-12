
#pragma once
#include "header.h"
#include <algorithm>

class NetworkBackground;

enum class ScreenID
{
    NONE,
    LAUNCHER,
    AUTH_CUSTOMER,
    AUTH_ADMIN,
    CUST_HOME,
    ADMIN_HOME,
    PACKAGES,
    BOOKING,
    CUSTOM_PACKAGE,
    REVIEWS,
    NOTIFICATIONS,
    MY_BOOKINGS,
    ADD_PACKAGE,
    ADD_CITY,
    CUSTOM_APPROVAL,
    REVENUE,
    DEMO,
    EXIT
};

struct Theme
{
    static const sf::Color BG;
    static const sf::Color PANEL;
    static const sf::Color PANEL_ALPHA;
    static const sf::Color HEADER;
    static const sf::Color FOOTER;
    static const sf::Color ACCENT;

    static const sf::Color TEXT_DARK;
    static const sf::Color TEXT_LIGHT;
    static const sf::Color TEXT_MUTED;
    static const sf::Color TEXT_LINK;

    static const sf::Color BTN_PRIMARY;
    static const sf::Color BTN_PRIMARY_HOVER;
    static const sf::Color BTN_DANGER;
    static const sf::Color BTN_DANGER_HOVER;
    static const sf::Color BTN_SUCCESS;
    static const sf::Color BTN_SUCCESS_HOVER;
    static const sf::Color BTN_NEUTRAL;
    static const sf::Color BTN_NEUTRAL_HOVER;
    static const sf::Color BTN_PURPLE;
    static const sf::Color BTN_PURPLE_HOVER;
    static const sf::Color BTN_WARN;
    static const sf::Color BTN_WARN_HOVER;

    static const sf::Color FIELD_BG;
    static const sf::Color FIELD_BORDER;
    static const sf::Color FIELD_BORDER_FOCUS;

    static const sf::Color SHADOW;
    static const sf::Color DIVIDER;

    static constexpr int WIN_W = 1100;
    static constexpr int WIN_H = 720;
    static constexpr int HEADER_H = 70;
    static constexpr int FOOTER_H = 40;
};

class ScreenManager;

struct AppContext
{
    ScreenManager* manager = nullptr;
    sf::Font* font = nullptr;

    string username;
    string role;

    string selectedPackageID;
    string selectedStart;
    string selectedDestination;
    int selectedPrice = 0;
    string selectedHotel;
    int selectedCapacity = 0;
    string selectedDepartureDate;

    string draftFromCity;
    string draftToCity;
    string draftDepartureDate;

    void go(ScreenID id);
    void back();
    void clearSession();
};

class Screen
{
protected:
    AppContext* ctx = nullptr;

public:
    virtual ~Screen() = default;

    void attach(AppContext* c)
    {
        ctx = c;
        onEnter();
    }

    virtual void onEnter() {}
    virtual void onExit() {}
    virtual void handleEvent(const sf::Event& e) {}
    virtual void update(float dt) {}
    virtual void draw(sf::RenderWindow& win) = 0;

    virtual string title() const { return ""; }
};

class ScreenManager
{
    sf::RenderWindow window;
    sf::Font font;
    bool fontOK = false;

    AppContext ctx;

    using ScreenFactory = Screen * (*)();
    ScreenFactory factories[40];
    Screen* current = nullptr;
    ScreenID currentID = ScreenID::NONE;

    NetworkBackground* netBg = nullptr;

    bool navPending = false;
    ScreenID navTarget = ScreenID::NONE;

    ScreenID backStack[32];
    int backStackSize = 0;

    void doNav();
    void drawChrome(const string& screenTitle);

public:
    ScreenManager();
    ~ScreenManager();

    bool init();

    void registerScreen(ScreenID id, ScreenFactory f);

    void go(ScreenID id);
    void back();

    void run(ScreenID startScreen);

    sf::Font* getFont() { return &font; }
    AppContext* getCtx() { return &ctx; }
    sf::RenderWindow& getWindow() { return window; }
};
