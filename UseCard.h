#include <iostream>
#include <SFML/Graphics.hpp>
#include "UseShader.h"

using namespace std;
using namespace sf;

// 扑克牌类型
enum CardType
{
	CardType_None = 0,
	CardType_Heart,		// 红桃
	CardType_Spade,		// 黑桃
	CardType_Club,		// 梅花
	CardType_Diamond,	// 方块
	CardType_Joker,		// 大小王
};

map<enum CardType, string> cardTypeMap = {
	{CardType_Heart, "Heart"},
	{CardType_Spade, "Spade"},
	{CardType_Club, "Club"},
	{CardType_Diamond, "Diamond"},
	{CardType_Joker, "Joker"}
};

class UseCard {
private:
	// 扑克点数
	int number;
	bool check = false;
	enum CardType cardType;
	UseShader<RectangleShape>* shader;
	UseShader<Sprite>**cardTypeSign;
	RenderWindow& window;
	float x, y, width, height, signWidth;
	sf::RectangleShape Cardshape;
	sf::Sprite* cardTypeShape;
	Clock clock, disappearClock;
	Texture cardTypeTexture;
	Font font, mid_font;
	Text text, mid_text;
	RenderTexture renderTexture, mid_renderTexture;

	int min(int x, int y) {
		return x < y ? x : y;
	}
	int max(int x, int y) {
		return x > y ? x : y;
	}


public:
	UseCard(sf::RenderWindow& windowSet, int numberSet, enum CardType cardTypeSet, float xSet, float ySet, float widthSet, float heightSet)
		: window(windowSet), number(numberSet), cardType(cardTypeSet), x(xSet), y(ySet), width(widthSet), height(heightSet), Cardshape(sf::Vector2f(window.getSize().x, window.getSize().y)), signWidth(width / 5) {
		Cardshape.setPosition(0., 0.);
		//Cardshape.setPosition(x + width / 2, y + height / 2);
		//Cardshape.setOrigin(width / 2, height / 2);
		shader = new UseShader<RectangleShape>(windowSet, clock, Cardshape, "shaders/card/card.vert", "shaders/card/card.frag");
		shader->setShapeSize(width, height);
		shader->initMidTexture();
		shader->setGlobalPosition(x, y);

		cardTypeSign = NULL;

		if (!font.loadFromFile("assets/TeyvatBlack-Regular.otf")) {
			cout << "Load TeyvatBlack-Regular.ttf failed!" << endl;
		} else cout << "Load TeyvatBlack-Regular.ttf successfully!" << endl;

		renderTexture.create(signWidth, signWidth);

		text.setFont(font);
		text.setCharacterSize(signWidth);
		text.setString((char)(number + 64));

		renderTexture.clear(Color::Transparent);
		renderTexture.draw(text);
		renderTexture.display();

		shader->setTextTexture(renderTexture);

		if (number >= 1 && number <= 10) {
			string cardTypeSignPath = "";
			cardTypeSignPath = "shaders/CardTypeSign/" + cardTypeMap[cardTypeSet];
			if (!cardTypeTexture.loadFromFile("assets/CardTypeSign.png")) {
				cout << "Load CardTypeSign.png failed!" << endl;
			}
			else cout << "Load CardTypeSign.png successfully!" << endl;
			cardTypeTexture.setRepeated(false);

			int rectWidth = cardTypeTexture.getSize().x / 2;
			int rectHeight = cardTypeTexture.getSize().y / 2;

			int startX = 0, startY = 0, endX = 0, endY = 0;
			switch (cardTypeSet) {
			case CardType_Heart:
				startX = rectWidth;
				startY = 0;
				endX = rectWidth * 2;
				endY = rectHeight;
				break;
			case CardType_Spade:
				startX = 0;
				startY = 0;
				endX = rectWidth;
				endY = rectHeight;
				break;
			case CardType_Club:
				startX = rectWidth;
				startY = rectHeight;
				endX = rectWidth * 2;
				endY = rectHeight * 2;
				break;
			case CardType_Diamond:
				startX = 0;
				startY = rectHeight;
				endX = rectWidth;
				endY = rectHeight * 2;
				break;
			default:
				break;
			}

			cardTypeSign = new UseShader<Sprite>*[number];
			cardTypeShape = new Sprite[number];
			for (int i = 0; i < number; i++) {
				cardTypeShape[i].setTexture(cardTypeTexture);
				cardTypeShape[i].setTextureRect(IntRect(startX, startY, endX, endY));
				cardTypeShape[i].setScale(signWidth / cardTypeShape[i].getLocalBounds().width, signWidth / cardTypeShape[i].getLocalBounds().height);
			}
			float shapeWidth = cardTypeShape[0].getGlobalBounds().width;
			float shapeHeight = cardTypeShape[0].getGlobalBounds().height;
			for (int i = 0; i < number; i++) {
				
				if (number <= 3) {
					cardTypeShape[i].setPosition(x + width / 2 - shapeWidth / 2, y + height / (number + 1) * (i + 1) - shapeHeight / 2);
				}
				else {
					int outRow = number / 3 + 1;
					int inRow = number - outRow * 2;
					if (inRow == 0) cardTypeShape[i].setPosition(x + width / 3 * (i % 2 + 1) - shapeWidth / 2, y + height / (number / 2 + 1) * (i / 2 + 1) - shapeHeight / 2);
					else if (i + 1 > outRow && i + 1 <= outRow + inRow) cardTypeShape[i].setPosition(x + width / 2 - shapeWidth / 2, y + height / (inRow + 1) * (i + 1 - outRow) - shapeHeight / 2);
					else cardTypeShape[i].setPosition(x + width / 3 * (max(i - inRow, 0) / outRow + 1) - shapeWidth / 2, y + height / (outRow + 1) * min(i + 1, number - i) - shapeHeight / 2);
				}
				cardTypeSign[i] = new UseShader<Sprite>(windowSet, clock, cardTypeShape[i], "shaders/CardTypeSign/Card.vert", "shaders/CardTypeSign/Card.frag");
				cardTypeSign[i]->setShapeSize(shapeWidth, shapeHeight);
			}
		}
		else {
			if (!mid_font.loadFromFile("assets/XianzhouSeal-Regular.ttf")) {
				cout << "Load XianzhouSeal-Regular.ttf failed!" << endl;
			}
			else cout << "Load XianzhouSeal-Regular.ttf successfully!" << endl;

			int midWidth = width / 2;

			mid_renderTexture.create(midWidth, midWidth);

			mid_text.setFont(mid_font);
			mid_text.setCharacterSize(midWidth);
			mid_text.setFillColor(Color::Black);
			mid_text.setString((char)(number + 64));

			mid_renderTexture.clear(Color::Transparent);
			mid_renderTexture.draw(mid_text);
			mid_renderTexture.display();

			shader->setMidTexture(mid_renderTexture);
		}
	}
	void setCardSize(float widthSet, float heightSet) {
		width = widthSet;
		height = heightSet;
		Cardshape.setSize(sf::Vector2f(width, height));
		shader->setShapeSize(width, height);
	}
	void drawCard() {
		shader->useShader();
		if (cardTypeSign != NULL)
			for (int i = 0; i < number; i++) cardTypeSign[i]->useShader();
		//Cardshape.setRotation(clock.getElapsedTime().asSeconds());
		if (clock.getElapsedTime().asSeconds() > 1 && !check) {
			check = true;
			shader->saveImg();
		}
		window.display();
	}
	void disappearCard() {
		disappearClock.restart();
		shader->disappearShader();
		if (cardTypeSign != NULL)
			for (int i = 0; i < number; i++) cardTypeSign[i]->disappearShader();
	}

};