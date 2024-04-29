#include <iostream>
#include <math.h>
#include <cassert>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <thread>
#include "UseCard.h"
#include "UseButton.h"
#include "UseChip.h"

using namespace std;
using namespace sf;

const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 800;

void BasicInput(RenderWindow& window);

bool StartInterface = true;
bool ChooseRoomInterface = false;
bool RoomOwnerInterface = false;
bool init = false;

template<class T>
const T& clamp(const T& v, const T& lo, const T& hi) {
    assert(!(hi < lo));
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

float smoothstep(float edge0, float edge1, float x) {
    x = clamp(static_cast<double>((x - edge0) / (edge1 - edge0)), 0.0, 1.0);
    return x * x * (3 - 2 * x);
}

float step(float edge, float x) {
    return x < edge ? 0.0 : 1.0;
}

int main() {
    TcpListener listener;


    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "SFML Game Project");
    Clock clock;
    Shader bg_shader;
    RenderStates bg_states;

    RectangleShape bg_shape(Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    bg_shape.setPosition(0, 0);

    string VertexShaderPath = "shaders/Bg/bg.vert";
    string FragmentShaderPath = "shaders/Bg/bg.frag";

    bool state = bg_shader.loadFromFile(VertexShaderPath, FragmentShaderPath);
    if (state) {
        std::cout << "Shader loaded successfully!" << std::endl;
    }
    else {
        std::cout << "Error: " << state << std::endl;
        return 0;
    }
    bg_states.shader = &bg_shader;

    bg_shader.setUniform("shape_size", sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));


    clock.restart();

    bool cardInit = false;

    Font tvt_font, normal_font;

    if (!tvt_font.loadFromFile("assets/TeyvatBlack-Regular.otf")) {
        cout << "Load TeyvatBlack-Regular.ttf failed!" << endl;
    }
    if (!normal_font.loadFromFile("assets/hk4e_zh-cn.ttf")) {
        cout << "Load hk4e_zh-cn.ttf failed!" << endl;
    }


    UseCard* cards[52];

    int index = 0;
    for (CardType type = CardType_Heart; type < CardType_Joker; type = (CardType)(type + 1)) {
        for (int i = 1; i <= 13; i++) {
            cards[index++] = new UseCard(window, i, type, 0, WINDOW_HEIGHT / 2, WINDOW_HEIGHT / 12, WINDOW_WIDTH / 6, tvt_font);
            bg_shader.setUniform("percent", (float)(((type - CardType_Heart) * 13 + i) / 60.));

            bg_shader.setUniform("time", clock.getElapsedTime().asSeconds());
            window.draw(bg_shape, bg_states);
            window.display();
        }
    }

    UseChip* chips[8];
    int chips_value[8] = { 1, 2, 5, 10, 20, 25, 50, 100 };
    for (int i = 0; i < 8; i++) {
        chips[i] = new UseChip(window, 100 + i * 100, 100, 50, chips_value[i], normal_font);
        bg_shader.setUniform("percent", (float)((i + 53) / 60.));

        bg_shader.setUniform("time", clock.getElapsedTime().asSeconds());
        window.draw(bg_shape, bg_states);
        window.display();
    }


    float circle_center_x = WINDOW_WIDTH / 2;
    float circle_center_y = WINDOW_HEIGHT * 1.2;

    int hoverIndex = -1;
    Clock hoverClock;


    UseButton* button_list[2];
    float button_x = WINDOW_WIDTH / 2;
    float button_y = WINDOW_HEIGHT / 3;

    button_list[0] = new UseButton(window, button_x, button_y, 100, "create game room", normal_font);
    button_list[1] = new UseButton(window, button_x, button_y + 70, 100, "join game room", normal_font);

    bg_shader.setUniform("init_complete", 1);
    clock.restart();

    while (StartInterface) {
        //BasicInput(window);
        window.clear(sf::Color::White);
        bg_shader.setUniform("time", clock.getElapsedTime().asSeconds());
        window.draw(bg_shape, bg_states);

        for (int i = 0; i < 52; i++) {
            if (clock.getElapsedTime().asSeconds() <= 1.) {
                float start_theta = atan2f(-WINDOW_HEIGHT / 2., -WINDOW_WIDTH / 2.) / 3.14 * 180;
                float end_theta = (abs(start_theta) - 90) / 52 * 2. * i;
                float now_theta = smoothstep(float(0.), 1., clock.getElapsedTime().asSeconds()) * end_theta;
                float x = circle_center_x + cosf((start_theta + now_theta) / 180 * 3.14) * WINDOW_WIDTH / 1;
                float y = circle_center_y + sinf((start_theta + now_theta) / 180 * 3.14) * WINDOW_HEIGHT / 2;
                cards[i]->setPos(x, y);
                cards[i]->setRotation(start_theta + now_theta - 90);
            }
            cards[i]->drawCard();
        }
        for (int i = 51; i >= 0; i--) {
            if (cards[i]->Hover()) {
                if (i != hoverIndex) {
                    if (hoverIndex != -1) cards[hoverIndex]->scale(1, 1);
                    hoverClock.restart();
                    hoverIndex = i;
                }
                else {
                    float scale = smoothstep(0., .01, hoverClock.getElapsedTime().asSeconds()) * .2 + 1.;
                    cards[i]->scale(scale, scale);
                }
                break;
            }
            else if (hoverIndex == i) {
                hoverIndex = -1;
                cards[i]->scale(1, 1);
            }
        }

        for (int i = 0; i < 8; i++) {
            chips[i]->show();
        }

        for (int i = 0; i < 2; i++) {
            button_list[i]->hover();
			button_list[i]->show();
		}

        if (button_list[0]->click()) {
            int port = 3000;
            while (listener.listen(port) != Socket::Done && port < 4000) {
				port++;
			}
			cout << "create game room, the room is on the local port: " << port << endl;
			RoomOwnerInterface = true;
			break;
		}
        if (button_list[1]->click()) {
            cout << "join game room" << endl;
            ChooseRoomInterface = true;
            break;
        }
        

        window.display();
    }

    if (RoomOwnerInterface) {
        cout << "start room owner interface" << endl;
        window.clear(sf::Color::White);
        window.display();
        while (true)
        {
        }
    }

    bg_shader.setUniform("init_complete", 0);
    clock.restart();

    vector<int> room_list;

    for (int port = 3000; port < 4000; port++) {
		TcpSocket socket;
        if (socket.connect("localhost", port) == Socket::Done) {
            room_list.push_back(port);
            cout << "room " << port << " is available" << endl;
            bg_shader.setUniform("percent", (float)((port - 3000.0) / 2000.0));

            bg_shader.setUniform("time", clock.getElapsedTime().asSeconds());
            window.draw(bg_shape, bg_states);
            window.display();
            socket.disconnect();
        }
        else break;
    }

    vector<UseButton*> room_button_list;

    for (int i = 0; i < room_list.size(); i++) {
        cout << "create room button " << i << endl;
		room_button_list.push_back(new UseButton(window, button_x, button_y + i * 70, 100, "room " + to_string(room_list[i]), normal_font));
        bg_shader.setUniform("percent", (float)(0.5 + (i + 1) / room_list.size()));

        bg_shader.setUniform("time", clock.getElapsedTime().asSeconds());
        window.draw(bg_shape, bg_states);
        window.display();
    }

    bg_shader.setUniform("init_complete", 1);
    clock.restart();

    cout << "start choose room interface" << endl;
    while (ChooseRoomInterface)
    {
        window.clear(sf::Color::White);
		bg_shader.setUniform("time", clock.getElapsedTime().asSeconds());
		window.draw(bg_shape, bg_states);

        for (int i = 0; i < room_button_list.size(); i++) {
			room_button_list[i]->hover();
			room_button_list[i]->show();
            if (room_button_list[i]->click()) {
				cout << "join room " << room_list[i] << endl;
				break;
			}
		}

		window.display();
    }
    window.close();

    return 0;
}