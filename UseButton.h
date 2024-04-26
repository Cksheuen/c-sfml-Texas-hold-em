#include <iostream>
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

class UseButton {
private:
	float width, height, x, y;
	RenderWindow& window;
	Clock clock;
	UseShader<RectangleShape>* shader;
	RectangleShape ButtonShape;
	string content;
	bool hoverState = false;
	Font font;
	RenderTexture renderTexture;
	Text text;
public:
	UseButton(RenderWindow& windowSet, float xSet, float ySet, float heightSet, string contentSet, Font fontSet)
	: width(contentSet.length() * 24.), height(heightSet), window(windowSet), ButtonShape(Vector2f(width, height)),
	x(xSet), y(ySet), content(contentSet), font(fontSet){
		ButtonShape.setPosition(x - width / 2., y);
		shader = new UseShader<RectangleShape>(windowSet, clock, ButtonShape, "shaders/Button/button.vert", "shaders/Button/button.frag");
		shader->setShapeSize(width, height);

		text.setFont(font);
		//text.setPosition(x, y + height / 2. - 18.);
		text.setString(content);
		text.setCharacterSize(24);
		text.setFillColor(Color::White);

		renderTexture.create(width, height);
		renderTexture.clear(Color::Transparent);
		renderTexture.draw(text);
		renderTexture.display();


		shader->setTextTexture(renderTexture);
		cout << "renderTexture" << endl;
	}
	void show() {
		shader->useShader();
		//window.draw(text);
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
		}
		else {
			shader->shader.setUniform("hover", 0);
			hoverState = false;
			return false;
		}
	}
	bool click() {
		Vector2i mousePos = Mouse::getPosition(window);
		Event event;
		window.pollEvent(event);

		if (event.type == Event::MouseButtonPressed) {
			if (event.mouseButton.button == Mouse::Left) {
				if (ButtonShape.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
					cout << "button click" << endl;
					return true;
				}
			}
		}

	}
};