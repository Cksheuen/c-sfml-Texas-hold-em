#include <iostream>
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

template <typename Drawable>
class UseShader {
private:
    sf::RenderStates states;
    sf::RenderWindow& window;
    Drawable& shape;
    Clock clock, disappearClock;
    Vector2f globalPosition;
    bool disappear = false;
    float x, y, width, height;
    void setGlobalPosition(Drawable& drawable) {
        if constexpr (std::is_base_of<sf::Shape, Drawable>::value) {
            setGlobalPositionShape(drawable);
        }
        else if constexpr (std::is_same<sf::Sprite, Drawable>::value) {
            setGlobalPositionSprite(drawable);
        }
        else {
            static_assert(false, "Drawable must be a sf::Shape or sf::Sprite");
        }
    }

    void setGlobalPositionSprite(sf::Sprite& sprite) {
        sf::Sprite* spritePtr = dynamic_cast<sf::Sprite*>(&shape);
        if (spritePtr != nullptr) {
            sf::Vector2f globalPosition = sf::Vector2f(spritePtr->getPosition().x, window.getSize().y - spritePtr->getPosition().y - spritePtr->getGlobalBounds().height);
            shader.setUniform("global_pos", globalPosition);
            int x = spritePtr->getTextureRect().getPosition().x / (spritePtr->getTexture()->getSize().x / 2);
            int y = spritePtr->getTextureRect().getPosition().y / (spritePtr->getTexture()->getSize().y / 2);
            shader.setUniform("texture_pos", Vector2f(x, y));
        }
    }

    void setGlobalPositionShape(Drawable& drawable) {
        sf::CircleShape* circleShape = dynamic_cast<sf::CircleShape*>(&drawable);
        if (circleShape != nullptr) {
            setGlobalPositionCircleShape(*circleShape);
        }
        else {
            sf::RectangleShape* rectangleShape = dynamic_cast<sf::RectangleShape*>(&drawable);
            if (rectangleShape != nullptr) {
                setGlobalPositionRectangleShape(*rectangleShape);
            }
            else {
                sf::ConvexShape* convexShape = dynamic_cast<sf::ConvexShape*>(&drawable);
                if (convexShape != nullptr) {
                    setGlobalPositionConvexShape(*convexShape);
                }
            }
        }
    }

    void setGlobalPositionCircleShape(sf::CircleShape& shape) {
        sf::Vector2f globalPosition = sf::Vector2f(shape.getPosition().x
            , window.getSize().y - shape.getPosition().y - shape.getRadius() * 2.0);
        shader.setUniform("global_pos", globalPosition);
    }

    void setGlobalPositionRectangleShape(sf::RectangleShape& shape) {
        sf::Vector2f globalPosition = sf::Vector2f(shape.getPosition().x
            , window.getSize().y - shape.getPosition().y - shape.getSize().y);
        shader.setUniform("global_pos", globalPosition);
    }

    void setGlobalPositionConvexShape(sf::ConvexShape& shape) {
        sf::Vector2f globalPosition = sf::Vector2f(shape.getPosition().x
            , window.getSize().y - shape.getPosition().y - shape.getPoint(0).y);
        shader.setUniform("global_pos", globalPosition);
    }
    
public:
    sf::Shader shader;
    UseShader(sf::RenderWindow& windowSet, Clock& clockSet, Drawable& shapeSet, const std::string& VertexShaderPath, const std::string& FragmentShaderPath, const std::string& GeometryShaderPath = "")
        : window(windowSet), shape(shapeSet), clock(clockSet)
    {
        bool state;
        if (!GeometryShaderPath.empty()) {
            state = shader.loadFromFile(VertexShaderPath, GeometryShaderPath, FragmentShaderPath);
        }
        else {
            state = shader.loadFromFile(VertexShaderPath, FragmentShaderPath);
        }
        if (state) {
            std::cout << "Shader loaded successfully!" << std::endl;
        }
        else {
            std::cout << "Error: " << state << std::endl;
            return;
        }
        states.shader = &shader;
        //setGlobalPosition(shape);
        shader.setUniform("shape_size", sf::Vector2f(width, height));
        shader.setUniform("disappear_time", 0.0f);
        shader.setUniform("texture", sf::Shader::CurrentTexture);


    }
    void useShader() {
        shader.setUniform("time", clock.getElapsedTime().asSeconds());
        if (disappear) {
			shader.setUniform("disappear_time", disappearClock.getElapsedTime().asSeconds());
			if (disappearClock.getElapsedTime().asSeconds() > 1.0f) {
				disappear = false;
				shader.setUniform("disappear_time", 0.0f);
			}
		}
        
        window.draw(shape, states);
    }
    void setShapeSize(float width, float height) {
        shader.setUniform("shape_size", sf::Vector2f(width, height));
        //setGlobalPosition(shape);
    }
    void setTextTexture(sf::RenderTexture& texture) {
		shader.setUniform("text_texture", texture.getTexture());
	}
    void clear() {
        window.clear(sf::Color::White);
    }
    void initMidTexture() {
		shader.setUniform("if_mid_texture", false);
	}
    void setGlobalPosition(float x, float y) {
        shader.setUniform("global_pos", Vector2f(x, y));
	}
    void setMidTexture(sf::RenderTexture& texture) {
        shader.setUniform("if_mid_texture", true);
        shader.setUniform("mid_texture", texture.getTexture());
        cout << "set mid texture" << endl;
    }
    void disappearShader() {
        disappearClock.restart();
        disappear = true;
    }
    void saveImg() {
        sf::RenderTexture renderTexture;
        renderTexture.create(window.getSize().x, window.getSize().y);

        sf::Shader shader;
        // �������GLSL��ɫ��

        renderTexture.clear();
        renderTexture.draw(shape, states);
        renderTexture.display();

        sf::Image image = renderTexture.getTexture().copyToImage();
        image.saveToFile("output.png");
    }
};