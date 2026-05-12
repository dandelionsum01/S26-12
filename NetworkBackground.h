#pragma once
#include "GuiCore.h"

class NetworkBackground
{
public:
    static constexpr int NODE_COUNT = 50;
    static constexpr int BLUR_COUNT = 30;
    static constexpr int CONNECTOR_COUNT = 3;
    static constexpr int TRI_CREATOR_CNT = 10;
    static constexpr float CONNECT_DIST = 300.f;

    void init();
    void update(float dt);
    void draw(sf::RenderWindow& w);

private:
    struct Node
    {
        sf::Vector2f pos, vel;
        sf::Color color;
        float radius;
        bool isConnector;
        bool isTriCreator;
    };

    struct BlurNode
    {
        sf::Vector2f pos, vel;
        float radius;
    };

    Node nodes[NODE_COUNT];
    BlurNode blurNodes[BLUR_COUNT];

    void drawThickLine(sf::RenderWindow& w,
        sf::Vector2f a, sf::Vector2f b,
        sf::Color c, float thickness) const;

    void drawBlurLine(sf::RenderWindow& w,
        sf::Vector2f a, sf::Vector2f b,
        sf::Color c) const;

    void drawBlurNode(sf::RenderWindow& w, const BlurNode& n) const;
};
