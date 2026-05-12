#include "GuiCore.h"
#include "Widgets.h"

struct PasswordLockout
{
    int wrongThisRound = 0;
    int lockoutSet = 0;
    bool locked = false;
    int secondsTotal = 0;
    int secondsLeft = 0;
    sf::Clock clock;

    bool recordWrong()
    {
        wrongThisRound++;
        if (wrongThisRound >= 3)
        {
            secondsTotal = 30 * static_cast<int>(pow(2.0, lockoutSet));
            secondsLeft = secondsTotal;
            locked = true;
            lockoutSet++;
            wrongThisRound = 0;
            clock.restart();
            return true;
        }
        return false;
    }

    bool update()
    {
        if (!locked)
            return false;
        if (clock.getElapsedTime().asSeconds() >= 1.f)
        {
            clock.restart();
            secondsLeft--;
            if (secondsLeft <= 0)
            {
                locked = false;
                secondsLeft = 0;
            }
        }
        return locked;
    }

    void reset()
    {
        wrongThisRound = 0;
        lockoutSet = 0;
        locked = false;
        secondsTotal = 0;
        secondsLeft = 0;
    }

    void drawBadge(sf::RenderWindow& w, sf::Font& font)
    {
        if (!locked)
            return;

        float bw = 260.f, bh = 72.f;
        float bx = Theme::WIN_W - bw - 20.f;
        float by = Theme::WIN_H - Theme::FOOTER_H - bh - 14.f;

        sf::RectangleShape shadow;
        shadow.setSize(sf::Vector2f(bw, bh));
        shadow.setPosition(bx + 3.f, by + 4.f);
        shadow.setFillColor(sf::Color(0, 0, 0, 60));
        w.draw(shadow);

        sf::RectangleShape bg;
        bg.setSize(sf::Vector2f(bw, bh));
        bg.setPosition(bx, by);
        bg.setFillColor(sf::Color(50, 20, 20));
        bg.setOutlineColor(sf::Color(200, 80, 50));
        bg.setOutlineThickness(2.f);
        w.draw(bg);

        sf::RectangleShape stripe;
        stripe.setSize(sf::Vector2f(5.f, bh));
        stripe.setPosition(bx, by);
        stripe.setFillColor(sf::Color(200, 80, 50));
        w.draw(stripe);

        sf::Text label("Too many attempts", font, 12);
        label.setFillColor(sf::Color(220, 180, 180));
        label.setPosition(bx + 14.f, by + 8.f);
        w.draw(label);

        string sec = to_string(secondsLeft) + "s";
        sf::Text cd(sec, font, 28);
        cd.setStyle(sf::Text::Bold);
        cd.setFillColor(sf::Color(230, 90, 60));
        cd.setPosition(bx + 14.f, by + 26.f);
        w.draw(cd);

        string round = "Lockout " + to_string(lockoutSet) +
            "  (" + to_string(secondsTotal) + "s)";
        sf::Text roundLbl(round, font, 11);
        roundLbl.setFillColor(sf::Color(180, 140, 140));
        roundLbl.setStyle(sf::Text::Italic);

        sf::FloatRect cdb = cd.getLocalBounds();
        roundLbl.setPosition(bx + 14.f + cdb.width + 10.f, by + 36.f);
        w.draw(roundLbl);

        float barW = bw - 14.f;
        float frac = (secondsTotal > 0)
            ? (float)secondsLeft / (float)secondsTotal
            : 0.f;
        sf::RectangleShape barBg;
        barBg.setSize(sf::Vector2f(barW, 5.f));
        barBg.setPosition(bx + 7.f, by + bh - 9.f);
        barBg.setFillColor(sf::Color(80, 40, 40));
        w.draw(barBg);

        sf::RectangleShape barFg;
        barFg.setSize(sf::Vector2f(barW * frac, 5.f));
        barFg.setPosition(bx + 7.f, by + bh - 9.f);
        barFg.setFillColor(sf::Color(200, 80, 50));
        w.draw(barFg);
    }
};

class CaptchaWidget
{
public:
    enum State
    {
        HIDDEN,
        ACTIVE,
        CLOCKED,
        PASSED,
        FAILED
    };

private:
    static constexpr int N = 5;
    const char* imgs[N] = {
        "reCaptcha1.png", "reCaptcha2.png", "reCaptcha3.png",
        "reCaptcha4.png", "reCaptcha5.png" };
    const char* ans[N] = {
        "A7kP9", "ALliteRaTIoN", "hou-se", "b4ckgr0uzd", "JkL5Np" };

    State state = HIDDEN;
    int idx = 0;
    string input;
    sf::Texture tex;
    sf::Font* font = nullptr;
    int wrongR = 0, lockSet = 0;
    int lockSec = 0, lockLeft = 0;
    sf::Clock lockClk;

    static constexpr float W = 480.f, H = 430.f;
    float ox = 0.f, oy = 0.f;
    sf::FloatRect subRect;

    void loadImg()
    {
        idx = rand() % N;
        tex = sf::Texture();
        if (!tex.loadFromFile(imgs[idx]))
            state = PASSED;
    }
    void startLock()
    {
        lockSec = 30 * (int)pow(2.0, lockSet);
        lockLeft = lockSec;
        lockSet++;
        wrongR = 0;
        state = CLOCKED;
        lockClk.restart();
    }
    void layout()
    {
        ox = (Theme::WIN_W - W) / 2.f;
        oy = (Theme::WIN_H - H) / 2.f;
        subRect = sf::FloatRect(ox + (W - 130.f) / 2.f, oy + H - 62.f, 130.f, 40.f);
    }

public:
    void setFont(sf::Font* f) { font = f; }
    void open()
    {
        srand((unsigned)time(nullptr));
        layout();
        input.clear();
        wrongR = 0;
        lockSet = 0;
        state = ACTIVE;
        loadImg();
    }
    State getState() const { return state; }
    bool isVisible() const { return state == ACTIVE || state == CLOCKED; }
    void reset()
    {
        state = HIDDEN;
        input.clear();
        wrongR = 0;
        lockSet = 0;
    }

    void update(float)
    {
        if (state != CLOCKED)
            return;
        if (lockClk.getElapsedTime().asSeconds() >= 1.f)
        {
            lockClk.restart();
            lockLeft--;
            if (lockLeft <= 0)
            {
                input.clear();
                state = ACTIVE;
                loadImg();
            }
        }
    }

    void handleEvent(const sf::Event& e)
    {
        if (state == CLOCKED)
        {
            if (e.type == sf::Event::KeyPressed &&
                e.key.code == sf::Keyboard::Escape)
                state = FAILED;
            return;
        }
        if (state != ACTIVE)
            return;
        if (e.type == sf::Event::KeyPressed &&
            e.key.code == sf::Keyboard::Escape)
        {
            state = FAILED;
            return;
        }
        if (e.type == sf::Event::TextEntered)
        {
            sf::Uint32 u = e.text.unicode;
            if (u == 8 && !input.empty())
                input.pop_back();
            else if (u == 13)
                trySubmit();
            else if (u >= 32 && u < 127)
                input += (char)u;
        }
        if (e.type == sf::Event::MouseButtonReleased &&
            e.mouseButton.button == sf::Mouse::Left &&
            subRect.contains(sf::Vector2f((float)e.mouseButton.x,
                (float)e.mouseButton.y)))
            trySubmit();
    }

    void trySubmit()
    {
        if (input == string(ans[idx]))
        {
            state = PASSED;
            return;
        }
        wrongR++;
        input.clear();
        if (wrongR >= 3)
            startLock();
        else
        {
            loadImg();
            if (state != PASSED)
                state = ACTIVE;
        }
    }

    void draw(sf::RenderWindow& w)
    {
        if (!isVisible() || !font)
            return;
        sf::RectangleShape dim;
        dim.setSize(sf::Vector2f((float)Theme::WIN_W, (float)Theme::WIN_H));
        dim.setFillColor(sf::Color(0, 0, 0, 160));
        w.draw(dim);
        sf::RectangleShape panel;
        panel.setSize(sf::Vector2f(W, H));
        panel.setPosition(ox, oy);
        panel.setFillColor(Theme::PANEL_ALPHA);
        panel.setOutlineColor(state == CLOCKED ? Theme::BTN_DANGER : Theme::HEADER);
        panel.setOutlineThickness(2.f);
        w.draw(panel);
        sf::RectangleShape s2;
        s2.setSize(sf::Vector2f(W, 5.f));
        s2.setPosition(ox, oy);
        s2.setFillColor(state == CLOCKED ? Theme::BTN_DANGER : Theme::ACCENT);
        w.draw(s2);
        if (state == CLOCKED)
            drawClock(w);
        else
            drawForm(w);
    }

private:
    void drawForm(sf::RenderWindow& w)
    {
        sf::Text t("Security Verification", *font, 20);
        t.setStyle(sf::Text::Bold);
        t.setFillColor(Theme::TEXT_DARK);
        t.setPosition(ox + 18.f, oy + 16.f);
        w.draw(t);
        string a = "Attempt " + to_string(wrongR + 1) + " of 3";
        sf::Text al(a, *font, 11);
        al.setFillColor(wrongR > 0 ? sf::Color(200, 80, 50) : Theme::TEXT_MUTED);
        sf::FloatRect ab = al.getLocalBounds();
        al.setPosition(ox + W - ab.width - 18.f, oy + 20.f);
        w.draw(al);
        sf::Text sub("Type the text shown in the image below", *font, 12);
        sub.setFillColor(Theme::TEXT_MUTED);
        sub.setPosition(ox + 18.f, oy + 44.f);
        w.draw(sub);
        sf::Sprite sp(tex);
        float iw = (float)tex.getSize().x, ih = (float)tex.getSize().y;
        float sc = min((W - 36.f) / iw, 180.f / ih);
        sp.setScale(sc, sc);
        sp.setPosition(ox + (W - iw * sc) / 2.f, oy + 68.f);
        w.draw(sp);
        sf::Text lbl("Your answer:", *font, 13);
        lbl.setFillColor(Theme::TEXT_DARK);
        lbl.setPosition(ox + 18.f, oy + 264.f);
        w.draw(lbl);
        sf::RectangleShape f2;
        f2.setSize(sf::Vector2f(W - 36.f, 38.f));
        f2.setPosition(ox + 18.f, oy + 284.f);
        f2.setFillColor(Theme::FIELD_BG);
        f2.setOutlineColor(Theme::FIELD_BORDER_FOCUS);
        f2.setOutlineThickness(2.f);
        w.draw(f2);
        sf::Text inp(input, *font, 16);
        inp.setFillColor(Theme::TEXT_DARK);
        inp.setPosition(ox + 28.f, oy + 293.f);
        w.draw(inp);
        sf::Vector2i mp = sf::Mouse::getPosition(w);
        bool hov = subRect.contains(sf::Vector2f((float)mp.x, (float)mp.y));
        sf::RectangleShape btn;
        btn.setSize(sf::Vector2f(subRect.width, subRect.height));
        btn.setPosition(subRect.left, subRect.top);
        btn.setFillColor(hov ? Theme::BTN_PRIMARY_HOVER : Theme::BTN_PRIMARY);
        w.draw(btn);
        sf::Text bt("Verify", *font, 15);
        bt.setStyle(sf::Text::Bold);
        bt.setFillColor(Theme::TEXT_LIGHT);
        sf::FloatRect bb = bt.getLocalBounds();
        bt.setPosition(subRect.left + (subRect.width - bb.width) / 2.f,
            subRect.top + (subRect.height - bb.height) / 2.f - 3.f);
        w.draw(bt);
        sf::Text h2("Press Enter or click Verify  |  ESC to cancel", *font, 11);
        h2.setFillColor(Theme::TEXT_MUTED);
        h2.setStyle(sf::Text::Italic);
        sf::FloatRect hb = h2.getLocalBounds();
        h2.setPosition(ox + (W - hb.width) / 2.f, oy + H - 20.f);
        w.draw(h2);
    }

    void drawClock(sf::RenderWindow& w)
    {
        sf::Text t("Captcha Locked", *font, 20);
        t.setStyle(sf::Text::Bold);
        t.setFillColor(Theme::TEXT_DARK);
        sf::FloatRect tb = t.getLocalBounds();
        t.setPosition(ox + (W - tb.width) / 2.f, oy + 16.f);
        w.draw(t);
        sf::Text cd(to_string(lockLeft), *font, 72);
        cd.setStyle(sf::Text::Bold);
        cd.setFillColor(sf::Color(200, 80, 50));
        sf::FloatRect cb = cd.getLocalBounds();
        cd.setPosition(ox + (W - cb.width) / 2.f - cb.left, oy + 200.f);
        w.draw(cd);
        sf::Text sl("seconds", *font, 16);
        sl.setFillColor(Theme::TEXT_MUTED);
        sf::FloatRect slb = sl.getLocalBounds();
        sl.setPosition(ox + (W - slb.width) / 2.f, oy + 290.f);
        w.draw(sl);
        float bw = W - 80.f, fr = (float)lockLeft / (float)(lockSec > 0 ? lockSec : 1);
        sf::RectangleShape bg2;
        bg2.setSize(sf::Vector2f(bw, 10.f));
        bg2.setPosition(ox + 40.f, oy + 330.f);
        bg2.setFillColor(sf::Color(220, 225, 235));
        w.draw(bg2);
        sf::RectangleShape fg;
        fg.setSize(sf::Vector2f(bw * fr, 10.f));
        fg.setPosition(ox + 40.f, oy + 330.f);
        fg.setFillColor(sf::Color(200, 80, 50));
        w.draw(fg);
    }
};

class ScreenAuthCustomer : public Screen
{
    enum Mode
    {
        SIGNIN,
        SIGNUP
    };
    Mode mode = SIGNIN;

    Label titleLbl;
    Button tabSignIn, tabSignUp, btnBack;

    Label siUserLbl, siPwLbl, siHintLbl;
    TextField siUserField, siPwField;
    Button btnSignIn;

    Label suUserLbl, suPwLbl, suRulesLbl;
    TextField suUserField, suPwField;
    Button btnSignUp;

    Dialog dialog;
    CaptchaWidget captcha;
    PasswordLockout pwLock;

    string pendingUser;

    static constexpr float FX = 330.f;
    static constexpr float FW = 440.f;
    static constexpr float FT = 200.f;

    void buildLayout()
    {
        sf::Font* f = ctx->font;
        titleLbl = Label(f,
            mode == SIGNIN ? "Customer Sign In" : "Create an Account",
            FX, FT - 55.f, 24);
        titleLbl.setStyle(sf::Text::Bold);

        float tw = FW / 2.f - 3.f;
        tabSignIn = Button(f, "Sign In", FX, FT - 15.f, tw, 34.f,
            mode == SIGNIN ? Button::PRIMARY : Button::NEUTRAL);
        tabSignUp = Button(f, "Sign Up", FX + tw + 6.f, FT - 15.f, tw, 34.f,
            mode == SIGNUP ? Button::PRIMARY : Button::NEUTRAL);
        btnBack = Button(f, "< Back", 28.f,
            Theme::WIN_H - Theme::FOOTER_H - 52.f,
            100.f, 34.f, Button::NEUTRAL);
        dialog.setFont(f);
        captcha.setFont(f);

        if (mode == SIGNIN)
        {
            float y = FT + 38.f;
            siUserLbl = Label(f, "Username", FX, y, 13);
            siUserField = TextField(f, FX, y + 20.f, FW, 38.f);
            siUserField.setPlaceholder("Enter your username");
            siPwLbl = Label(f, "Password", FX, y + 74.f, 13);
            siPwField = TextField(f, FX, y + 94.f, FW, 38.f);
            siPwField.setPlaceholder("Enter your password");
            siPwField.setPassword(true);
            siHintLbl = Label(f, "A reCaptcha will appear to verify your identity.",
                FX, y + 148.f, 11);
            siHintLbl.setColor(Theme::TEXT_MUTED);
            btnSignIn = Button(f, "Sign In", FX, y + 170.f, FW, 44.f, Button::PRIMARY);
            // Keep disabled state if currently locked
            btnSignIn.setEnabled(!pwLock.locked);
        }
        else
        {
            float y = FT + 38.f;
            suUserLbl = Label(f, "Username", FX, y, 13);
            suUserField = TextField(f, FX, y + 20.f, FW, 38.f);
            suUserField.setPlaceholder("Choose a unique username");
            suPwLbl = Label(f, "Password", FX, y + 74.f, 13);
            suPwField = TextField(f, FX, y + 94.f, FW, 38.f);
            suPwField.setPlaceholder("See requirements below");
            suPwField.setPassword(true);
            suRulesLbl = Label(f, "8+ chars, 2+ digits, 1+ uppercase, 1+ lowercase, 1+ special",
                FX, y + 140.f, 11);
            suRulesLbl.setColor(Theme::TEXT_MUTED);
            btnSignUp = Button(f, "Create Account", FX, y + 162.f, FW, 44.f, Button::SUCCESS);
        }
    }

public:
    string title() const override
    {
        return mode == SIGNIN ? "Customer Sign In" : "Customer Sign Up";
    }

    void onEnter() override
    {
        mode = SIGNIN;
        captcha.reset();
        pwLock.reset();
        buildLayout();
    }

    void handleEvent(const sf::Event& e) override
    {
        if (captcha.isVisible())
        {
            captcha.handleEvent(e);
            if (captcha.getState() == CaptchaWidget::PASSED)
            {
                ctx->username = pendingUser;
                ctx->role = "customer";
                ctx->go(ScreenID::CUST_HOME);
            }
            else if (captcha.getState() == CaptchaWidget::FAILED)
            {
                dialog.show("Captcha Cancelled",
                    "Verification cancelled. Please try again.",
                    Dialog::ERROR_KIND);
                captcha.reset();
                siPwField.setText("");
            }
            return;
        }
        if (dialog.isOpen())
        {
            dialog.handleEvent(e);
            return;
        }
        if (pwLock.locked)
            return; // block all input during lockout

        if (tabSignIn.handleEvent(e) && mode != SIGNIN)
        {
            mode = SIGNIN;
            buildLayout();
            return;
        }
        if (tabSignUp.handleEvent(e) && mode != SIGNUP)
        {
            mode = SIGNUP;
            buildLayout();
            return;
        }
        if (btnBack.handleEvent(e))
        {
            ctx->go(ScreenID::LAUNCHER);
            return;
        }

        if (mode == SIGNIN)
        {
            siUserField.handleEvent(e);
            siPwField.handleEvent(e);

            if (btnSignIn.handleEvent(e))
            {
                string user = siUserField.getText();
                string pw = siPwField.getText();
                if (user.empty() || pw.empty())
                {
                    dialog.show("Missing Fields",
                        "Please enter both username and password.",
                        Dialog::ERROR_KIND);
                    return;
                }
                Customer c;
                c.setUsername(user);
                if (!c.findUsername())
                {
                    dialog.show("Not Found",
                        "Username not found.\n"
                        "Check spelling or sign up first.",
                        Dialog::ERROR_KIND);
                    return;
                }
                c.setPassword(pw);
                if (!c.checkPassword())
                {
                    siPwField.setText("");
                    bool triggered = pwLock.recordWrong();
                    if (triggered)
                    {
                        btnSignIn.setEnabled(false);
                    }
                    else
                    {
                        int left = 3 - pwLock.wrongThisRound;
                        dialog.show("Wrong Password",
                            "Incorrect password.\n" +
                            to_string(left) + " attempt" +
                            (left == 1 ? "" : " s") +
                            " remaining before lockout.",
                            Dialog::ERROR_KIND);
                    }
                    return;
                }
                pendingUser = user;
                captcha.open();
            }
        }
        else
        {
            suUserField.handleEvent(e);
            suPwField.handleEvent(e);
            if (btnSignUp.handleEvent(e))
            {
                string user = suUserField.getText();
                string pw = suPwField.getText();
                if (user.empty() || pw.empty())
                {
                    dialog.show("Missing Fields", "Both fields are required.",
                        Dialog::ERROR_KIND);
                    return;
                }
                Customer c;
                c.setUsername(user);
                if (c.findUsername())
                {
                    dialog.show("Username Taken",
                        "That username already exists.\n"
                        "Please choose a different one.",
                        Dialog::ERROR_KIND);
                    return;
                }
                c.setPassword(pw);
                if (!c.checkStrength())
                {
                    dialog.show("Weak Password",
                        "Password must have:\n"
                        "  8+ characters\n  2+ digits\n"
                        "  1+ uppercase\n  1+ lowercase\n"
                        "  1+ special char\n  No common words",
                        Dialog::ERROR_KIND);
                    return;
                }
                bool empty = false;
                {
                    ifstream chk("CustomerInfo.csv");
                    if (!chk.is_open())
                        empty = true;
                    else
                    {
                        chk.seekg(0, ios::end);
                        empty = (chk.tellg() == 0);
                    }
                }
                ofstream out("CustomerInfo.csv", ios::app);
                if (!out.is_open())
                {
                    dialog.show("File Error", "Cannot open CustomerInfo.csv.",
                        Dialog::ERROR_KIND);
                    return;
                }
                if (empty)
                    out << "username,password\n";
                out << user << "," << pw << "\n";
                out.close();
                dialog.show("Account Created!",
                    "Welcome, " + user + "!\n"
                    "Your account has been created.\n"
                    "Switch to Sign In to continue.",
                    Dialog::SUCCESS);
                mode = SIGNIN;
                buildLayout();
                siUserField.setText(user);
            }
        }
    }

    void update(float dt) override
    {
        captcha.update(dt);
        bool wasLocked = pwLock.locked;
        pwLock.update();
        if (wasLocked && !pwLock.locked)
            btnSignIn.setEnabled(true); // timer expired -- re-enable
        if (mode == SIGNIN)
        {
            siUserField.update(dt);
            siPwField.update(dt);
        }
        else
        {
            suUserField.update(dt);
            suPwField.update(dt);
        }
    }

    void draw(sf::RenderWindow& w) override
    {
        sf::RectangleShape shadow;
        shadow.setSize(sf::Vector2f(FW + 64.f, 384.f));
        shadow.setPosition(FX - 28.f, FT - 71.f);
        shadow.setFillColor(sf::Color(0, 0, 0, 90));
        w.draw(shadow);

        sf::RectangleShape panel;
        panel.setSize(sf::Vector2f(FW + 60.f, 380.f));
        panel.setPosition(FX - 30.f, FT - 75.f);
        panel.setFillColor(Theme::PANEL_ALPHA);
        panel.setOutlineColor(Theme::DIVIDER);
        panel.setOutlineThickness(1.f);
        w.draw(panel);
        sf::RectangleShape accent;
        accent.setSize(sf::Vector2f(5.f, 380.f));
        accent.setPosition(FX - 30.f, FT - 75.f);
        accent.setFillColor(Theme::ACCENT);
        w.draw(accent);

        titleLbl.draw(w);
        tabSignIn.draw(w);
        tabSignUp.draw(w);
        btnBack.draw(w);

        if (mode == SIGNIN)
        {
            siUserLbl.draw(w);
            siUserField.draw(w);
            siPwLbl.draw(w);
            siPwField.draw(w);
            siHintLbl.draw(w);
            btnSignIn.draw(w);
        }
        else
        {
            suUserLbl.draw(w);
            suUserField.draw(w);
            suPwLbl.draw(w);
            suPwField.draw(w);
            suRulesLbl.draw(w);
            btnSignUp.draw(w);
        }

        if (ctx && ctx->font)
            pwLock.drawBadge(w, *ctx->font);
        dialog.draw(w);
        captcha.draw(w);
    }
};

class ScreenAuthAdmin : public Screen
{
    Label titleLbl, subLbl, userLbl, pwLbl;
    TextField userField, pwField;
    Button btnSignIn, btnBack;
    Dialog dialog;
    static constexpr float FX = 330.f, FW = 440.f, FT = 230.f;

public:
    string title() const override { return "Admin Sign In"; }

    void onEnter() override
    {
        sf::Font* f = ctx->font;
        titleLbl = Label(f, "Admin Portal", FX, FT - 55.f, 24);
        titleLbl.setStyle(sf::Text::Bold);
        subLbl = Label(f, "Restricted to administrators.", FX, FT - 25.f, 13);
        subLbl.setColor(Theme::TEXT_MUTED);
        userLbl = Label(f, "Username", FX, FT + 10.f, 13);
        userField = TextField(f, FX, FT + 30.f, FW, 38.f);
        userField.setPlaceholder("Admin username");
        pwLbl = Label(f, "Password", FX, FT + 84.f, 13);
        pwField = TextField(f, FX, FT + 104.f, FW, 38.f);
        pwField.setPlaceholder("Admin password");
        pwField.setPassword(true);
        btnSignIn = Button(f, "Sign In", FX, FT + 164.f, FW, 44.f, Button::DANGER);
        btnBack = Button(f, "< Back", 28.f,
            Theme::WIN_H - Theme::FOOTER_H - 52.f,
            100.f, 34.f, Button::NEUTRAL);
        dialog.setFont(f);
    }

    void handleEvent(const sf::Event& e) override
    {
        if (dialog.isOpen())
        {
            dialog.handleEvent(e);
            return;
        }
        userField.handleEvent(e);
        pwField.handleEvent(e);
        if (btnBack.handleEvent(e))
        {
            ctx->go(ScreenID::LAUNCHER);
            return;
        }
        if (btnSignIn.handleEvent(e))
        {
            string user = userField.getText(), pw = pwField.getText();
            if (user.empty() || pw.empty())
            {
                dialog.show("Missing Fields", "Please enter username and password.",
                    Dialog::ERROR_KIND);
                return;
            }
            Admin a;
            a.setUsername(user);
            if (!a.findUsername())
            {
                dialog.show("Not Found", "Admin username not found.",
                    Dialog::ERROR_KIND);
                return;
            }
            a.setPassword(pw);
            if (!a.checkPassword())
            {
                dialog.show("Wrong Password", "Incorrect password.",
                    Dialog::ERROR_KIND);
                pwField.setText("");
                return;
            }
            ctx->username = user;
            ctx->role = "admin";
            ctx->go(ScreenID::ADMIN_HOME);
        }
    }

    void update(float dt) override
    {
        userField.update(dt);
        pwField.update(dt);
    }

    void draw(sf::RenderWindow& w) override
    {
        sf::RectangleShape shadow;
        shadow.setSize(sf::Vector2f(FW + 64.f, 324.f));
        shadow.setPosition(FX - 28.f, FT - 71.f);
        shadow.setFillColor(sf::Color(0, 0, 0, 90));
        w.draw(shadow);

        sf::RectangleShape panel;
        panel.setSize(sf::Vector2f(FW + 60.f, 320.f));
        panel.setPosition(FX - 30.f, FT - 75.f);
        panel.setFillColor(Theme::PANEL_ALPHA);
        panel.setOutlineColor(Theme::DIVIDER);
        panel.setOutlineThickness(1.f);
        w.draw(panel);
        sf::RectangleShape accent;
        accent.setSize(sf::Vector2f(5.f, 320.f));
        accent.setPosition(FX - 30.f, FT - 75.f);
        accent.setFillColor(Theme::ACCENT);
        w.draw(accent);
        titleLbl.draw(w);
        subLbl.draw(w);
        userLbl.draw(w);
        userField.draw(w);
        pwLbl.draw(w);
        pwField.draw(w);
        btnSignIn.draw(w);
        btnBack.draw(w);
        dialog.draw(w);
    }
};

Screen* makeAuthCustomer() { return new ScreenAuthCustomer(); }
Screen* makeAuthAdmin() { return new ScreenAuthAdmin(); }