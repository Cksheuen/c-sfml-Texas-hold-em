#ifndef USE_BUTTON
#define USE_BUTTON

#define ANIMATION_TIME 0.2

#include "UseCal.h"
#include "UseShader.h"
#include <SFML/Graphics.hpp>
#include <iostream>

using namespace std;
using namespace sf;

class UseButton {
private:
  float width, height, x, y, dx, dy, origin_x, origin_y;
  RenderWindow &window;
  Clock clock, animation_clock;
  UseShader<RectangleShape> *shader;
  RectangleShape ButtonShape;
  Font font;
  RenderTexture renderTexture;
  Text text;
  float start_time;
  bool first_show = true;
  int points;

public:
  string content;
  bool hoverState = false;
  UseButton(RenderWindow &windowSet, float xSet, float ySet, float heightSet,
            string contentSet, Font fontSet)
      : height(heightSet), points(height * .25),
        width(contentSet.length() * points), window(windowSet), origin_x(xSet),
        origin_y(ySet), content(contentSet), font(fontSet) {
    cout << "Button Created: " << content << endl;
    width = contentSet.length() * points;
    ButtonShape.setSize(Vector2f(width, height));

    x = origin_x - width / 2.;
    y = origin_y - height / 10.;
    ButtonShape.setPosition(x, y);
    shader = new UseShader<RectangleShape>(windowSet, clock, ButtonShape,
                                           "shaders/Button/button.vert",
                                           "shaders/Button/button.frag");
    shader->setShapeSize(width, height);

    dx = 0;
    dy = height / 10.;

    text.setFont(font);
    text.setString(content);
    text.setCharacterSize(points);
    text.setFillColor(Color::White);

    renderTexture.create(width, height);
    renderTexture.clear(Color::Transparent);
    renderTexture.draw(text);
    renderTexture.display();

    shader->setTextTexture(renderTexture);

    start_time = animation_clock.getElapsedTime().asSeconds();
  }
  
  void clearTime() {
    start_time = animation_clock.getElapsedTime().asSeconds();
    first_show = true;
  }

  void moveTo(float xSet, float ySet) {
    origin_x = xSet;
    origin_y = ySet;

    x = origin_x - width / 2.;
    y = origin_y - height / 10.;
    ButtonShape.setPosition(x, y);

    dx = 0;
    dy = height / 10.;

    start_time = animation_clock.getElapsedTime().asSeconds();
  }
  void show() {
    float time =
        smoothstep(0., ANIMATION_TIME,
                   animation_clock.getElapsedTime().asSeconds() - start_time);
    if (time < 1. && time > 0. && first_show)
      shader->setOpacity(time);
    else if (first_show)
      first_show = false;

    ButtonShape.setPosition(x + dx * (1. - time), y + dy * (1. - time));
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
    width = content.length() * points;
    ButtonShape.setSize(Vector2f(width, height));
    x = origin_x - width / 2.;
    y = origin_y;
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