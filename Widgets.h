#pragma once
#include "GuiCore.h"

class Label
{
    string text;
    sf::Vector2f pos;
    int fontSize = 14;
    sf::Color color = Theme::TEXT_DARK;
    sf::Uint32 style = sf::Text::Regular;
    sf::Font* font = nullptr;

public:
    Label() = default;
    Label(sf::Font* f, const string& t, float x, float y, int sz = 14)
        : text(t), pos(x, y), fontSize(sz), font(f) {
    }

    void setText(const string& t)
    {
        text = t;
    }
    void setPos(float x, float y)
    {
        pos = sf::Vector2f(x, y);
    }
    void setColor(sf::Color c)
    {
        color = c;
    }
    void setStyle(sf::Uint32 s)
    {
        style = s;
    }
    void setSize(int sz)
    {
        fontSize = sz;
    }
    void setFont(sf::Font* f)
    {
        font = f;
    }

    string getText() const
    {
        return text;
    }

    void draw(sf::RenderWindow& w);
    sf::FloatRect bounds() const;
};

class Button
{
public:
    enum Variant
    {
        PRIMARY,
        DANGER,
        SUCCESS,
        NEUTRAL,
        PURPLE,
        WARN
    };

private:
    sf::FloatRect rect;
    string label;
    string subtitle;
    Variant variant = PRIMARY;
    sf::Font* font = nullptr;
    bool hovered = false;
    bool pressed = false;
    bool enabled = true;

public:
    Button() = default;
    Button(sf::Font* f, const string& lbl, float x, float y,
        float w = 200.f, float h = 40.f, Variant v = PRIMARY)
        : rect(x, y, w, h), label(lbl), variant(v), font(f) {
    }

    void setLabel(const string& l)
    {
        label = l;
    }
    void setSubtitle(const string& s)
    {
        subtitle = s;
    }
    void setVariant(Variant v)
    {
        variant = v;
    }
    void setRect(float x, float y, float w, float h)
    {
        rect = sf::FloatRect(x, y, w, h);
    }
    void setEnabled(bool e)
    {
        enabled = e;
    }
    void setFont(sf::Font* f)
    {
        font = f;
    }

    sf::FloatRect getRect() const
    {
        return rect;
    }

    bool handleEvent(const sf::Event& e);

    void draw(sf::RenderWindow& w);
};

class TextField
{
    sf::FloatRect rect;
    string text;
    string placeholder;
    bool focused = false;
    bool password = false;
    bool numeric = false;
    size_t maxLen = 64;
    sf::Font* font = nullptr;

    float cursorBlink = 0.f;

public:
    TextField() = default;
    TextField(sf::Font* f, float x, float y, float w = 280.f, float h = 36.f)
        : rect(x, y, w, h), font(f) {
    }

    void setRect(float x, float y, float w, float h)
    {
        rect = sf::FloatRect(x, y, w, h);
    }
    void setPlaceholder(const string& p)
    {
        placeholder = p;
    }
    void setPassword(bool p)
    {
        password = p;
    }
    void setNumeric(bool n)
    {
        numeric = n;
    }
    void setMaxLen(size_t m)
    {
        maxLen = m;
    }
    void setText(const string& t)
    {
        text = t;
    }
    void setFont(sf::Font* f)
    {
        font = f;
    }
    void setFocused(bool f)
    {
        focused = f;
    }

    string getText() const

    {
        return text;
    }
    bool isFocused() const
    {
        return focused;
    }
    sf::FloatRect getRect() const
    {
        return rect;
    }

    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw(sf::RenderWindow& w);
};

class Dialog
{
public:
    enum Result
    {
        NONE,
        OK,
        CANCEL
    };
    enum Kind
    {
        INFO,
        CONFIRM,
        ERROR_KIND,
        SUCCESS
    };

private:
    bool open = false;
    Kind kind = INFO;
    string title;
    string message;
    Result lastResult = NONE;
    sf::Font* font = nullptr;

    Button btnOK;
    Button btnCancel;

public:
    Dialog() = default;
    void setFont(sf::Font* f)
    {
        font = f;
    }

    void show(const string& title, const string& message,

        Kind k = INFO, bool withCancel = false);
    void close()
    {
        open = false;
    }

    bool isOpen() const
    {
        return open;
    }
    Result result();

    void handleEvent(const sf::Event& e);
    void draw(sf::RenderWindow& w);
};

class Calendar
{
    bool open = false;
    int viewYear = 2026;
    int viewMonth = 1;
    int selectedYear = 0;
    int selectedMonth = 0;
    int selectedDay = 0;
    string selectedISO;
    bool hasResult = false;

    sf::Font* font = nullptr;
    sf::Vector2f pos;
    float width = 320.f;

    Button btnPrev;
    Button btnNext;
    Button btnClose;

    static int daysInMonth(int year, int month);
    static int weekdayOfFirst(int year, int month);
    void rebuildButtons();

public:
    Calendar() = default;
    void setFont(sf::Font* f)
    {
        font = f;
    }
    void setPos(float x, float y)
    {
        pos = sf::Vector2f(x, y);
        rebuildButtons();
    }

    void openAt(int year, int month);
    void close()
    {
        open = false;
    }
    bool isOpen() const
    {
        return open;
    }

    bool hasSelection() const
    {
        return hasResult;
    }
    string getSelected() const
    {
        return selectedISO;
    }
    void clearResult()
    {
        hasResult = false;
        selectedISO.clear();
    }

    void handleEvent(const sf::Event& e);
    void draw(sf::RenderWindow& w);
};

class Table
{
    sf::FloatRect rect;
    vector<string> headers;
    vector<vector<string>> rows;
    vector<float> colWidths; // computed
    sf::Font* font = nullptr;

    int selectedIndex = -1;
    int hoverIndex = -1;
    float scrollY = 0.f;
    float rowHeight = 36.f;
    float headerHeight = 42.f;

    void recomputeColumnWidths();

public:
    Table() = default;
    Table(sf::Font* f, float x, float y, float w, float h)
        : rect(x, y, w, h), font(f) {
    }

    void setRect(float x, float y, float w, float h)
    {
        rect = sf::FloatRect(x, y, w, h);
    }
    void setFont(sf::Font* f)
    {
        font = f;
    }

    void setHeaders(const vector<string>& h);
    void setRows(const vector<vector<string>>& r);
    void clear()
    {
        rows.clear();
        selectedIndex = -1;
        scrollY = 0.f;
    }

    int getSelectedIndex() const
    {
        return selectedIndex;
    }
    int rowCount() const
    {
        return (int)rows.size();
    }
    vector<string> getRow(int i) const
    {
        if (i < 0 || i >= (int)rows.size())
            return {};
        return rows[i];
    }

    void handleEvent(const sf::Event& e);
    void draw(sf::RenderWindow& w);
};
