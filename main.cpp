#include <iostream>
#include <SFML/Graphics.hpp>
#include "UseCard.h"

using namespace std;
using namespace sf;

void Input(RenderWindow& window, sf::RectangleShape& button, UseCard& shader);

bool isPlaying = true;

int main() {
    RenderWindow window(VideoMode(1000, 800), "SFML Game Project");

    UseCard shader(window, 13, CardType_Spade, 100, 100, 200, 300);

    sf::RectangleShape button(sf::Vector2f(50, 50));
    button.setPosition(500, 100);
    button.setFillColor(sf::Color::Red);

    while (isPlaying) {
        Input(window, button, shader);
        window.clear(sf::Color::White);

        window.draw(button);

        shader.drawCard();
    }
    window.close();

    return 0;
}

void Input(RenderWindow& window, sf::RectangleShape& button, UseCard& shader) {
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
                    shader.disappearCard();
                }
            }
        }

        if (Keyboard::isKeyPressed(Keyboard::Escape)) {
            isPlaying = false;
        }
    }
}