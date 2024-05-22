#include "UseAlert.h"
#include "UseCal.h"
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <functional>
#include <iostream>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

UseAlert::UseAlert(RenderWindow &windowSet, int widthSet, int heightSet,
                   Font fontSet)
    : window(windowSet), font(fontSet), width(widthSet), height(heightSet) {
  clock.restart();
};

void UseAlert::addAlert(string alert) {
  Text text;
  text.setFont(font);
  text.setString(alert);
  text.setCharacterSize(24);
  text.setFillColor(Color::Red);
  text.setPosition(width / 2 - alert.length() * width / 100, height / 8);

  alertList.push_back(text);
  alertTime.push_back(ANIMATION_TIME + clock.getElapsedTime().asSeconds());
}

void UseAlert::printAlert() {
  int i = 0;

  while (i < alertList.size()) {
    if (alertTime[i] < clock.getElapsedTime().asSeconds()) {
      alertList.erase(alertList.begin() + i);
      alertTime.erase(alertTime.begin() + i);
    } else {
      color.a = 255 * (1. - smoothstep(0, ANIMATION_TIME / 2,
                                       fabs(alertTime[i] -
                                            clock.getElapsedTime().asSeconds() -
                                            ANIMATION_TIME / 2)));
      alertList[i].setFillColor(color);
      alertList[i].setPosition(
          alertList[i].getPosition().x,
          height / 8. *
              (smoothstep(0, ANIMATION_TIME,
                          alertTime[i] - clock.getElapsedTime().asSeconds())));
      window.draw(alertList[i]);
      i++;
    }
  }
}