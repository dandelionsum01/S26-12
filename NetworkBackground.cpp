 #include "NetworkBackground.h"

static constexpr float DIST2_MIN =
NetworkBackground::CONNECT_DIST_MIN * NetworkBackground::CONNECT_DIST_MIN;
static constexpr float DIST2_MAX =
NetworkBackground::CONNECT_DIST_MAX * NetworkBackground::CONNECT_DIST_MAX;
static constexpr float DIST2 = DIST2_MAX;
static constexpr float CURSOR_DIST2 =
NetworkBackground::CURSOR_DIST * NetworkBackground::CURSOR_DIST;

void NetworkBackground::pushQuadSegment(sf::VertexArray& va,
    sf::Vector2f a, sf::Vector2f b,
    sf::Color c, float thickness) const
{
    sf::Vector2f dir = b - a;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len < 0.001f)
        return;
    dir /= len;
    sf::Vector2f perp(-dir.y * thickness * 0.5f,
        dir.x * thickness * 0.5f);
    va.append(sf::Vertex(a + perp, c));
    va.append(sf::Vertex(b + perp, c));
    va.append(sf::Vertex(b - perp, c));
    va.append(sf::Vertex(a - perp, c));
}

void NetworkBackground::drawThickLine(sf::RenderWindow& w,
    sf::Vector2f a, sf::Vector2f b,
    sf::Color c, float thickness) const
{
    sf::VertexArray quad(sf::Quads, 0);
    pushQuadSegment(quad, a, b, c, thickness);
    w.draw(quad);
}

void NetworkBackground::appendBezierQuads(sf::VertexArray& va,
    sf::Vector2f a, sf::Vector2f b,
    sf::Color c, float thickness,
    int segments) const
{
    sf::Vector2f mid = (a + b) * 0.5f;
    sf::Vector2f d = b - a;
    float len = std::sqrt(d.x * d.x + d.y * d.y);
    if (len < 0.001f)
        return;
    sf::Vector2f perp(-d.y / len, d.x / len);
    float off = len * 0.10f;
    sf::Vector2f ctrl = mid + perp * off;

    sf::Vector2f prev = a;
    for (int s = 1; s <= segments; ++s)
    {
        float t = (float)s / (float)segments;
        float u = 1.f - t;
        sf::Vector2f p = a * (u * u) + ctrl * (2.f * u * t) + b * (t * t);
        pushQuadSegment(va, prev, p, c, thickness);
        prev = p;
    }
}

void NetworkBackground::drawHUD(sf::RenderWindow& w) const
{
    sf::VertexArray grid(sf::Lines, 0);
    const sf::Color gc(60, 180, 220, 14);

    for (int x = 0; x < Theme::WIN_W; x += 64)
    {
        grid.append(sf::Vertex(sf::Vector2f((float)x, 0.f), gc));
        grid.append(sf::Vertex(sf::Vector2f((float)x, (float)Theme::WIN_H), gc));
    }
    for (int y = 0; y < Theme::WIN_H; y += 64)
    {
        grid.append(sf::Vertex(sf::Vector2f(0.f, (float)y), gc));
        grid.append(sf::Vertex(sf::Vector2f((float)Theme::WIN_W, (float)y), gc));
    }
    w.draw(grid);
}

void NetworkBackground::drawCornerBrackets(sf::RenderWindow& w) const
{
    sf::VertexArray va(sf::Quads, 0);
    const sf::Color cc(80, 210, 240, 100);
    const float arm = 38.f;
    const float thick = 2.f;
    const float pad = 10.f;
    const float W = (float)Theme::WIN_W;
    const float H = (float)Theme::WIN_H;

    sf::Vector2f corners[4] = {
        {pad, pad}, {W - pad, pad},
        {pad, H - pad}, {W - pad, H - pad}
    };
    sf::Vector2f dx[4] = {
        { arm, 0.f}, {-arm, 0.f},
        { arm, 0.f}, {-arm, 0.f}
    };
    sf::Vector2f dy[4] = {
        {0.f,  arm}, {0.f,  arm},
        {0.f, -arm}, {0.f, -arm}
    };

    for (int k = 0; k < 4; ++k)
    {
        pushQuadSegment(va, corners[k], corners[k] + dx[k], cc, thick);
        pushQuadSegment(va, corners[k], corners[k] + dy[k], cc, thick);
    }
    w.draw(va);
}

void NetworkBackground::init()
{
    srand((unsigned)time(nullptr));
    timeAccum = 0.f;

    int idx[NODE_COUNT];
    for (int i = 0; i < NODE_COUNT; ++i)
        idx[i] = i;
    for (int i = NODE_COUNT - 1; i > 0; --i)
    {
        int j = rand() % (i + 1);
        int t = idx[i];
        idx[i] = idx[j];
        idx[j] = t;
    }

    for (int i = 0; i < NODE_COUNT; ++i)
    {
        Node& n = nodes[i];
        n.pos = { (float)(rand() % Theme::WIN_W),
                 (float)(rand() % Theme::WIN_H) };
        float a = (float)(rand() % 628) / 100.f;
        float sp = 18.f + (float)(rand() % 22);
        n.vel = { sp * std::cos(a), sp * std::sin(a) };
        n.color = (rand() % 2)
            ? sf::Color(210, 120, 135, 180)
            : sf::Color(110, 150, 220, 180);
        n.radius = 3.f + (float)(rand() % 2);
        n.phase = (float)(rand() % 628) / 100.f;
        n.isTriCreator = false;
    }
    for (int i = 0; i < TRI_CREATOR_CNT; ++i)
        nodes[idx[i]].isTriCreator = true;

    for (auto& g : ghosts)
        g.life = 0.f;
    for (int i = 0; i < NODE_COUNT; ++i)
        for (int j = 0; j < NODE_COUNT; ++j)
            prevConn[i][j] = false;

    linkCount = 0;
}

void NetworkBackground::update(float dt)
{
    timeAccum += dt;

    const float W = (float)Theme::WIN_W;
    const float H = (float)Theme::WIN_H;

    auto bounce = [W, H](sf::Vector2f& p, sf::Vector2f& v)
        {
            if (p.x < 0.f) { p.x = 0.f; v.x = -v.x; }
            if (p.x > W) { p.x = W; v.x = -v.x; }
            if (p.y < 0.f) { p.y = 0.f; v.y = -v.y; }
            if (p.y > H) { p.y = H; v.y = -v.y; }
        };

    for (auto& n : nodes)
    {
        n.pos += n.vel * dt;
        bounce(n.pos, n.vel);
    }

    linkCount = 0;
    bool curConn[NODE_COUNT][NODE_COUNT];
    for (int i = 0; i < NODE_COUNT; ++i)
        for (int j = 0; j < NODE_COUNT; ++j)
            curConn[i][j] = false;

    for (int i = 0; i < NODE_COUNT; ++i)
    {
        for (int j = i + 1; j < NODE_COUNT; ++j)
        {
            if ((i * 31 + j) % 10 >= 5)
                continue;

            float dx = nodes[i].pos.x - nodes[j].pos.x;
            float dy = nodes[i].pos.y - nodes[j].pos.y;
            float d2 = dx * dx + dy * dy;
            if (d2 >= DIST2_MIN && d2 < DIST2_MAX)
            {
                links[linkCount++] = { i, j };
                curConn[i][j] = true;
            }
        }
    }

    for (int i = 0; i < NODE_COUNT; ++i)
    {
        for (int j = i + 1; j < NODE_COUNT; ++j)
        {
            if (prevConn[i][j] && !curConn[i][j])
            {
                for (auto& g : ghosts)
                {
                    if (g.life <= 0.f)
                    {
                        g.a = nodes[i].pos;
                        g.b = nodes[j].pos;
                        g.color = (i % 2 == 0)
                            ? sf::Color(222, 0, 0, 85)
                            : sf::Color(0, 45, 209, 85);
                        g.life = GHOST_LIFE;
                        break;
                    }
                }
            }
            prevConn[i][j] = curConn[i][j];
        }
    }

    for (auto& g : ghosts)
    {
        if (g.life > 0.f)
            g.life -= dt;
    }
}

void NetworkBackground::draw(sf::RenderWindow& w)
{
    drawHUD(w);
    drawCornerBrackets(w);

    sf::VertexArray tri(sf::Triangles, 0);
    const sf::Color triCol(255, 255, 255, 22);
    for (int i = 0; i < NODE_COUNT - 2; ++i)
    {
        if (!nodes[i].isTriCreator) continue;
        for (int j = i + 1; j < NODE_COUNT - 1; ++j)
        {
            if (!nodes[j].isTriCreator) continue;
            float dxij = nodes[i].pos.x - nodes[j].pos.x;
            float dyij = nodes[i].pos.y - nodes[j].pos.y;
            if (dxij * dxij + dyij * dyij >= DIST2) continue;
            for (int k = j + 1; k < NODE_COUNT; ++k)
            {
                if (!nodes[k].isTriCreator) continue;
                float dxik = nodes[i].pos.x - nodes[k].pos.x;
                float dyik = nodes[i].pos.y - nodes[k].pos.y;
                if (dxik * dxik + dyik * dyik >= DIST2) continue;
                float dxjk = nodes[j].pos.x - nodes[k].pos.x;
                float dyjk = nodes[j].pos.y - nodes[k].pos.y;
                if (dxjk * dxjk + dyjk * dyjk >= DIST2) continue;
                tri.append(sf::Vertex(nodes[i].pos, triCol));
                tri.append(sf::Vertex(nodes[j].pos, triCol));
                tri.append(sf::Vertex(nodes[k].pos, triCol));
            }
        }
    }
    if (tri.getVertexCount() > 0)
        w.draw(tri);

    sf::VertexArray linkQuads(sf::Quads, 0);
    for (int li = 0; li < linkCount; ++li)
    {
        int i = links[li].a;
        int j = links[li].b;
        sf::Color lc = (i % 2 == 0)
            ? sf::Color(222, 0, 0, 85)
            : sf::Color(0, 45, 209, 85);
        appendBezierQuads(linkQuads, nodes[i].pos, nodes[j].pos, lc, 2.f, 10);
    }

    for (const auto& g : ghosts)
    {
        if (g.life <= 0.f) continue;
        float k = g.life / GHOST_LIFE;
        sf::Color c = g.color;
        c.a = (sf::Uint8)(c.a * k);
        appendBezierQuads(linkQuads, g.a, g.b, c, 1.6f, 8);
    }

    sf::Vector2i mp = sf::Mouse::getPosition(w);
    sf::Vector2f mouse((float)mp.x, (float)mp.y);
    bool mouseInside = mp.x >= 0 && mp.y >= 0
        && mp.x < Theme::WIN_W && mp.y < Theme::WIN_H;

    bool nearMouse[NODE_COUNT] = { false };
    if (mouseInside)
    {
        for (int i = 0; i < NODE_COUNT; ++i)
        {
            float dx = nodes[i].pos.x - mouse.x;
            float dy = nodes[i].pos.y - mouse.y;
            float d2 = dx * dx + dy * dy;
            if (d2 < CURSOR_DIST2)
            {
                nearMouse[i] = true;
                float dist = std::sqrt(d2);
                auto alpha = (sf::Uint8)((1.f - dist / CURSOR_DIST) * 160.f);
                sf::Color lc(180, 230, 255, alpha);
                pushQuadSegment(linkQuads, mouse, nodes[i].pos, lc, 1.5f);
            }
        }
    }

    if (linkQuads.getVertexCount() > 0)
        w.draw(linkQuads);

    sf::CircleShape circle;
    circle.setOutlineThickness(0.f);
    for (int i = 0; i < NODE_COUNT; ++i)
    {
        const Node& n = nodes[i];
        float pulse = 1.f + 0.20f * std::sin(timeAccum * 2.4f + n.phase);
        float r = n.radius * pulse;
        sf::Color c = n.color;
        if (nearMouse[i])
        {
            r *= 1.6f;
            c = sf::Color(255, 255, 255, 230);
        }
        circle.setRadius(r);
        circle.setFillColor(c);
        circle.setPosition(n.pos.x - r, n.pos.y - r);
        w.draw(circle);
    }

}
