#include "NetworkBackground.h"

static constexpr float DIST2 =
NetworkBackground::CONNECT_DIST * NetworkBackground::CONNECT_DIST;

void NetworkBackground::drawThickLine(sf::RenderWindow& w,
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

    sf::VertexArray quad(sf::Quads, 4);
    quad[0] = sf::Vertex(a + perp, c);
    quad[1] = sf::Vertex(b + perp, c);
    quad[2] = sf::Vertex(b - perp, c);
    quad[3] = sf::Vertex(a - perp, c);
    w.draw(quad);
}

void NetworkBackground::drawBlurLine(sf::RenderWindow& w,
    sf::Vector2f a, sf::Vector2f b,
    sf::Color c) const
{
    drawThickLine(w, a, b,
        sf::Color(c.r, c.g, c.b, (sf::Uint8)(c.a / 6)), 7.f);
    drawThickLine(w, a, b,
        sf::Color(c.r, c.g, c.b, (sf::Uint8)(c.a / 3)), 3.5f);
    drawThickLine(w, a, b, c, 1.2f);
}

void NetworkBackground::drawBlurNode(sf::RenderWindow& w,
    const BlurNode& n) const
{
    sf::CircleShape c;
    c.setOutlineThickness(0.f);

    float r0 = n.radius * 2.4f;
    c.setRadius(r0);
    c.setFillColor(sf::Color(255, 255, 255, 8));
    c.setPosition(n.pos.x - r0, n.pos.y - r0);
    w.draw(c);

    float r1 = n.radius * 1.6f;
    c.setRadius(r1);
    c.setFillColor(sf::Color(255, 255, 255, 18));
    c.setPosition(n.pos.x - r1, n.pos.y - r1);
    w.draw(c);

    c.setRadius(n.radius);
    c.setFillColor(sf::Color(255, 255, 255, 72));
    c.setPosition(n.pos.x - n.radius, n.pos.y - n.radius);
    w.draw(c);
}

void NetworkBackground::init()
{
    srand((unsigned)time(nullptr));

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
        n.isConnector = false;
        n.isTriCreator = false;
    }
    for (int i = 0; i < CONNECTOR_COUNT; ++i)
        nodes[idx[i]].isConnector = true;
    for (int i = 0; i < TRI_CREATOR_CNT; ++i)
        nodes[idx[i]].isTriCreator = true;

    for (auto& n : blurNodes)
    {
        n.pos = { (float)(rand() % Theme::WIN_W),
                 (float)(rand() % Theme::WIN_H) };
        float a = (float)(rand() % 628) / 100.f;
        float sp = 8.f + (float)(rand() % 8);
        n.vel = { sp * std::cos(a), sp * std::sin(a) };
        n.radius = 4.5f + (float)(rand() % 2);
    }
}

void NetworkBackground::update(float dt)
{
    const float W = (float)Theme::WIN_W;
    const float H = (float)Theme::WIN_H;

    auto bounce = [W, H](sf::Vector2f& p, sf::Vector2f& v)
        {
            if (p.x < 0.f)
            {
                p.x = 0.f;
                v.x = -v.x;
            }
            if (p.x > W)
            {
                p.x = W;
                v.x = -v.x;
            }
            if (p.y < 0.f)
            {
                p.y = 0.f;
                v.y = -v.y;
            }
            if (p.y > H)
            {
                p.y = H;
                v.y = -v.y;
            }
        };

    for (auto& n : nodes)
    {
        n.pos += n.vel * dt;
        bounce(n.pos, n.vel);
    }
    for (auto& n : blurNodes)
    {
        n.pos += n.vel * dt;
        bounce(n.pos, n.vel);
    }
}

void NetworkBackground::draw(sf::RenderWindow& w)
{
    for (int i = 0; i < BLUR_COUNT; ++i)
    {
        for (int j = i + 1; j < BLUR_COUNT; ++j)
        {
            float dx = blurNodes[i].pos.x - blurNodes[j].pos.x;
            float dy = blurNodes[i].pos.y - blurNodes[j].pos.y;
            float d2 = dx * dx + dy * dy;
            if (d2 >= DIST2)
                continue;

            float dist = std::sqrt(d2);
            auto alpha = (sf::Uint8)((1.f - dist / CONNECT_DIST) * 55.f);
            sf::Color lc = (i % 2 == 0)
                ? sf::Color(222, 0, 0, alpha)
                : sf::Color(0, 45, 209, alpha);

            drawBlurLine(w, blurNodes[i].pos, blurNodes[j].pos, lc);
        }
    }

    for (const auto& n : blurNodes)
        drawBlurNode(w, n);

    sf::VertexArray tri(sf::Triangles, 3);
    const sf::Color triCol(255, 255, 255, 22);

    for (int i = 0; i < NODE_COUNT - 2; ++i)
    {
        if (!nodes[i].isTriCreator)
            continue;
        for (int j = i + 1; j < NODE_COUNT - 1; ++j)
        {
            if (!nodes[j].isTriCreator)
                continue;
            float dxij = nodes[i].pos.x - nodes[j].pos.x;
            float dyij = nodes[i].pos.y - nodes[j].pos.y;
            if (dxij * dxij + dyij * dyij >= DIST2)
                continue;

            for (int k = j + 1; k < NODE_COUNT; ++k)
            {
                if (!nodes[k].isTriCreator)
                    continue;

                float dxik = nodes[i].pos.x - nodes[k].pos.x;
                float dyik = nodes[i].pos.y - nodes[k].pos.y;
                if (dxik * dxik + dyik * dyik >= DIST2)
                    continue;

                float dxjk = nodes[j].pos.x - nodes[k].pos.x;
                float dyjk = nodes[j].pos.y - nodes[k].pos.y;
                if (dxjk * dxjk + dyjk * dyjk >= DIST2)
                    continue;

                tri[0] = sf::Vertex(nodes[i].pos, triCol);
                tri[1] = sf::Vertex(nodes[j].pos, triCol);
                tri[2] = sf::Vertex(nodes[k].pos, triCol);
                w.draw(tri);
            }
        }
    }

    for (int i = 0; i < NODE_COUNT; ++i)
    {
        if (!nodes[i].isConnector)
            continue;
        for (int j = 0; j < NODE_COUNT; ++j)
        {
            if (i == j)
                continue;
            float dx = nodes[i].pos.x - nodes[j].pos.x;
            float dy = nodes[i].pos.y - nodes[j].pos.y;
            float d2 = dx * dx + dy * dy;
            if (d2 >= DIST2)
                continue;

            float dist = std::sqrt(d2);
            auto alpha = (sf::Uint8)((1.f - dist / CONNECT_DIST) * 85.f);
            sf::Color lc = (i % 2 == 0)
                ? sf::Color(222, 0, 0, alpha)
                : sf::Color(0, 45, 209, alpha);

            drawThickLine(w, nodes[i].pos, nodes[j].pos, lc, 2.f);
        }
    }

    sf::CircleShape circle;
    circle.setOutlineThickness(0.f);
    for (const auto& n : nodes)
    {
        circle.setRadius(n.radius);
        circle.setFillColor(n.color);
        circle.setPosition(n.pos.x - n.radius, n.pos.y - n.radius);
        w.draw(circle);
    }
}
