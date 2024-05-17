#ifndef USE_CARD
#define USE_CARD

#include <iostream>
#include <SFML/Graphics.hpp>
#include "UseShader.h"
#include "UseCal.h"

using namespace std;
using namespace sf;

class UseCard {
private:
	// 扑克点数
	int number;
	bool check = false;
	enum CardType cardType;
	UseShader<RectangleShape>* shader;
	UseShader<Sprite>**cardTypeSign;
	RenderWindow& window;
	Sprite completeCard;
	float  width, height, signWidth;
	sf::RectangleShape Cardshape;
	sf::Sprite* cardTypeShape;
	Clock clock, disappearClock;
	Texture cardTypeTexture, comTexture;
	Font font, mid_font;
	Text text, mid_text;
	RenderTexture renderTexture, mid_renderTexture, cardRenderTexture;
	FloatRect originalBounds;
	Text hoverText;
	bool hoverState = false;


	int min(int x, int y) {
		return x < y ? x : y;
	}
	int max(int x, int y) {
		return x > y ? x : y;
	}


public:
	float x, y, angle;
	UseCard(sf::RenderWindow& windowSet, int numberSet, enum CardType cardTypeSet, float xSet, float ySet,
		float widthSet, float heightSet, Font fontSet)
		: window(windowSet), number(numberSet), cardType(cardTypeSet), x(xSet), y(ySet),
		width(widthSet), height(heightSet), Cardshape(sf::Vector2f(width, height)), signWidth(width / 4),
		font(fontSet){
		Cardshape.setPosition(x, y);
		shader = new UseShader<RectangleShape>(windowSet, clock, Cardshape, "shaders/card/card.vert", "shaders/card/card.frag");
		shader->setShapeSize(width, height);
		shader->initMidTexture();

		cardTypeSign = NULL;

		renderTexture.create(signWidth, signWidth);

		text.setFont(font);
		text.setCharacterSize(signWidth);
		text.setString((char)(number + 64));

		renderTexture.clear(Color::Transparent);
		renderTexture.draw(text);
		renderTexture.display();

		shader->setTextTexture(renderTexture);

		if (number >= 1 && number <= 10) {

			//shader->saveImg();

			Image image = shader->getImg();
			Texture cardTexture;
			Sprite cardSprite;

			cardTexture.loadFromImage(image);
			cardSprite.setTexture(cardTexture);

			cardSprite.setPosition(-x, -y);

			cardRenderTexture.create(width, height);
			cardRenderTexture.clear(Color::Transparent);
			cardRenderTexture.draw(cardSprite);

			string cardTypeSignPath = "";
			cardTypeSignPath = "shaders/CardTypeSign/" + cardTypeMap[cardTypeSet];
			if (!cardTypeTexture.loadFromFile("assets/CardTypeSign.png")) {
				cout << "Load CardTypeSign.png failed!" << endl;
			}
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

			Sprite signSprite;
			signSprite.setTexture(cardTypeTexture);
			signSprite.setTextureRect(IntRect(startX, startY, endX - startX, endY - startY));
			signSprite.setScale(signWidth / signSprite.getLocalBounds().width, signWidth / signSprite.getLocalBounds().height);

			for (int i = 0; i < number; i++) {
				if (number <= 3) {
					signSprite.setPosition(width / 2 - signWidth / 2, height / (number + 1) * (i + 1) - signWidth / 2);
				}
				else {
					int outRow = number / 3 + 1;
					int inRow = number - outRow * 2;
					if (inRow == 0) signSprite.setPosition(width / 3 * (i % 2 + 1) - signWidth / 2, height / (number / 2 + 1) * (i / 2 + 1) - signWidth / 2);
					else if (i + 1 > outRow && i + 1 <= outRow + inRow) signSprite.setPosition(width / 2 - signWidth / 2, height / (inRow + 1) * (i + 1 - outRow) - signWidth / 2);
					else signSprite.setPosition(width / 3 * (max(i - inRow, 0) / outRow + 1) - signWidth / 2, height / (outRow + 1) * min(i + 1, number - i) - signWidth / 2);

				}
				cardRenderTexture.draw(signSprite);
			}

			cardRenderTexture.display();
			completeCard.setTexture(cardRenderTexture.getTexture());

		}
		else {
			if (!mid_font.loadFromFile("assets/XianzhouSeal-Regular.ttf")) {
				cout << "Load XianzhouSeal-Regular.ttf failed!" << endl;
			}

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

			Image image = shader->getImg();
			comTexture.loadFromImage(image);

			completeCard.setTexture(comTexture);
			completeCard.setTextureRect(sf::IntRect(x, y, width, height));
		}

		completeCard.setOrigin(width / 2, height / 2);
	}
	void setCardSize(float widthSet, float heightSet) {
		width = widthSet;
		height = heightSet;
		Cardshape.setSize(sf::Vector2f(width, height));
		shader->setShapeSize(width, height);
	}
	void drawCard() {
		/*shader->useShader();
		if (cardTypeSign != NULL)
			for (int i = 0; i < number; i++) cardTypeSign[i]->useShader();
		*/
		completeCard.setPosition(x, y);
		completeCard.setRotation(angle);
		originalBounds = completeCard.getGlobalBounds();
		window.draw(completeCard);
		//window.display();
		if (hoverState) {
			window.draw(hoverText);
		}
	}
	void disappearCard() {
		disappearClock.restart();
		shader->disappearShader();
		for (int i = 0; i < number; i++) cardTypeSign[i]->disappearShader();
	}

	void saveAsPng(Sprite sprite) {

		sf::RenderTexture renderTexture;
		renderTexture.create(sprite.getGlobalBounds().width, sprite.getGlobalBounds().height);
		renderTexture.clear(sf::Color::Transparent);
		renderTexture.draw(sprite);
		renderTexture.display();

		sf::Image image = renderTexture.getTexture().copyToImage();
		image.saveToFile("output.png");
		cout << "save as png" << endl;
	}
	void setRotation(float angleSet) {
		angle = angleSet;
	}
	void setPos(float xSet, float ySet) {
		x = xSet;
		y = ySet;
	}

	bool Hover() {
		Vector2i mousePos = Mouse::getPosition(window);

		if (originalBounds.contains(mousePos.x, mousePos.y)) {
			hoverText.setFont(font);
			hoverText.setCharacterSize(24);

			string cardTypeStr = cardTypeNumMap[cardType] + " " + number_to_string(number);
			hoverText.setString(cardTypeStr);
			hoverText.setFillColor(Color::Blue);
			hoverText.setPosition(x, y - 50);
			hoverState = true;
			return true;
		}
		else {
			hoverState = false;
			return false;
		}
	}

	void scale(float x, float y) {
		completeCard.setScale(x, y);
	}

};

#endif