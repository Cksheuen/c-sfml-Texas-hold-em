#ifndef USE_ALERT
#define USE_ALERT

#include <iostream>
#include <vector>
#include <SFML/Network.hpp>
#include <thread>
#include <functional>
#include <random>
#include <mutex>
#include <SFML/Graphics.hpp>
#include <string>
#include "UseCal.h"

#define ANIMATION_TIME 1.

using namespace std;
using namespace sf;

class UseAlert {
private:
	RenderWindow& window;
	vector<Text> alertList;
	vector<float> alertTime;
	Clock clock;
	int width, height;
	
	Color color = Color::Red;

public:
	Font font;
	UseAlert(RenderWindow& windowSet, int widthSet, int heightSet
		, Font fontSet) : window(windowSet), font(fontSet), width(widthSet), height(heightSet)  {
		clock.restart();
	};

	void addAlert(string alert) {
		Text text;
		text.setFont(font);
		text.setString(alert);
		text.setCharacterSize(24);
		text.setFillColor(Color::Red);
		text.setPosition(width / 2 - alert.length() * width / 100, height / 8);

		alertList.push_back(text);
		alertTime.push_back(ANIMATION_TIME + clock.getElapsedTime().asSeconds());
	}

	void printAlert() {
		int i = 0;

		while (i < alertList.size()) {
			if (alertTime[i] < clock.getElapsedTime().asSeconds()) {
				alertList.erase(alertList.begin() + i);
				alertTime.erase(alertTime.begin() + i);
			}
			else {
				color.a = 255 * (1. - smoothstep(0, ANIMATION_TIME / 2, fabs(alertTime[i] - clock.getElapsedTime().asSeconds() - ANIMATION_TIME / 2 ) ));
				alertList[i].setFillColor(color);
				alertList[i].setPosition(alertList[i].getPosition().x,
					height / 8. 
					* ( smoothstep(0, ANIMATION_TIME, alertTime[i] - clock.getElapsedTime().asSeconds() )));
				window.draw(alertList[i]);
				i++;
			}
		}
	}
};

#endif