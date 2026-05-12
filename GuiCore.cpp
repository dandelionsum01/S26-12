#include "GuiCore.h"
#include "NetworkBackground.h"

const sf::Color Theme::BG = sf::Color(12, 18, 40);
const sf::Color Theme::PANEL = sf::Color(20, 30, 62);
const sf::Color Theme::PANEL_ALPHA = sf::Color(6, 8, 18, 180);
const sf::Color Theme::HEADER = sf::Color(7, 12, 28);
const sf::Color Theme::FOOTER = sf::Color(7, 12, 28);
const sf::Color Theme::ACCENT = sf::Color(155, 30, 55);

const sf::Color Theme::TEXT_DARK = sf::Color(220, 228, 245);
const sf::Color Theme::TEXT_LIGHT = sf::Color(255, 255, 255);
const sf::Color Theme::TEXT_MUTED = sf::Color(120, 140, 175);
const sf::Color Theme::TEXT_LINK = sf::Color(100, 165, 255);

const sf::Color Theme::BTN_PRIMARY = sf::Color(65, 105, 200);
const sf::Color Theme::BTN_PRIMARY_HOVER = sf::Color(90, 140, 240);
const sf::Color Theme::BTN_DANGER = sf::Color(155, 30, 55);
const sf::Color Theme::BTN_DANGER_HOVER = sf::Color(190, 50, 75);
const sf::Color Theme::BTN_SUCCESS = sf::Color(25, 135, 85);
const sf::Color Theme::BTN_SUCCESS_HOVER = sf::Color(40, 168, 110);
const sf::Color Theme::BTN_NEUTRAL = sf::Color(50, 62, 98);
const sf::Color Theme::BTN_NEUTRAL_HOVER = sf::Color(72, 88, 130);
const sf::Color Theme::BTN_PURPLE = sf::Color(105, 55, 185);
const sf::Color Theme::BTN_PURPLE_HOVER = sf::Color(130, 80, 220);
const sf::Color Theme::BTN_WARN = sf::Color(165, 110, 15);
const sf::Color Theme::BTN_WARN_HOVER = sf::Color(200, 140, 28);
const sf::Color Theme::FIELD_BG = sf::Color(14, 22, 48);
const sf::Color Theme::FIELD_BORDER = sf::Color(48, 65, 100);
const sf::Color Theme::FIELD_BORDER_FOCUS = sf::Color(95, 148, 255);
const sf::Color Theme::SHADOW = sf::Color(0, 0, 0, 110);
const sf::Color Theme::DIVIDER = sf::Color(32, 48, 80);

void AppContext::go(ScreenID id)
{
    if (manager)
        manager->go(id);
}

void AppContext::back()
{
    if (manager)
        manager->back();
}

void AppContext::clearSession()
{
    username.clear();
    role.clear();
    selectedPackageID.clear();
    selectedStart.clear();
    selectedDestination.clear();
    selectedPrice = 0;
    selectedHotel.clear();
    selectedCapacity = 0;
    selectedDepartureDate.clear();
    draftFromCity.clear();
    draftToCity.clear();
    draftDepartureDate.clear();
}

ScreenManager::ScreenManager()
{
    for (int i = 0; i < 40; ++i)
        factories[i] = nullptr;
    ctx.manager = this;
    netBg = new NetworkBackground();
}

ScreenManager::~ScreenManager()
{
    if (current)
    {
        current->onExit();
        delete current;
    }
    delete netBg;
}

bool ScreenManager::init()
{
    fontOK = font.loadFromFile("arial.ttf");
    if (!fontOK)
    {
        cout << "[GuiCore] FATAL: arial.ttf not found in working directory.\n";
        cout << "          Place arial.ttf next to the executable (or in\n";
        cout << "          your Visual Studio project folder when running\n";
        cout << "          via F5).\n";
        return false;
    }

    ctx.font = &font;

    window.create(sf::VideoMode(Theme::WIN_W, Theme::WIN_H),
        "Travel Management System");
    window.setFramerateLimit(60);

    netBg->init();
    return true;
}

void ScreenManager::registerScreen(ScreenID id, ScreenFactory f)
{
    int idx = static_cast<int>(id);
    if (idx >= 0 && idx < 40)
        factories[idx] = f;
}

void ScreenManager::go(ScreenID id)
{
    if (currentID != ScreenID::NONE && backStackSize < 32)
        backStack[backStackSize++] = currentID;
    navPending = true;
    navTarget = id;
}

void ScreenManager::back()
{
    if (backStackSize > 0)
    {
        navPending = true;
        navTarget = backStack[--backStackSize];
    }
    else
    {
        navPending = true;
        navTarget = ScreenID::LAUNCHER;
    }
}

void ScreenManager::doNav()
{
    if (!navPending)
        return;

    if (navTarget == ScreenID::EXIT)
    {
        window.close();
        navPending = false;
        return;
    }

    int idx = static_cast<int>(navTarget);
    if (idx < 0 || idx >= 40 || factories[idx] == nullptr)
    {
        cout << "[GuiCore] WARNING: ScreenID " << idx
            << " not registered. Going to launcher.\n";
        navTarget = ScreenID::LAUNCHER;
        idx = static_cast<int>(navTarget);
        if (factories[idx] == nullptr)
        {
            cout << "[GuiCore] FATAL: launcher not registered.\n";
            window.close();
            navPending = false;
            return;
        }
    }

    if (current)
    {
        current->onExit();
        delete current;
        current = nullptr;
    }

    currentID = navTarget;
    current = factories[idx]();
    if (current)
        current->attach(&ctx);

    navPending = false;
}

void ScreenManager::drawChrome(const string& screenTitle)
{
    // Header bar
    sf::RectangleShape headerBg;
    headerBg.setSize(sf::Vector2f((float)Theme::WIN_W, (float)Theme::HEADER_H));
    headerBg.setFillColor(Theme::HEADER);
    window.draw(headerBg);

    sf::RectangleShape stripe;
    stripe.setSize(sf::Vector2f((float)Theme::WIN_W, 6.f));
    stripe.setPosition(0.f, (float)Theme::HEADER_H);
    stripe.setFillColor(Theme::ACCENT);
    window.draw(stripe);

    sf::Text appTitle("Travel Management System", font, 22);
    appTitle.setStyle(sf::Text::Bold);
    appTitle.setFillColor(Theme::TEXT_LIGHT);
    appTitle.setPosition(20.f, 20.f);
    window.draw(appTitle);

    if (!screenTitle.empty())
    {
        sf::Text scrTitle(screenTitle, font, 16);
        scrTitle.setFillColor(sf::Color(190, 210, 240));
        scrTitle.setStyle(sf::Text::Italic);
        sf::FloatRect b = scrTitle.getLocalBounds();
        scrTitle.setPosition(Theme::WIN_W - b.width - 25.f, 28.f);
        window.draw(scrTitle);
    }

    if (!ctx.username.empty())
    {
        sf::Text sess("Signed in: " + ctx.username + " (" + ctx.role + ")",
            font, 11);
        sess.setFillColor(sf::Color(170, 190, 220));
        sess.setPosition(22.f, 48.f);
        window.draw(sess);
    }

    sf::RectangleShape footBg;
    footBg.setSize(sf::Vector2f((float)Theme::WIN_W, (float)Theme::FOOTER_H));
    footBg.setPosition(0.f, (float)(Theme::WIN_H - Theme::FOOTER_H));
    footBg.setFillColor(Theme::FOOTER);
    window.draw(footBg);

    sf::Text foot("S26-12  /  C++ OOP Project  /  SFML 2.x", font, 12);
    foot.setFillColor(sf::Color(180, 200, 230));
    foot.setPosition(15.f, (float)(Theme::WIN_H - Theme::FOOTER_H + 12));
    window.draw(foot);
}

void ScreenManager::run(ScreenID startScreen)
{
    if (!fontOK)
    {
        cout << "[GuiCore] cannot run: init() failed.\n";
        return;
    }

    go(startScreen);
    if (backStackSize > 0)
        backStackSize = 0;
    doNav();

    sf::Clock clock;

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();

        sf::Event ev;
        while (window.pollEvent(ev))
        {
            if (ev.type == sf::Event::Closed)
            {
                window.close();
                continue;
            }
            if (current)
                current->handleEvent(ev);
        }

        netBg->update(dt);
        if (current)
            current->update(dt);

        if (navPending)
            doNav();

        window.clear(Theme::BG);
        netBg->draw(window);
        if (current)
            current->draw(window);
        drawChrome(current ? current->title() : "");
        window.display();
    }
}