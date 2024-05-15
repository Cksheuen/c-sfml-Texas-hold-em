#ifndef USE_BUTTON
#define USE_BUTTON

#include <SFML/Graphics.hpp>
#include <iostream>
#include "UseShader.h"
#include "UseCal.h"

using namespace std;
using namespace sf;

class UseButton {
private:
  float width, height, x, y, dx, dy;
  RenderWindow &window;
  Clock clock, animation_clock;
  UseShader<RectangleShape> *shader;
  RectangleShape ButtonShape;
  Font font;
  RenderTexture renderTexture;
  Text text;
  float start_time;

public:
  string content;
  bool hoverState = false;
  UseButton(RenderWindow &windowSet, float xSet, float ySet, float heightSet,
            string contentSet, Font fontSet)
      : width(contentSet.length() * 24.), height(heightSet), window(windowSet),
        ButtonShape(Vector2f(width, height)), x(xSet), y(ySet),
        content(contentSet), font(fontSet) {
    x = x - width / 2.;
    y = y + height / 10.;
    ButtonShape.setPosition(x, y);
    shader = new UseShader<RectangleShape>(windowSet, clock, ButtonShape,
                                           "shaders/Button/button.vert",
                                           "shaders/Button/button.frag");
    shader->setShapeSize(width, height);

    dx = 0;
    dy = -height / 10.;

    text.setFont(font);
    text.setString(content);
    text.setCharacterSize(24);
    text.setFillColor(Color::White);

    renderTexture.create(width, height);
    renderTexture.clear(Color::Transparent);
    renderTexture.draw(text);
    renderTexture.display();

    shader->setTextTexture(renderTexture);

    start_time = animation_clock.getElapsedTime().asSeconds();
  }
  void moveTo(float xSet, float ySet) {
    x = ButtonShape.getPosition().x;
    y = ButtonShape.getPosition().y;
    dx = xSet - x;
    dy = ySet - y;
    start_time = animation_clock.getElapsedTime().asSeconds();
  }
  void show() {
    float time =
        smoothstep(0., 1., animation_clock.getElapsedTime().asSeconds() - start_time);
    if (time < 1. && time > 0. ) cout << "y " <<  y << endl;

    ButtonShape.setPosition(x + dx * (1, -time), y + dy * (1. - time));
    shader->updatePos();

    shader->useShader();
  }
  void setHoverState(bool hoverStateSet) {
    if (hoverStateSet) {
      shader->shader.setUniform("hover", 1);
    } else {
      shader->shader.setUniform("hover", 0);
    }
  }

  bool hover() {
    Vector2i mousePos = Mouse::getPosition(window);
    if (ButtonShape.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
      if (!hoverState) {
        hoverState = true;
        clock.restart();
      }
      shader->shader.setUniform("hover", 1);
      return true;
    } else {
      if (hoverState) {
        shader->shader.setUniform("hover", 0);
        hoverState = false;
      }
      return false;
    }
  }
  bool click() {
    Event event;
    window.pollEvent(event);

    if (event.type == Event::MouseButtonPressed &&
        event.mouseButton.button == Mouse::Left) {
      if (hoverState)
        return true;
    }
    return false;
  }
  void setString(string contentSet) {
    content = contentSet;
    width = content.length() * 24.;
    ButtonShape.setSize(Vector2f(width, height));
    ButtonShape.setPosition(x, y);

    shader->setShapeSize(width, height);

    text.setString(content);

    renderTexture.create(width, height);
    renderTexture.clear(Color::Transparent);
    renderTexture.draw(text);
    renderTexture.display();

    shader->setTextTexture(renderTexture);
  }
};

#endif