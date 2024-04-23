#include <iostream>
#include <math.h>
#include <cassert>
#include <SFML/Graphics.hpp>
#include "UseCard.h"

using namespace std;
using namespace sf;

const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 800;

void Input(RenderWindow& window, sf::RectangleShape& button);

bool isPlaying = true;

template<class T>
const T& clamp(const T& v, const T& lo, const T& hi) {
    assert(!(hi < lo));
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

float smoothstep(float edge0, float edge1, float x) {
    // Clamp x to range [edge0, edge1]
    x = clamp(static_cast<double>((x - edge0) / (edge1 - edge0)), 0.0, 1.0);
    return x * x * (3 - 2 * x);
}

float step(float edge, float x){
    return x < edge ? 0.0 : 1.0;
}

int main() {
    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "SFML Game Project");

    //UseCard shader(window, 5, CardType_Spade, 100, 100, 200, 300);
    UseCard* cards[52];

    int index = 0;
    for (CardType type = CardType_Heart; type < CardType_Joker; type = (CardType)(type + 1)) {
        for (int i = 1; i <= 13; i++) {
            cards[index++] = new UseCard(window, i, type, 0, WINDOW_HEIGHT / 2, WINDOW_HEIGHT / 12, WINDOW_WIDTH / 6);
        }
    }

    Clock clock;

    sf::RectangleShape button(sf::Vector2f(50, 50));
    button.setPosition(500, 100);
    button.setFillColor(sf::Color::Red);

    float circle_center_x = WINDOW_WIDTH / 2;
    float circle_center_y = WINDOW_HEIGHT;

    while (isPlaying) {
        Input(window, button);
        window.clear(sf::Color::White);

        for (int i = 0; i < 52; i++) {
            float start_theta = atan2f(-WINDOW_HEIGHT / 2., -WINDOW_WIDTH / 2.) / 3.14 * 180;
            float end_theta = (abs(start_theta) - 90) / 52 * 2. * i;
            float now_theta = smoothstep(float(0.), 1., clock.getElapsedTime().asSeconds() ) * end_theta;
            float x = circle_center_x + cosf((start_theta + now_theta) / 180 * 3.14) * WINDOW_WIDTH / 1;
            float y = circle_center_y + sinf((start_theta + now_theta) / 180 * 3.14) * WINDOW_HEIGHT / 2;
            cards[i]->setPos(x, y);
            cards[i]->setRotation(start_theta + now_theta - 90);

			cards[i]->drawCard();
		}

        window.display();
    }
    window.close();

    return 0;
}

void Input(RenderWindow& window, sf::RectangleShape& button) {
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            window.close();

        if (event.type == sf::Event::MouseButtonPressed)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                // 获取鼠标的位置
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);

                // 检查鼠标是否在按键上
                if (button.getGlobalBounds().contains(mousePos.x, mousePos.y))
                {
                    // 鼠标在按键上，处理点击事件
                    std::cout << "Button clicked!" << std::endl;
                    //shader.disappearCard();
                }
            }
        }

        if (Keyboard::isKeyPressed(Keyboard::Escape)) {
            isPlaying = false;
        }
    }
}