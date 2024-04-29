#include <iostream>
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

map<int, Vector3f> ChipColor {
	{1, Vector3f(222, 126, 127)},
	{2, Vector3f(67, 61, 56)},
	{5, Vector3f(182, 36, 22)},
	{10, Vector3f(51, 100, 179)},
	{20, Vector3f(40, 93, 50)},
	{25, Vector3f(196, 36, 95)},
	{50, Vector3f(29, 34, 74)},
	{100, Vector3f(21, 19, 18)},
};

class UseChip {
private:
	float width, radius, x, y;
	RenderWindow& window;
	Clock clock;
	UseShader<CircleShape>* shader;
	CircleShape ChipShape;
	int value;
	bool hoverState = false;
	Font font;
	RenderTexture renderTexture;
	Text text;
public:
	UseChip(RenderWindow& windowSet, float xSet, float ySet, float radiusSet, int valueSet, Font fontSet)
		:  radius(radiusSet), window(windowSet), ChipShape(radius),
		x(xSet), y(ySet), value(valueSet), font(fontSet) {
		ChipShape.setPosition(x - radius / 2., y);
		shader = new UseShader<CircleShape>(windowSet, clock, ChipShape, "shaders/Chip/chip.vert", "shaders/Chip/chip.frag");
		shader->setShapeSize(radius);
		shader->shader.setUniform("color_set_in", ChipColor[value]);

		text.setFont(font);
		text.setString(to_string(value));
		text.setCharacterSize(radius / 50 * 24);
		text.setFillColor(Color::White);


		sf::FloatRect textBounds = text.getLocalBounds();

		float x = (radius - textBounds.width) / 2 - textBounds.left;
		float y = (radius - textBounds.height) / 2 - textBounds.top;

		text.setPosition(x, y);

		int width = to_string(value).length() * 16;
		renderTexture.create(radius, radius);
		renderTexture.clear(Color::Transparent);
		renderTexture.draw(text);
		renderTexture.display();


		shader->setTextTexture(renderTexture);
	}
	void show() {
		shader->useShader();
	}
	bool hover() {
		Vector2i mousePos = Mouse::getPosition(window);
		if (ChipShape.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
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
				if (ChipShape.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
					return true;
				}
			}
		}

	}
};