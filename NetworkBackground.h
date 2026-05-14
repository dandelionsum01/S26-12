#pragma once
#include "GuiCore.h"

class NetworkBackground
{
public:
    static constexpr int NODE_COUNT = 50;
    static constexpr int TRI_CREATOR_CNT = 10;
    static constexpr int GHOST_POOL = 48;
    static constexpr int LINK_CAP = NODE_COUNT * (NODE_COUNT - 1) / 2;
    static constexpr float CONNECT_DIST_MIN = 100.f;
    static constexpr float CONNECT_DIST_MAX = 300.f;
    static constexpr float CONNECT_DIST = CONNECT_DIST_MAX;
    static constexpr float CURSOR_DIST = 180.f;
    static constexpr float GHOST_LIFE = 0.45f;

    void init();
    void setFont(sf::Font* f) { font = f; }
    void update(float dt);
    void draw(sf::RenderWindow& w);

private:
    struct Node
    {
        sf::Vector2f pos, vel;
        sf::Color color;
        float radius;
        float phase;
        bool isTriCreator;
    };

    struct Ghost
    {
        sf::Vector2f a, b;
        sf::Color color;
        float life;
    };

    struct Link
    {
        int a, b;
    };

    Node nodes[NODE_COUNT];
    Ghost ghosts[GHOST_POOL];
    Link links[LINK_CAP];
    int linkCount = 0;
    bool prevConn[NODE_COUNT][NODE_COUNT];

    sf::Font* font = nullptr;
    float timeAccum = 0.f;

    void drawThickLine(sf::RenderWindow& w,
        sf::Vector2f a, sf::Vector2f b,
        sf::Color c, float thickness) const;

    void pushQuadSegment(sf::VertexArray& va,
        sf::Vector2f a, sf::Vector2f b,
        sf::Color c, float thickness) const;

    void appendBezierQuads(sf::VertexArray& va,
        sf::Vector2f a, sf::Vector2f b,
        sf::Color c, float thickness,
        int segments = 10) const;

    void drawHUD(sf::RenderWindow& w) const;
    void drawCornerBrackets(sf::RenderWindow& w) const;
};
