enum ServerOrClient {
	Server,
	Client
};

class UseUI
{
private:
    TcpListener* listener;
    RenderWindow window;
    Clock clock;
    RenderStates bg_states;
    RectangleShape bg_shape;

    Font tvt_font, normal_font;

    UseCard* cards[52];
    UseChip* chips[8];

    vector<float> move_card_start_time, move_card_end_angle, move_card_time;
    vector<Vector2i> move_card_dest;

    bool my_turn = false;

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
    int WINDOW_WIDTH, WINDOW_HEIGHT;
    int init_complete = 0;
    bool to_fill = false;

public:
    vector<int> show_cards;
    Shader bg_shader;

    UseUI(int widthSet, int heightSet, TcpListener* listenerSet)
    : WINDOW_WIDTH(widthSet), WINDOW_HEIGHT(heightSet), listener(listenerSet){
        window.create(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "SFML Game Project");
        bg_shape.setSize(Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        bg_shape.setPosition(0, 0);

        string VertexShaderPath = "shaders/Bg/bg.vert";
        string FragmentShaderPath = "shaders/Bg/bg.frag";

        bool state = bg_shader.loadFromFile(VertexShaderPath, FragmentShaderPath);
        if (state) {
            std::cout << "Shader loaded successfully!" << std::endl;
        }
        else {
            std::cout << "Error: " << state << std::endl;
        }
        bg_states.shader = &bg_shader;

        bg_shader.setUniform("shape_size", sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));

        bool cardInit = false;

        if (!tvt_font.loadFromFile("assets/TeyvatBlack-Regular.otf")) {
            std::cout << "Load TeyvatBlack-Regular.ttf failed!" << endl;
        }
        if (!normal_font.loadFromFile("assets/hk4e_zh-cn.ttf")) {
            std::cout << "Load hk4e_zh-cn.ttf failed!" << endl;
        }
    };
    void UpdateProgressBar(float percent) {
        bg_shader.setUniform("percent", percent);
        if (percent == 1.0) {
            init_complete = 1;
            bg_shader.setUniform("init_complete", 1);
        }
    };
    void BasicUI(std::function<void()> InsertFn) {
        window.clear();
        InsertFn();
        bg_shader.setUniform("time", clock.getElapsedTime().asSeconds());
        window.draw(bg_shape, bg_states);
        window.display();
    };
    void RunProgressBar() {
        clock.restart();
        init_complete = 0;
        bg_shader.setUniform("init_complete", 0);
    };

    void InitCards() {
        int index = 0;
        for (CardType type = CardType_Heart; type < CardType_Joker; type = (CardType)(type + 1)) {
            for (int i = 1; i <= 13; i++) {
                cards[index++] = new UseCard(window, i, type, 0, WINDOW_HEIGHT / 2, WINDOW_HEIGHT / 12, WINDOW_HEIGHT / 4, tvt_font);

                BasicUI([&]() {
                    bg_shader.setUniform("percent", (float)(((type - CardType_Heart) * 13 + i) / 60.));
                    });
            }
        }
    };
    void InitChips() {
        int chips_value[8] = { 1, 2, 5, 10, 20, 25, 50, 100 };
        for (int i = 0; i < 8; i++) {
            chips[i] = new UseChip(window, WINDOW_WIDTH / 10 * (i + 1), WINDOW_HEIGHT / 4, WINDOW_WIDTH / 12, chips_value[i], normal_font);

            BasicUI([&]() {
                bg_shader.setUniform("percent", (float)((i + 53) / 60.));
                });
        }
    };

    ServerOrClient GameMenu() {
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

        while (true) {
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
                while (listener->listen(port) != Socket::Done && port < 4000) {
                    port++;
                }
                std::cout << "create game room, the room is on the local port: " << port << endl;
                return Server;
                break;
            }
            if (button_list[1]->click()) {
                std::cout << "join game room" << endl;
                return Client;
                break;
            }

            window.display();
        }
    };

    void RoomOwnerInterface(UseServer& server, int* player_count, bool* GameStart) {
        std::cout << "start room owner interface" << endl;
        float button_x = WINDOW_WIDTH / 2;
        float button_y = WINDOW_HEIGHT / 3;
        cout << "room owner interface init start" << endl;
        UseButton* start_button = new UseButton(window, button_x, button_y, 100, "start game", normal_font);
        clock.restart();

        cout << "room owner interface init complete" << endl;

        while (!*GameStart)
        {
            //server.ReceiveMessageInWhile();
            window.clear(sf::Color::White);

            bg_shader.setUniform("time", clock.getElapsedTime().asSeconds());
            window.draw(bg_shape, bg_states);

            start_button->hover();
            if (start_button->click()) {
                *GameStart = true;
                std::cout << "game_start" << endl;
                Packet packet;
                packet << "game_start";
                server.SendToAllClients(packet);
            }
            start_button->show();

            Text text;
            text.setFont(normal_font);
            text.setString("waiting for clients..., now " + to_string(*player_count) + " players");
            text.setCharacterSize(24);
            text.setFillColor(Color::Black);
            text.setPosition(10, 10);
            window.draw(text);

            window.display();
        }
        cout << "room owner interface end" << endl;
    };
    int ChooseRoomInterface(UseClient client, bool* search_room_complete, vector<int>& room_list) {

        float button_x = WINDOW_WIDTH / 2;
        float button_y = WINDOW_HEIGHT / 3;

        RunProgressBar();

        while (!*search_room_complete)
        {
            bg_shader.setUniform("time", clock.getElapsedTime().asSeconds());
            window.draw(bg_shape, bg_states);
            window.display();
        }

        cout << "room_list size: " << room_list.size() << endl;

        vector<UseButton*> room_button_list;

        for (int i = 0; i < room_list.size(); i++) {
            std::cout << "room_but " << room_list[i] << " is ready" << endl;
            room_button_list.push_back(new UseButton(window, button_x, button_y + i * 70, 100, "room " + to_string(room_list[i]), normal_font));

            BasicUI([&]() {
                bg_shader.setUniform("percent", (float)(0.5 + (i + 1) / room_list.size()));
                });
        }

        bg_shader.setUniform("init_complete", 1);
        clock.restart();

        std::cout << "start choose room interface" << endl;

        int room_index = -1;
        bool GameStart = false;
        while (!GameStart)
        {
            window.clear(sf::Color::White);
            bg_shader.setUniform("time", clock.getElapsedTime().asSeconds());
            window.draw(bg_shape, bg_states);

            for (int i = 0; i < room_button_list.size(); i++) {
                room_button_list[i]->hover();
                room_button_list[i]->show();
                if (room_button_list[i]->click()) {
                    return room_list[i];
                    GameStart = true;
                    break;
                }
            }

            window.display();
        }
    };

    void ServerGameInterface(UseServer& server) {
        Packet packet;

        UseButton call_button(window, WINDOW_WIDTH / 4, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 8, "call", normal_font);
        UseButton fill_button(window, WINDOW_WIDTH / 4 * 2, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 8, "fill", normal_font);
        UseButton back_button(window, WINDOW_WIDTH / 3, WINDOW_HEIGHT / 3, WINDOW_WIDTH / 8, "back", normal_font);
        UseButton give_up_button(window, WINDOW_WIDTH / 4 * 3, WINDOW_HEIGHT / 3, WINDOW_WIDTH / 8, "give up", normal_font);
        UseButton over_button(window, WINDOW_WIDTH / 3 * 2, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 8, "over", normal_font);

        bg_shader.setUniform("init_complete", 1);
        clock.restart();

        server.SendCardToAll([this](int x, int y) {
            cout << "Send Card To Server Self" << endl;
            AddMoveCard(x, WINDOW_WIDTH / 3, WINDOW_HEIGHT / 3 * 2, 0);
            AddMoveCard(y, WINDOW_WIDTH / 3 * 2, WINDOW_HEIGHT / 3 * 2, 0);
            });

        server.RunTurns([this](int new_public_card) {
            cout << "to run AddNewPublicCard" << endl;
            AddNewPublicCard(new_public_card);
            cout << "end AddNewPublicCard" << endl;
            },
            [this](bool my_turn) {
                SetMyTurn(my_turn);
            });

        Text text;
        text.setFont(normal_font);
        text.setString("your turn");
        text.setCharacterSize(24);
        text.setFillColor(Color::Black);
        text.setPosition(10, 10);

        while (true)
        {
            window.clear(sf::Color::White);
            bg_shader.setUniform("time", clock.getElapsedTime().asSeconds());
            window.draw(bg_shape, bg_states);

            call_button.hover();
            fill_button.hover();
            give_up_button.hover();
            
            if (my_turn) {
                if (call_button.click()) {
                    packet << "call";
                    server.ReceiveOwnMessage(packet);
                    std::cout << "call" << endl;
                }
                if (fill_button.click()) {
                    packet << "fill";
                    server.ReceiveOwnMessage(packet);
                    std::cout << "start fill" << endl;

                    to_fill = true;
                }
                if (give_up_button.click()) {
                    packet << "give_up";
                    server.ReceiveOwnMessage(packet);
                    std::cout << "give_up" << endl;
                }

                if (to_fill) {
                    for (int i = 0; i < 8; i++) {
                        chips[i]->show();
                        chips[i]->hover();
                        if (chips[i]->click()) {
                            packet << chips[i]->value;
                            server.ReceiveOwnMessage(packet);
                            std::cout << "fill " << chips[i]->value << endl;
                        }
                    }
                    back_button.show();
                    back_button.hover();
                    if (back_button.click()) {
                        to_fill = false;
                        packet << "over_turn";
                        server.ReceiveOwnMessage(packet);
                        std::cout << "over_turn" << endl;
                        my_turn = false;
                    }
                }
            }

            call_button.show();
            fill_button.show();
            give_up_button.show();

            if (show_cards.size() != 0) {
                for (int i = 0; i < show_cards.size(); i++) {
                    if (clock.getElapsedTime().asSeconds() < move_card_start_time[i] + move_card_time[i])
                    {
                        float time = clock.getElapsedTime().asSeconds() - move_card_start_time[i];
                        float dx = smoothstep(0, move_card_time[i], time) * move_card_dest[i].x;
                        float dy = smoothstep(0, move_card_time[i], time) * move_card_dest[i].y;
                        float angle = smoothstep(0, 1, clock.getElapsedTime().asSeconds()) * move_card_end_angle[i];
                        cards[show_cards[i]]->setPos(dx, dy);
                        cards[show_cards[i]]->setRotation(angle);
                    }
                    cards[show_cards[i]]->drawCard();
                }
            }

            if (my_turn) {
				window.draw(text);
			}

            window.display();
        }
	};

    void ClientGameInterface(UseClient client, int room_index) {
        std::cout << "to join room " << room_index << endl;
        TcpSocket* socket = new TcpSocket;
        if (socket->connect("localhost", room_index) == sf::Socket::Done) {
            std::cout << "connect to room " << room_index << " successfully" << endl;
            Packet packet;
            packet << "hello hello";
            socket->send(packet);
        }
        else {
            std::cout << "connect to room " << room_index << " failed" << endl;
        }
        Packet packet;

        UseButton call_button(window, WINDOW_WIDTH / 4, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 8, "call", normal_font);
        UseButton fill_button(window, WINDOW_WIDTH / 4 * 2, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 8, "fill", normal_font);
        UseButton back_button(window, WINDOW_WIDTH / 3, WINDOW_HEIGHT / 3, WINDOW_WIDTH / 8, "back", normal_font);
        UseButton give_up_button(window, WINDOW_WIDTH / 4 * 3, WINDOW_HEIGHT / 3, WINDOW_WIDTH / 8, "give up", normal_font);
        UseButton over_button(window, WINDOW_WIDTH / 3 * 2, WINDOW_HEIGHT / 2, WINDOW_WIDTH / 8, "over", normal_font);

        bg_shader.setUniform("init_complete", 1);
        clock.restart();

        Text text;
        text.setFont(normal_font);
        text.setString("your turn");
        text.setCharacterSize(24);
        text.setFillColor(Color::Black);
        text.setPosition(10, 10);

        while (true)
        {
            window.clear(sf::Color::White);
            bg_shader.setUniform("time", clock.getElapsedTime().asSeconds());
            window.draw(bg_shape, bg_states);

            call_button.hover();
            fill_button.hover();
            give_up_button.hover();
            back_button.hover();
            over_button.hover();

            if (my_turn) {
                if (call_button.click()) {
                    packet << "call";
                    socket->send(packet);
                    std::cout << "call" << endl;
                }
                if (fill_button.click()) {
                    packet << "fill";
                    socket->send(packet);
                    std::cout << "start fill" << endl;

                    to_fill = true;
                }
                if (give_up_button.click()) {
                    packet << "give_up";
                    socket->send(packet);
                    std::cout << "give_up" << endl;
                }

                if (to_fill) {
                    for (int i = 0; i < 8; i++) {
                        chips[i]->setPosition(i, WINDOW_HEIGHT / 2.);
                        chips[i]->show();
                        chips[i]->hover();
                        if (chips[i]->click()) {
                            packet << chips[i]->value;
                            socket->send(packet);
                            std::cout << "fill " << chips[i]->value << endl;
                        }
                    }
                    back_button.show();
                    back_button.hover();
                    if (back_button.click()) {
                        to_fill = false;
                        packet << "over_turn";
                        socket->send(packet);
                        std::cout << "over_turn" << endl;
                        my_turn = false;
                    }
                }
            }

            call_button.show();
            fill_button.show();
            give_up_button.show();
            back_button.show();
            over_button.show();


            if (show_cards.size() != 0) {
                for (int i = 0; i < show_cards.size(); i++) {
                    if (clock.getElapsedTime().asSeconds() < move_card_start_time[i] + move_card_time[i])
                    {
                        float time = clock.getElapsedTime().asSeconds() - move_card_start_time[i];
                        float dx = smoothstep(0, move_card_time[i], time) * move_card_dest[i].x;
                        float dy = smoothstep(0, move_card_time[i], time) * move_card_dest[i].y;
                        float angle = smoothstep(0, 1, clock.getElapsedTime().asSeconds()) * move_card_end_angle[i];
                        cards[show_cards[i]]->setPos(dx, dy);
                        cards[show_cards[i]]->setRotation(angle);
                    }
                    cards[show_cards[i]]->drawCard();
                }
            }

            if (my_turn) {
                window.draw(text);
            }

            window.display();
        }
    };

    void AddMoveCard(int cardIndex, float x, float y, float angle) {
		show_cards.push_back(cardIndex);
        float move_time = (x + y) / 700.;
        move_card_start_time.push_back(clock.getElapsedTime().asSeconds());
        move_card_time.push_back(move_time);
        move_card_dest.push_back(Vector2i(x, y));
        move_card_end_angle.push_back(angle);
	};

    void AddNewPublicCard(int cardIndex) {
        AddMoveCard(cardIndex, WINDOW_WIDTH / 7 * show_cards.size(), WINDOW_HEIGHT / 3, 0);
	};

    void SetMyTurn(bool my_turn_set) {
		my_turn = my_turn_set;
	};

};
