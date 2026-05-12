#include "Widgets.h"

static void drawRoundedRect(sf::RenderWindow& w,
    float x, float y, float width, float height,
    float radius,
    sf::Color fill,
    sf::Color outlineColor = sf::Color::Transparent,
    float outlineThick = 0.f)
{
    radius = std::min(radius, std::min(width, height) / 2.f);
    const int pts = 10;
    const float pi = 3.14159265f;
    sf::ConvexShape shape(pts * 4);
    int p = 0;

    const float cx[4] = { x + width - radius, x + width - radius, x + radius, x + radius };
    const float cy[4] = { y + radius, y + height - radius, y + height - radius, y + radius };
    const float sa[4] = { -pi / 2.f, 0.f, pi / 2.f, pi };

    for (int c = 0; c < 4; ++c)
        for (int i = 0; i < pts; ++i)
        {
            float angle = sa[c] + (pi / 2.f) * (float)i / (float)(pts - 1);
            shape.setPoint(p++, sf::Vector2f(
                cx[c] + radius * std::cos(angle),
                cy[c] + radius * std::sin(angle)));
        }

    shape.setFillColor(fill);
    if (outlineThick > 0.f)
    {
        shape.setOutlineColor(outlineColor);
        shape.setOutlineThickness(outlineThick);
    }
    w.draw(shape);
}

static string fitText(const string& text, sf::Font& font, unsigned sz, float maxWidth)
{
    sf::Text t(text, font, sz);
    if (t.getLocalBounds().width <= maxWidth)
        return text;
    string r = text;
    while (!r.empty())
    {
        r.pop_back();
        sf::Text test(r + "...", font, sz);
        if (test.getLocalBounds().width <= maxWidth)
            return r + "...";
    }
    return "...";
}

void Label::draw(sf::RenderWindow& w)
{
    if (!font)
        return;
    sf::Text t(text, *font, fontSize);
    t.setFillColor(color);
    t.setStyle(style);
    t.setPosition(pos);
    w.draw(t);
}

sf::FloatRect Label::bounds() const
{
    if (!font)
        return sf::FloatRect();
    sf::Text t(text, *font, fontSize);
    sf::FloatRect b = t.getLocalBounds();
    b.left += pos.x;
    b.top += pos.y;
    return b;
}

bool Button::handleEvent(const sf::Event& e)
{
    if (!enabled)
        return false;

    if (e.type == sf::Event::MouseMoved)
    {
        sf::Vector2f mp((float)e.mouseMove.x, (float)e.mouseMove.y);
        hovered = rect.contains(mp);
    }
    else if (e.type == sf::Event::MouseButtonPressed &&
        e.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f mp((float)e.mouseButton.x, (float)e.mouseButton.y);
        if (rect.contains(mp))
        {
            pressed = true;
            return false;
        }
    }
    else if (e.type == sf::Event::MouseButtonReleased &&
        e.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f mp((float)e.mouseButton.x, (float)e.mouseButton.y);
        bool wasPressed = pressed;
        pressed = false;
        if (wasPressed && rect.contains(mp))
            return true;
    }
    return false;
}

void Button::draw(sf::RenderWindow& w)
{
    sf::Color base, hover;
    switch (variant)
    {
    case PRIMARY:
        base = Theme::BTN_PRIMARY;
        hover = Theme::BTN_PRIMARY_HOVER;
        break;
    case DANGER:
        base = Theme::BTN_DANGER;
        hover = Theme::BTN_DANGER_HOVER;
        break;
    case SUCCESS:
        base = Theme::BTN_SUCCESS;
        hover = Theme::BTN_SUCCESS_HOVER;
        break;
    case NEUTRAL:
        base = Theme::BTN_NEUTRAL;
        hover = Theme::BTN_NEUTRAL_HOVER;
        break;
    case PURPLE:
        base = Theme::BTN_PURPLE;
        hover = Theme::BTN_PURPLE_HOVER;
        break;
    case WARN:
        base = Theme::BTN_WARN;
        hover = Theme::BTN_WARN_HOVER;
        break;
    }
    if (!enabled)
    {
        base = sf::Color(40, 48, 72);
        hover = base;
    }

    drawRoundedRect(w, rect.left + 2.f, rect.top + 4.f,
        rect.width, rect.height, 10.f, Theme::SHADOW);

    sf::Color outline(255, 255, 255, hovered ? 180 : 45);
    drawRoundedRect(w, rect.left, rect.top, rect.width, rect.height,
        10.f, hovered ? hover : base, outline, hovered ? 2.f : 1.f);

    if (!font)
        return;

    int lblSize = subtitle.empty() ? 17 : 18;
    sf::Text lbl(label, *font, lblSize);
    lbl.setStyle(sf::Text::Bold);
    lbl.setFillColor(enabled ? Theme::TEXT_LIGHT : Theme::TEXT_MUTED);
    sf::FloatRect lb = lbl.getLocalBounds();

    if (subtitle.empty())
    {
        lbl.setPosition(rect.left + (rect.width - lb.width) / 2.f - lb.left,
            rect.top + (rect.height - lb.height) / 2.f - lb.top - 2.f);
    }
    else
    {
        lbl.setPosition(rect.left + 18.f, rect.top + 10.f);
    }
    w.draw(lbl);

    if (!subtitle.empty())
    {
        string subTxt = fitText(subtitle, *font, 13, rect.width - 36.f);
        sf::Text sub(subTxt, *font, 13);
        sub.setFillColor(sf::Color(200, 215, 240, 210));
        sub.setPosition(rect.left + 18.f, rect.top + 34.f);
        w.draw(sub);
    }
}

void TextField::handleEvent(const sf::Event& e)
{
    if (e.type == sf::Event::MouseButtonPressed &&
        e.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f mp((float)e.mouseButton.x, (float)e.mouseButton.y);
        focused = rect.contains(mp);
    }
    else if (focused && e.type == sf::Event::TextEntered)
    {
        sf::Uint32 u = e.text.unicode;

        if (u == 8) // Backspace
        {
            if (!text.empty())
                text.pop_back();
        }
        else if (u == 13 || u == 9)
        {
        }
        else if (u >= 32 && u < 127)
        {
            char c = (char)u;
            if (numeric && !(c >= '0' && c <= '9'))
                return;
            if (text.size() < maxLen)
                text += c;
        }
    }
}

void TextField::update(float dt)
{
    cursorBlink += dt;
    if (cursorBlink > 1.f)
        cursorBlink -= 1.f;
}

void TextField::draw(sf::RenderWindow& w)
{
    drawRoundedRect(w, rect.left, rect.top, rect.width, rect.height, 7.f,
        Theme::FIELD_BG,
        focused ? Theme::FIELD_BORDER_FOCUS : Theme::FIELD_BORDER,
        focused ? 2.f : 1.f);

    if (!font)
        return;

    string visible = text;
    if (password)
        visible = string(text.size(), '*');

    float textY = rect.top + (rect.height - 20.f) / 2.f;

    if (visible.empty() && !focused && !placeholder.empty())
    {
        sf::Text ph(placeholder, *font, 14);
        ph.setFillColor(Theme::TEXT_MUTED);
        ph.setStyle(sf::Text::Italic);
        ph.setPosition(rect.left + 12.f, textY);
        w.draw(ph);
    }
    else
    {
        sf::Text t(visible, *font, 16);
        t.setFillColor(Theme::TEXT_DARK);
        t.setPosition(rect.left + 12.f, textY);
        w.draw(t);

        if (focused && cursorBlink < 0.5f)
        {
            sf::FloatRect tb = t.getLocalBounds();
            float cx = rect.left + 12.f + tb.width + 2.f;
            sf::RectangleShape cur;
            cur.setSize(sf::Vector2f(2.f, rect.height - 12.f));
            cur.setPosition(cx, rect.top + 6.f);
            cur.setFillColor(Theme::FIELD_BORDER_FOCUS);
            w.draw(cur);
        }
    }
}

void Dialog::show(const string& t, const string& m, Kind k, bool withCancel)
{
    open = true;
    kind = k;
    title = t;
    message = m;
    lastResult = NONE;

    float dw = 480.f, dh = 220.f;
    float dx = (Theme::WIN_W - dw) / 2.f;
    float dy = (Theme::WIN_H - dh) / 2.f;

    btnOK.setFont(font);
    btnCancel.setFont(font);

    if (withCancel)
    {
        btnCancel.setRect(dx + dw - 240.f, dy + dh - 50.f, 100.f, 36.f);
        btnCancel.setLabel("Cancel");
        btnCancel.setVariant(Button::NEUTRAL);
    }
    btnOK.setRect(dx + dw - 130.f, dy + dh - 50.f, 110.f, 36.f);
    btnOK.setLabel(withCancel ? "Confirm" : "OK");
    btnOK.setVariant(k == ERROR_KIND ? Button::DANGER
        : k == SUCCESS ? Button::SUCCESS
        : Button::PRIMARY);
}

Dialog::Result Dialog::result()
{
    Result r = lastResult;
    lastResult = NONE;
    return r;
}

void Dialog::handleEvent(const sf::Event& e)
{
    if (!open)
        return;
    if (btnOK.handleEvent(e))
    {
        lastResult = OK;
        open = false;
    }
    if (btnCancel.getRect().width > 0.f && btnCancel.handleEvent(e))
    {
        lastResult = CANCEL;
        open = false;
    }
    if (e.type == sf::Event::KeyPressed)
    {
        if (e.key.code == sf::Keyboard::Escape)
        {
            lastResult = CANCEL;
            open = false;
        }
        if (e.key.code == sf::Keyboard::Enter)
        {
            lastResult = OK;
            open = false;
        }
    }
}

void Dialog::draw(sf::RenderWindow& w)
{
    if (!open || !font)
        return;

    sf::RectangleShape dim;
    dim.setSize(sf::Vector2f((float)Theme::WIN_W, (float)Theme::WIN_H));
    dim.setFillColor(sf::Color(0, 0, 0, 165));
    w.draw(dim);

    float dw = 490.f, dh = 230.f;
    float dx = (Theme::WIN_W - dw) / 2.f;
    float dy = (Theme::WIN_H - dh) / 2.f;

    sf::Color stripColor;
    switch (kind)
    {
    case ERROR_KIND:
        stripColor = Theme::BTN_DANGER;
        break;
    case SUCCESS:
        stripColor = Theme::BTN_SUCCESS;
        break;
    case CONFIRM:
        stripColor = Theme::ACCENT;
        break;
    default:
        stripColor = Theme::BTN_PRIMARY;
        break;
    }

    drawRoundedRect(w, dx + 4.f, dy + 6.f, dw, dh, 14.f, sf::Color(0, 0, 0, 90));
    drawRoundedRect(w, dx, dy, dw, dh, 14.f, Theme::PANEL, stripColor, 2.f);

    sf::RectangleShape sideStripe;
    sideStripe.setSize(sf::Vector2f(5.f, dh - 28.f));
    sideStripe.setPosition(dx + 14.f, dy + 14.f);
    sideStripe.setFillColor(stripColor);
    w.draw(sideStripe);

    sf::RectangleShape divLine;
    divLine.setSize(sf::Vector2f(dw - 60.f, 1.f));
    divLine.setPosition(dx + 30.f, dy + 52.f);
    divLine.setFillColor(Theme::DIVIDER);
    w.draw(divLine);

    sf::Text t(title, *font, 20);
    t.setStyle(sf::Text::Bold);
    t.setFillColor(Theme::TEXT_DARK);
    t.setPosition(dx + 30.f, dy + 16.f);
    w.draw(t);

    float my = dy + 62.f;
    string buf = message;
    while (!buf.empty())
    {
        size_t nl = buf.find('\n');
        string line = (nl == string::npos) ? buf : buf.substr(0, nl);
        string fittedLine = fitText(line, *font, 15, dw - 60.f);
        sf::Text m(fittedLine, *font, 15);
        m.setFillColor(Theme::TEXT_DARK);
        m.setPosition(dx + 30.f, my);
        w.draw(m);
        my += 24.f;
        if (nl == string::npos)
            break;
        buf = buf.substr(nl + 1);
    }

    btnOK.draw(w);
    if (btnCancel.getRect().width > 0.f)
        btnCancel.draw(w);
}

int Calendar::daysInMonth(int y, int m)
{
    static const int dm[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (m < 1 || m > 12)
        return 30;
    if (m == 2)
    {
        bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
        return leap ? 29 : 28;
    }
    return dm[m - 1];
}

int Calendar::weekdayOfFirst(int y, int m)
{
    tm t{};
    t.tm_year = y - 1900;
    t.tm_mon = m - 1;
    t.tm_mday = 1;
    mktime(&t);
    return t.tm_wday; // 0=Sun
}

void Calendar::rebuildButtons()
{
    btnPrev.setFont(font);
    btnNext.setFont(font);
    btnClose.setFont(font);

    btnPrev.setRect(pos.x + 12.f, pos.y + 12.f, 36.f, 30.f);
    btnPrev.setLabel("<");
    btnPrev.setVariant(Button::NEUTRAL);

    btnNext.setRect(pos.x + width - 48.f, pos.y + 12.f, 36.f, 30.f);
    btnNext.setLabel(">");
    btnNext.setVariant(Button::NEUTRAL);

    btnClose.setRect(pos.x + (width - 100.f) / 2.f, pos.y + 320.f, 100.f, 32.f);
    btnClose.setLabel("Cancel");
    btnClose.setVariant(Button::NEUTRAL);
}

void Calendar::openAt(int y, int m)
{
    if (y == 0 || m == 0)
    {
        time_t now = time(nullptr);
        tm* lt = localtime(&now);
        viewYear = lt->tm_year + 1900;
        viewMonth = lt->tm_mon + 1;
    }
    else
    {
        viewYear = y;
        viewMonth = m;
    }
    open = true;
    hasResult = false;
    rebuildButtons();
}

void Calendar::handleEvent(const sf::Event& e)
{
    if (!open)
        return;

    if (btnPrev.handleEvent(e))
    {
        if (--viewMonth < 1)
        {
            viewMonth = 12;
            --viewYear;
        }
    }
    if (btnNext.handleEvent(e))
    {
        if (++viewMonth > 12)
        {
            viewMonth = 1;
            ++viewYear;
        }
    }
    if (btnClose.handleEvent(e))
    {
        open = false;
        return;
    }

    if (e.type == sf::Event::MouseButtonPressed &&
        e.mouseButton.button == sf::Mouse::Left)
    {
        float gridX = pos.x + 16.f;
        float gridY = pos.y + 80.f;
        float cellW = (width - 32.f) / 7.f;
        float cellH = 32.f;

        int weekday = weekdayOfFirst(viewYear, viewMonth);
        int days = daysInMonth(viewYear, viewMonth);

        sf::Vector2f mp((float)e.mouseButton.x, (float)e.mouseButton.y);

        for (int d = 1; d <= days; ++d)
        {
            int idx = weekday + (d - 1);
            int row = idx / 7;
            int col = idx % 7;
            float cx = gridX + col * cellW;
            float cy = gridY + row * cellH;
            sf::FloatRect cellRect(cx + 2.f, cy + 2.f, cellW - 4.f, cellH - 4.f);
            if (cellRect.contains(mp))
            {
                selectedYear = viewYear;
                selectedMonth = viewMonth;
                selectedDay = d;
                char buf[16];
                snprintf(buf, sizeof(buf), "%04d-%02d-%02d",
                    selectedYear, selectedMonth, selectedDay);
                selectedISO = buf;
                hasResult = true;
                open = false;
                return;
            }
        }
    }

    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape)
        open = false;
}

void Calendar::draw(sf::RenderWindow& w)
{
    if (!open || !font)
        return;

    sf::RectangleShape dim;
    dim.setSize(sf::Vector2f((float)Theme::WIN_W, (float)Theme::WIN_H));
    dim.setFillColor(sf::Color(0, 0, 0, 165));
    w.draw(dim);

    float h = 360.f;

    drawRoundedRect(w, pos.x + 4.f, pos.y + 6.f, width, h, 14.f, sf::Color(0, 0, 0, 90));
    drawRoundedRect(w, pos.x, pos.y, width, h, 14.f, Theme::PANEL, Theme::ACCENT, 2.f);

    btnPrev.draw(w);
    btnNext.draw(w);

    static const char* monthNames[] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December" };
    string title = string(monthNames[viewMonth - 1]) + " " + to_string(viewYear);
    sf::Text t(title, *font, 18);
    t.setStyle(sf::Text::Bold);
    t.setFillColor(Theme::TEXT_DARK);
    sf::FloatRect tb = t.getLocalBounds();
    t.setPosition(pos.x + (width - tb.width) / 2.f, pos.y + 16.f);
    w.draw(t);

    static const char* days[] = { "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa" };
    float gridX = pos.x + 16.f;
    float gridY = pos.y + 80.f;
    float cellW = (width - 32.f) / 7.f;
    float cellH = 32.f;

    for (int i = 0; i < 7; ++i)
    {
        sf::Text wd(days[i], *font, 13);
        wd.setStyle(sf::Text::Bold);
        wd.setFillColor(Theme::TEXT_MUTED);
        sf::FloatRect b = wd.getLocalBounds();
        wd.setPosition(gridX + i * cellW + (cellW - b.width) / 2.f - 2.f,
            pos.y + 56.f);
        w.draw(wd);
    }

    int weekday = weekdayOfFirst(viewYear, viewMonth);
    int days_ = daysInMonth(viewYear, viewMonth);

    sf::Vector2i mpInt = sf::Mouse::getPosition(w);
    sf::Vector2f mp((float)mpInt.x, (float)mpInt.y);

    for (int d = 1; d <= days_; ++d)
    {
        int idx = weekday + (d - 1);
        int row = idx / 7;
        int col = idx % 7;
        float cx = gridX + col * cellW;
        float cy = gridY + row * cellH;

        sf::FloatRect cellRect(cx + 2.f, cy + 2.f, cellW - 4.f, cellH - 4.f);
        bool hover = cellRect.contains(mp);

        sf::RectangleShape cell;
        cell.setSize(sf::Vector2f(cellRect.width, cellRect.height));
        cell.setPosition(cellRect.left, cellRect.top);
        cell.setFillColor(hover ? Theme::BTN_PRIMARY_HOVER
            : sf::Color(28, 42, 78));
        cell.setOutlineColor(Theme::FIELD_BORDER);
        cell.setOutlineThickness(1.f);
        w.draw(cell);

        sf::Text dt(to_string(d), *font, 14);
        dt.setFillColor(Theme::TEXT_DARK);
        sf::FloatRect db = dt.getLocalBounds();
        dt.setPosition(cellRect.left + (cellRect.width - db.width) / 2.f - 1.f,
            cellRect.top + (cellRect.height - db.height) / 2.f - 2.f);
        w.draw(dt);
    }

    btnClose.draw(w);
}

void Table::setHeaders(const vector<string>& h)
{
    headers = h;
    recomputeColumnWidths();
}

void Table::setRows(const vector<vector<string>>& r)
{
    rows = r;
    selectedIndex = -1;
    scrollY = 0.f;
    recomputeColumnWidths();
}

void Table::recomputeColumnWidths()
{
    if (headers.empty())
    {
        colWidths.clear();
        return;
    }
    colWidths.assign(headers.size(), 0.f);
    if (!font)
        return;

    for (size_t i = 0; i < headers.size(); ++i)
    {
        sf::Text t(headers[i], *font, 13);
        colWidths[i] = max(colWidths[i], t.getLocalBounds().width + 24.f);
    }
    for (const auto& r : rows)
    {
        for (size_t i = 0; i < headers.size() && i < r.size(); ++i)
        {
            sf::Text t(r[i], *font, 13);
            colWidths[i] = max(colWidths[i], t.getLocalBounds().width + 24.f);
        }
    }

    float total = 0.f;
    for (float cw : colWidths)
        total += cw;
    float avail = rect.width - 16.f;
    if (total > 0.f)
    {
        float scale = avail / total;
        for (float& cw : colWidths)
            cw *= scale;
    }
}

void Table::handleEvent(const sf::Event& e)
{
    if (e.type == sf::Event::MouseMoved)
    {
        sf::Vector2f mp((float)e.mouseMove.x, (float)e.mouseMove.y);
        hoverIndex = -1;
        if (rect.contains(mp))
        {
            float relY = mp.y - (rect.top + headerHeight) + scrollY;
            int idx = (int)(relY / rowHeight);
            if (idx >= 0 && idx < (int)rows.size())
                hoverIndex = idx;
        }
    }
    else if (e.type == sf::Event::MouseButtonPressed &&
        e.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f mp((float)e.mouseButton.x, (float)e.mouseButton.y);
        if (rect.contains(mp) && mp.y > rect.top + headerHeight)
        {
            float relY = mp.y - (rect.top + headerHeight) + scrollY;
            int idx = (int)(relY / rowHeight);
            if (idx >= 0 && idx < (int)rows.size())
                selectedIndex = idx;
        }
    }
    else if (e.type == sf::Event::MouseWheelScrolled)
    {
        sf::Vector2f mp((float)e.mouseWheelScroll.x, (float)e.mouseWheelScroll.y);
        if (rect.contains(mp))
        {
            scrollY -= e.mouseWheelScroll.delta * 30.f;
            float maxScroll = max(0.f,
                rows.size() * rowHeight - (rect.height - headerHeight));
            if (scrollY < 0.f)
                scrollY = 0.f;
            if (scrollY > maxScroll)
                scrollY = maxScroll;
        }
    }
}

void Table::draw(sf::RenderWindow& w)
{
    if (!font)
        return;

    sf::RectangleShape body;
    body.setSize(sf::Vector2f(rect.width, rect.height));
    body.setPosition(rect.left, rect.top);
    body.setFillColor(Theme::PANEL);
    body.setOutlineColor(Theme::FIELD_BORDER);
    body.setOutlineThickness(1.f);
    w.draw(body);

    sf::RectangleShape hdr;
    hdr.setSize(sf::Vector2f(rect.width, headerHeight));
    hdr.setPosition(rect.left, rect.top);
    hdr.setFillColor(Theme::HEADER);
    w.draw(hdr);

    sf::RectangleShape hdrStripe;
    hdrStripe.setSize(sf::Vector2f(rect.width, 3.f));
    hdrStripe.setPosition(rect.left, rect.top + headerHeight - 3.f);
    hdrStripe.setFillColor(Theme::ACCENT);
    w.draw(hdrStripe);

    float x = rect.left + 10.f;
    for (size_t i = 0; i < headers.size(); ++i)
    {
        string hdrTxt = fitText(headers[i], *font, 14, colWidths[i] - 16.f);
        sf::Text t(hdrTxt, *font, 14);
        t.setStyle(sf::Text::Bold);
        t.setFillColor(Theme::TEXT_LIGHT);
        t.setPosition(x, rect.top + (headerHeight - 18.f) / 2.f);
        w.draw(t);
        x += colWidths[i];
    }

    sf::View prevView = w.getView();
    sf::View clipped(sf::FloatRect(rect.left, rect.top + headerHeight,
        rect.width, rect.height - headerHeight));
    sf::FloatRect vp(rect.left / (float)Theme::WIN_W,
        (rect.top + headerHeight) / (float)Theme::WIN_H,
        rect.width / (float)Theme::WIN_W,
        (rect.height - headerHeight) / (float)Theme::WIN_H);
    clipped.setViewport(vp);
    w.setView(clipped);

    float ry = (rect.top + headerHeight) - scrollY;
    for (size_t i = 0; i < rows.size(); ++i)
    {
        sf::RectangleShape rowBg;
        rowBg.setSize(sf::Vector2f(rect.width, rowHeight));
        rowBg.setPosition(rect.left, ry);
        if ((int)i == selectedIndex)
            rowBg.setFillColor(sf::Color(55, 85, 160));
        else if ((int)i == hoverIndex)
            rowBg.setFillColor(sf::Color(30, 45, 88));
        else
            rowBg.setFillColor(i % 2 ? sf::Color(24, 36, 68) : Theme::PANEL);
        w.draw(rowBg);

        sf::RectangleShape div;
        div.setSize(sf::Vector2f(rect.width, 1.f));
        div.setPosition(rect.left, ry + rowHeight - 1.f);
        div.setFillColor(Theme::DIVIDER);
        w.draw(div);

        float cx = rect.left + 10.f;
        for (size_t c = 0; c < headers.size() && c < rows[i].size(); ++c)
        {
            string cellTxt = fitText(rows[i][c], *font, 14, colWidths[c] - 18.f);
            sf::Text t(cellTxt, *font, 14);
            t.setFillColor((int)i == selectedIndex ? Theme::TEXT_LIGHT : Theme::TEXT_DARK);
            t.setPosition(cx, ry + (rowHeight - 18.f) / 2.f);
            w.draw(t);
            cx += colWidths[c];
        }

        ry += rowHeight;
    }

    w.setView(prevView);

    if (rows.empty())
    {
        sf::Text empty("No data to display.", *font, 15);
        empty.setStyle(sf::Text::Italic);
        empty.setFillColor(Theme::TEXT_MUTED);
        sf::FloatRect eb = empty.getLocalBounds();
        empty.setPosition(rect.left + (rect.width - eb.width) / 2.f,
            rect.top + headerHeight + 20.f);
        w.draw(empty);
    }
}
